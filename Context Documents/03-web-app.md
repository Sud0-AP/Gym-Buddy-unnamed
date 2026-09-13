# Web App + Backend — Full Spec

## Role of the web app

Replaces the original native mobile app entirely. Responsibilities:
- Account creation/login (Google via Firebase Auth)
- Spotify connection (separate OAuth, PKCE)
- Workout template/library authoring (splits, workouts, exercises, targets)
- Playlist shortlist curation (which of the user's Spotify playlists appear on-device)
- Hype/Rest configuration (durations, target playlists, second-press behavior,
  exercise-based-time overrides)
- Unit preference (lb/kg) and half-rep-increment toggle (see `04-workout-logging.md`)
- Auto-progression toggle (see `04-workout-logging.md`)
- Device management (claim a new device, forget a device, push WiFi networks)
- Full workout history browsing + progress/analysis dashboards, including a per-day
  "sore muscles" overview and per-exercise/workout progress graphs (see
  `04-workout-logging.md` for the analysis logic and what's shown where)
- Surfacing account-level nudges (e.g. "reconnect Spotify" as the 6-month refresh-token
  window approaches)

The device never talks to Firestore or Spotify's OAuth endpoints directly for
credentials — everything credential-related is mediated by the backend (see below).

---

## Auth model (locked)

1. **Identity**: Google login via Firebase Auth. This is the account the device gets
   claimed to, and the identity all Firestore data is scoped under (`uid`).
2. **Spotify connection**: a separate, explicit "Connect Spotify" step inside the web
   app, using Authorization Code + PKCE. The resulting refresh token is stored
   server-side only (Firestore, readable only by backend Cloud Functions — never
   returned to any client, web or device).
3. **Device claim token**: a third, independent credential — a long-lived token minted
   by the backend when a device is claimed (see `01-hardware-firmware.md`), scoped to
   that device + that account. Revocable independently of the Google login or the
   Spotify connection (forgetting a device doesn't disconnect Spotify; disconnecting
   Spotify doesn't unclaim a device).

### Spotify API constraints to design around (confirmed current, Sep 2026)

- App must run in Spotify's **Developer Mode** (no extended-quota application needed —
  that tier requires a registered business with 250k+ MAU, not applicable here).
- Developer Mode requires the **app owner's Spotify account to have an active Premium
  subscription** — already required anyway since playback control needs Premium.
- Developer Mode caps **authenticated users at 5** via an allowlist — fine for personal
  or small household use; the web app's Spotify-connect step should add the connecting
  user to that allowlist (one-time Spotify Dashboard step per new user, not automatable
  via API).
- **Refresh tokens expire 6 months after original consent**, regardless of how often the
  access token is refreshed in between. The backend tracks each user's original consent
  timestamp. **Reminder (locked, v1 scope)**: an in-app banner in the web app starting
  **2 weeks before expiry**, shown on every web app load until reconnected. Email is
  explicitly **out of scope for v1** (would need a transactional-email provider wired
  up for one low-frequency notification — not worth the setup cost yet); revisit only if
  the in-app banner turns out to be missed in practice (e.g. users who rarely open the
  web app).
- Endpoints this project needs — all confirmed live as of the most recent Spotify Web API
  changelog: playback state, currently-playing, queue, devices, play/pause/next/
  previous/seek/volume/shuffle/repeat, and reading the user's own playlists. (Endpoints
  Spotify removed in Feb 2026 — creating playlists on a user's behalf, artist top-tracks,
  new-releases, batch album/artist lookups — are not things this project needs.)
- **Rate limiting (confirmed current, Sep 2026)**: as of the July 23, 2026 policy update,
  Development Mode quota is a shared per-account budget rather than per-Client-ID, and
  429 responses carry a `reason: QUOTA_EXCEEDED` field. Backend proxy code should check
  for that reason (not just a bare 429 status) and back off per the response's
  `Retry-After` header before retrying.

---

## Backend (locked — Firebase Cloud Functions, thin layer)

A small set of HTTP-callable Cloud Functions sits between the device/web app and
Firestore/Spotify. Nothing is self-hosted — Cloud Functions, Firestore, and Firebase Auth
are all Firebase-managed.

**Why a backend layer instead of direct device→Firestore/Spotify access**: keeps Spotify
refresh tokens off the device entirely, centralizes token-refresh logic in one place,
lets Firestore security rules stay simple (only the backend's service account writes
device-originated data), and gives a natural place to run server-side analysis
(`04-workout-logging.md`) without duplicating that logic in firmware.

### API surface (locked for Phase 2 — implement directly, refine signatures/error shapes as needed but don't re-litigate the surface)

**Device-facing** (authenticated via device claim token):
- `POST /device/claim` — called by the **web app's browser session** (not the device)
  once the user confirms linking, using the claim code + their Firebase Auth ID token.
  Marks the claim fulfilled server-side and mints the device token.
- `GET /device/claim/status?code={claimCode}` — **locked mechanism (device polling)**:
  called by the device itself, unauthenticated (the claim code is the credential, and
  it's short-lived/single-use). Returns `{status: "pending"}` until `POST /device/claim`
  has been called for that code, then returns `{status: "fulfilled", deviceToken}`
  exactly once. The device polls this every ~3s while on the setup screen, with a
  client-side timeout (~10 min) after which it re-generates a fresh claim code rather
  than polling a stale/expired one forever. See `01-hardware-firmware.md` for the full
  claim-flow sequencing and rationale for choosing polling over a SoftAP-local handoff.
- `POST /device/wifi` — pushes a new `{ssid, password}` to an already-claimed, currently-
  reachable device. **Locked (resolved)**: the backend relays the password to the device
  in this single request and does not persist it — see the schema note below.
- `GET /device/config` — pull current settings, cached template(s), Hype/Rest config,
  saved WiFi networks
- `POST /device/logs` — push pending workout log entries; returns ack per entry so the
  device can clear its local pending-sync queue
- `POST /device/heartbeat` — battery level, firmware version, last-seen (optional, for
  future OTA gating / "device needs charging" web app prompts)
- `POST /spotify/*` — thin proxy endpoints for playback commands. **Locked**: the
  backend always proxies; it never hands the device a Spotify access token, short-lived
  or otherwise. See `01-hardware-firmware.md` for the full rationale.

**Web-app-facing** (authenticated via Firebase Auth ID token):
- Standard CRUD over templates/splits/workouts/exercises
- Playlist shortlist management (list the user's Spotify playlists via backend-proxied
  call, save the chosen shortlist)
- Hype/Rest settings CRUD
- Device management: initiate claim, list claimed devices, forget a device, push a new
  WiFi network to a device
- Spotify connect/disconnect/reconnect (PKCE flow endpoints)
- Workout history + analysis queries (see `04-workout-logging.md`)

### Sync model (locked)

**Polling, not real-time push.** The device polls `GET /device/config` **on wake, on
reconnect, and every 5 minutes while idle-and-connected** (concrete interval, locked —
frequent enough that a web-app edit shows up on-device within a workout-relevant
timeframe, infrequent enough not to matter for battery/backend load) rather than holding
a persistent connection or using FCM push. This is simpler to implement on both ends and
matches the "reconcile whenever connected" philosophy already locked for offline-first
behavior. **Deferred, not v1**: an FCM-triggered config-changed push for the
low-latency case ("I just edited my workout and want it on the device now") — the user
can always back out to Home and back in, or wait up to 5 minutes, in v1; this isn't worth
the added infrastructure (FCM setup, device-side push handling) until it's proven to
matter in practice.

---

## Firestore schema (locked for Phase 2 — implement directly)

```
users/{uid}
  spotify: { connected: bool, consentedAt: timestamp, ... }  // refresh token NOT here, see subcollection
  homeScreenPreference: "combined" | "music" | "workout"
  unitPreference: "lb" | "kg"              // default "lb"
  halfRepIncrementEnabled: bool            // default false — see 04-workout-logging.md
  autoProgressionEnabled: bool             // default false — see 04-workout-logging.md

users/{uid}/spotifyAuth/{singleton}   // backend-only read/write
  refreshToken, accessToken, expiresAt, consentedAt

users/{uid}/devices/{deviceId}
  claimedAt, deviceToken (hashed), name, lastSeen, batteryPct, firmwareVersion
  savedNetworks: [ { ssid, priority } ]   // locked: passwords are NEVER persisted here —
                                          // see "WiFi password handling" below

users/{uid}/splits/{splitId}
  name, order

users/{uid}/splits/{splitId}/workouts/{workoutId}
  name, order, exerciseIds: [ ... ]

users/{uid}/exercises/{exerciseId}
  name, targetMuscles: [ ... ], warmupSets: [ {weight,reps} ], workSets: [ {weight,reps} ]
  targetHypeSeconds, targetRestSeconds   // nullable, per Hype & Rest "exercise-based time"

users/{uid}/sessions/{sessionId}
  workoutId, startedAt, endedAt, status
  loggedSets: [ { exerciseId, setNumber, weight, reps, loggedAt } ]

users/{uid}/playlistShortlist
  [ { spotifyPlaylistId, name } ]

users/{uid}/hypeRestConfig
  hype: { durationSeconds, playlistId, exerciseBasedTime, secondPressBehavior }
  rest: { durationSeconds, playlistId, exerciseBasedTime, secondPressBehavior }
```

**WiFi password handling (resolved — transient, never persisted server-side).** When the
web app pushes a new network to an already-claimed device (the hybrid provisioning
flow), the plaintext password travels in the body of `POST /device/wifi`, the backend
forwards it to the device over that same request/response cycle, and the device
acknowledges receipt — the backend does **not** write the password to Firestore at any
point, before or after the ack. Only `{ssid, priority}` is persisted (in
`savedNetworks` above), matching what's needed to show the saved-network list in the web
app and on the device's Settings — Network screen. The device itself still stores the
password persistently in its own on-device flash (unchanged, required for offline
reconnection — see `01-hardware-firmware.md`) — that's a separate, already-locked piece
of local storage, not affected by this decision. This mechanism requires the device to
already have *some* working connection to receive the push in the first place; it's not
a substitute for the SoftAP first-boot flow, only for adding networks afterward.

---

## Open questions to resolve during implementation

All decisions previously flagged in this doc are now locked (WiFi password handling,
reconnect-Spotify reminder lead time/channel, claim-token handoff mechanism — see above
and `01-hardware-firmware.md`). What's left is routine implementation work, not
decisions a coding agent should stop for:

- Firestore security rules — schema above is now locked, so rules can be written
  directly in Phase 2: only the backend service account (via Cloud Functions) writes
  device-originated fields (`lastSeen`, `batteryPct`, `loggedSets`, etc.); each `{uid}`
  subtree is readable/writable only by that authenticated user's own ID token for the
  web-app-facing fields, and never client-writable for `spotifyAuth` or `deviceToken`.
