# Music Control + Hype/Rest

## Scope & integration (updated for WiFi/backend architecture)

Spotify only, v1 (locked). The device no longer relays commands through a phone app —
there is no BLE, no phone-side relaying of any kind. Instead, every Spotify interaction
goes **device → backend → Spotify**: the device never calls Spotify directly and never
holds any Spotify credential, not even a short-lived access token (locked call shape,
see `01-hardware-firmware.md`). The device sends generic commands over WiFi to the
backend's `/spotify/*` proxy endpoints (`03-web-app.md`); the backend attaches a fresh
Spotify access token (refreshed server-side from the account's stored refresh token) and
makes the actual `/me/player/*` call.

Command layer should stay generic ("next track," "play playlist X") rather than shaped
around Spotify's exact response payloads — same rationale as before: keeps a future
service swap realistic without spending real effort on that abstraction now.

## Standard playback controls (unchanged — locked)

- Play / Pause
- Next / Previous track
- Volume up / down (see Volume section — this changed meaningfully in the pivot)
- Playlist selection (from a pre-configured shortlist curated in the web app, not full
  library browsing)
- Song/track switching within the current playlist or queue

## Playlist browsing model (unchanged — locked)

- User curates a shortlist of playlists **in the web app** (reading their own Spotify
  playlists via the backend), which is what appears in the on-device playlist picker.
- Music Queue screen shows shuffle toggle, repeat toggle, and next 5–10 upcoming tracks,
  scrollable via the encoder — see `02-hardware-ui.md` for the exact interaction (shuffle/
  repeat are rows in the same scrollable list as the tracks, not a separate toggle UI).
- Keeps on-device data light and the interaction fast — no full-library sync needed.

## Volume control (resolved — locked, simpler than the original BLE-era design)

**What changed**: in the original BLE design, the device had no audio output itself, so
"volume" had to mean either the phone's system volume or the Spotify Connect device's
volume, and the phone-relay architecture forced an awkward Android/iOS platform split
(silent system-volume change on Android, an unavoidable visible HUD on iOS).

**Now**: the device sends a generic "set volume to N" command to the backend, which
translates it into a `PUT /me/player/volume` call to Spotify — there is no
platform-specific behavior to design around at all, since the underlying Spotify call is
identical regardless of who issues it. This eliminates the old Android/iOS asymmetry
entirely.

**How target-device selection works (confirmed)**: Spotify only ever has one *active*
playback device on an account at a time — if a user has multiple devices open, they only
ever see "playing on [device X]" and can only control playback, not silently start
audio on themselves. This device's model matches that exactly: it never needs to know or
choose *which* physical speaker/phone is producing sound — every command
(play/pause/next/volume/etc.) is issued without a `device_id`, and Spotify automatically
applies it to whatever the currently-active device is. **No device picker UI is needed,
on-device or in the web app.**

**Edge case — nothing playing anywhere (locked wording)**: if the account has no active
device at all (e.g. Spotify hasn't been opened anywhere recently), playback-state and
command calls will fail/return empty. The music block on Home and the fixed block atop
Music Queue both show, in place of track info: **"Nothing playing — open Spotify on any
device"** — an inline message, not an error state or a dedicated screen, consistent with
the minimal-screens philosophy. A Hype/Rest press with nothing active can't
snapshot/restore anything meaningful, so it does **not** start a timer in that case —
instead the mode-indicator box on Home briefly shows the same **"Nothing playing — open
Spotify on any device"** message instead of a countdown, then reverts to its normal
idle state after ~3s.

## Hype / Rest — flagship feature (unchanged — locked)

Two dedicated physical buttons, each independently configurable:

1. **Trigger** (only reachable on the Home screen — see `02-hardware-ui.md` for the
   locked global button-mapping rule and rationale): atomically snapshots current
   playback state (track, position, queue/context, shuffle, repeat), starts the
   configured target playlist, starts the configured countdown.
2. **On-screen**: countdown shown alongside whatever else is on screen.
3. **On expiry**: playback state restored exactly from the snapshot.
4. **Second-press behavior during an active Hype/Rest** (user-configurable, one of):
   reset the timer; reset the timer and reshuffle/restart the playlist; cancel entirely
   and restore immediately. Configured in Settings — Hype & Rest.
5. **Stacking behavior (locked, revised)**: only **one snapshot slot** exists at a time.
   Pressing the *other* button while a mode is active does **not** take a new snapshot —
   it keeps the **original snapshot from before the first press in the current chain**,
   and just switches the active mode's playlist/duration to the newly-pressed button's
   config, restarting the timer at that new duration. Pressing the *same* button again
   while active is unaffected by this — that's still governed by the per-button
   "second-press behavior" setting above (reset timer / reset+reshuffle / cancel-
   restore-immediately), which also doesn't touch the original snapshot. **On final
   expiry (or a cancel), playback always restores to that one original snapshot** —
   never to whatever was playing at some intermediate point in the chain. Concretely:
   press Hype (snapshots pre-Hype state, starts Hype playlist/timer) → press Rest before
   Hype expires (does *not* re-snapshot the Hype playlist; switches to Rest's
   playlist/timer, original pre-Hype snapshot untouched) → Rest expires → restores the
   **original pre-Hype state**, not the Hype playlist. Same logic applies to any
   same-button chain (Hype→Hype, Rest→Rest): only the original pre-chain snapshot is
   ever the restore target, and it's forgotten only once that restore actually happens.
6. **"Exercise based time" opt-in** (confirmed during the UI discussion, unchanged):
   per-button setting that, when enabled, sources that trigger's *duration* from a
   per-exercise authored field (`targetHypeSeconds` / `targetRestSeconds`, see
   `04-workout-logging.md`) instead of the fixed Settings duration. Does not change the
   manual-trigger-only rule — only where the duration number comes from.

## Relationship to workout rest timers (unchanged — locked, explicitly separate)

Hype/Rest is entirely independent from any rest-between-sets timing in the guided
workout flow — manual only, per `04-workout-logging.md`.

## Now-playing data (unchanged — locked, but see album art note)

**There is no separate "Now Playing" screen.** This is one data component — track
title, artist, elapsed/total time with progress bar, current exercise context where
relevant — rendered in two places: as part of the Home screen's music block, and as the
fixed non-interactive block at the top of the Music Queue screen (see
`02-hardware-ui.md`). Same data, same fields, two render locations.

**Album art**: per the resolved UI discussion, the Home screen shows real synced Spotify
album art (not fixed local icons as originally planned). The backend's proxied
playback-state response includes the track's album art URL (a public Spotify CDN URL);
the device then fetches the image bytes directly from that URL over WiFi — this doesn't
violate the backend-proxy rule for authenticated Spotify API calls, since CDN image
fetches need no Spotify auth at all. This also means the BLE-transfer-format question
from the original design no longer applies.

**Format/caching approach (locked):**
- **Fetch trigger**: fetch-on-track-change only — compare the incoming track ID against
  the currently-cached one on every poll response; skip the fetch entirely if unchanged.
  This is the dominant case (polling happens far more often than tracks change), so it's
  the only optimization worth doing.
- **Requested size**: Spotify's CDN image URLs come in multiple fixed sizes (typically
  ~64px, ~300px, ~640px); request the **~300px variant** and downscale on-device — closer
  to the Home screen's art box than the 640px variant, so less to decode and resize, while
  still being sharp after downscaling (avoid the 64px variant, which would upscale and
  look soft).
- **Decode library**: **TJpg_Decoder** (Spotify CDN art is JPEG) — chosen because it's
  the standard pairing with TFT_eSPI already locked as the display library, decodes
  progressively into caller-provided callback tiles (good fit for direct-to-framebuffer
  rendering without a full second RGB565 buffer in RAM), and is a well-maintained,
  commonly-used Arduino library (low integration risk).
- **On-device cache**: **RAM only, single most-recent image, not persisted to flash.**
  Album art is inherently ephemeral (changes with the track) and re-fetching on the rare
  case of a reboot mid-song is cheap; persisting it to flash would add wear and
  complexity for no real benefit given the fetch-on-change strategy above already
  minimizes redundant fetches.
- **Decode target size**: decode straight to the Home screen's art box dimensions (TBD
  exact px once the Lopaka re-mock for 240×320 is done, see `02-hardware-ui.md`) rather
  than decoding full-size and scaling separately — TJpg_Decoder supports scale-during-
  decode (1/2, 1/4, 1/8), so pick whichever factor lands closest to the target box size
  from the ~300px source.

## Reconnection / token-expiry behavior (new consideration for this architecture)

Because every Spotify interaction is backend-proxied (the device never talks to Spotify
directly), "music unavailable" now has two possible causes: no WiFi (device can't reach
the backend at all), or a backend/Spotify auth problem (e.g. the 6-month Spotify
refresh-token expiry described in `03-web-app.md`, which the backend surfaces as a
proxy-call failure). **Locked**: the device doesn't distinguish these on-screen — a
single inline state, **"Music unavailable — check the web app"**, covers both, replacing
the music block on Home and the fixed block atop Music Queue. The *why* (reconnect WiFi
vs. reconnect Spotify) is surfaced in the web app, which is better equipped to explain
it than a small on-device message. The distinguishing signal for *which* inline state to
show: a WiFi-icon check (disconnected → this is clearly a connectivity issue, though the
message stays the same either way) is not needed to pick the message — the message is
identical regardless of cause, so the device doesn't need to distinguish the two causes
at all, only detect "a Spotify call failed or WiFi is down" as one combined condition.

---

## Open questions to resolve during implementation

All decisions previously open in this doc are now locked (album art fetch/decode/cache
approach, inline-state wording, Hype/Rest stacking behavior — see above). The only thing
left is a pixel-level detail that depends on the Phase 1 Lopaka re-mock, not a product
decision:

- Exact art-box pixel dimensions on the re-mocked 240×320 Home screen (needed to pick
  the TJpg_Decoder scale factor above) — resolved automatically once
  `02-hardware-ui.md`'s Lopaka re-mock task is done in Phase 1; no separate decision
  needed here.
