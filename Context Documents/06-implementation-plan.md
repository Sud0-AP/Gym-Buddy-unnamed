# Implementation Plan — Gym Companion Device (v2)

## How to use this doc

- Add concrete tasks under a phase as checkboxes (`- [ ]`) as they come up — this doc
  grows organically, it isn't meant to be fully planned upfront.
- Check items off (`- [x]`) as completed; leave a short inline note if something needed
  a decision or deviated from a doc (and update the relevant project doc itself if it's
  a locked decision, not just a task).
- Keep **Current status** at the top up to date — fastest way to see where things stand.
- The **Key decisions & learnings log** at the bottom is a running changelog of anything
  that corrected or added to the other docs — one line + which doc it landed in, full
  detail lives in that doc.
- **Phase order here is deliberately sequenced by dependency, not by the numbering in
  earlier drafts of this project**: UI/interaction has to be real and navigable before
  it's worth wiring to live data; music/Hype-Rest has to be real before workout logging
  is added on top (they don't depend on each other, but music is the flagship feature
  and gets proven out first); everything that doesn't block anything else (battery
  validation, polish, enclosure) comes last. See the rationale line under each phase
  heading below.
- **Before starting any phase**, resolve every item under that phase's "Must decide
  before starting" list. These are called out separately from the task checklist
  because they're decisions, not implementation work — a coding agent should stop and
  ask rather than assume an answer for these.

---

## Current status

**Phase: 1 — Hardware bring-up completion + UI lock-in.** Bring-up carried over from the
pre-pivot prototype (encoder, expander, buttons) is done; display driver confirmed
(ST7789) and wiring + TFT_eSPI rendering confirmed working. First real Lopaka screen
(Music Queue) rendering correctly on-device. Remaining: Arduino-ESP32 migration,
`draw_screen()` interface, wiring the rest of the screen tree with dummy data, 3-way
switch detection.

---

## What already works (carried over from the pre-pivot prototype)

Confirmed working before the WiFi/Firebase pivot; none of it depended on BLE or the old
display panel, so none of it needs to be redone:

- [x] Rotary encoder wired to native GPIO, direction decode confirmed (single
      CHANGE-interrupt method), now on D6/D7
- [x] IO expander (PCF8574T) wired, all inputs confirmed over I2C (address 0x20)
- [x] Push buttons (5x) wired via expander — mapping: Previous=P2, Play/Pause=P4,
      Next=P0, Hype=P3, Rest=P1
- [x] Encoder push-button wired and confirmed

Everything not yet done — including the pre-pivot-deferred 3-way switch detection, the
new display bring-up, and the Arduino-ESP32 migration — is tracked once, in the Phase 1
checklist below, not duplicated here.

---

## Phase checklist

### Phase 1 — Hardware bring-up completion + UI lock-in

**Why first**: you can't validate any interaction design without a working screen, and
locking the full navigation/UI layer against dummy data now means every later phase is
"replace dummy data with real data," not "design the UI while also building the feature."

**Must decide before starting**: none — display driver chip (ILI9341 vs ST7789) is
confirmed on physical arrival, not a design decision to make in advance.

**Tasks**:
- [x] Confirm display driver chip (ILI9341 vs ST7789) on arrival, lock the TFT_eSPI
      `User_Setup.h` accordingly — **ST7789 confirmed.** Locking the config took more
      than the driver/pin defines: `CGRAM_OFFSET` and `USE_HSPI_PORT` were both required
      (not optional tuning) to get clean, correctly-positioned rendering on this
      panel/MCU combination — see `01-hardware-firmware.md`.
- [x] Display wired + rendering confirmed via TFT_eSPI — confirmed via a solid-color/
      text bring-up test, then via the first real Lopaka screen (Music Queue). Wiring
      table now locked in `01-hardware-firmware.md`.
- [ ] 3-way switch state detection confirmed for all 3 positions (deferred pre-pivot,
      still outstanding)
- [ ] Arduino-ESP32 project structure set up (migrated from ESP-IDF native)
- [ ] FreeRTOS tasks scaffolded: display, input, network, app-logic, storage (see
      `01-hardware-firmware.md` for the task responsibilities — these are already
      specified, not to be redesigned here)
- [ ] `draw_screen(screen_id, state)` interface defined (per the extensibility note in
      `01-hardware-firmware.md`)
- [ ] Lopaka screens re-mocked for 240×320 (**user-authored in Lopaka, exported as
      TFT_eSPI code** — the coding agent's job is wiring the exported drawing code
      behind `draw_screen()`, not designing layout) per the per-screen behavior spec in
      `02-hardware-ui.md` (every screen in the tree, all interaction patterns — carousel,
      windowed-list-pinned-edge, fixed-slot clamping — working end to end on dummy data;
      Settings — Network and Device Claim/Setup are still open — see their entries in
      `02-hardware-ui.md` — and can be wired up once you've designed them)
- [ ] Animated selection icon working in the render loop
- [ ] All 5 buttons + encoder + 3-way switch driving real navigation exactly per the
      global input mapping and per-screen spec in `02-hardware-ui.md` (Hype/Rest as
      Back/Info everywhere except Home, where they're stubbed — no real trigger yet,
      that's Phase 2)

**Exit criterion**: every screen in the tree is reachable and fully navigable with
dummy data, using only the physical inputs, matching `02-hardware-ui.md` exactly.

### Phase 2 — Backend + Spotify + Hype/Rest, then wire to hardware

**Why second**: this is the flagship feature and the thing that makes the device more
than a UI shell. It also has to exist before the device has anything real to poll, so
it's built and proven (web app + backend + Spotify) before being wired into the
already-locked UI from Phase 1.

**Must decide before starting**: none remaining — both previously-open decisions are now
locked. Claim-token handoff is **device polling** (`GET /device/claim/status`, see
`01-hardware-firmware.md` and `03-web-app.md`); WiFi password handling for the
web-app-push path is **transient relay only, never persisted in Firestore** (device-side
storage unchanged/unaffected — see `01-hardware-firmware.md`). Both are reflected as
constraints on the (still-open, user-designed) Settings — Network and Device Claim/Setup
screens in `02-hardware-ui.md`.

**Tasks**:
- [ ] SoftAP + captive portal WiFi provisioning working
- [ ] Firebase project set up (Auth, Firestore, Cloud Functions)
- [ ] Cloud Functions API skeleton deployed, Firestore schema implemented per
      `03-web-app.md`
- [ ] Device claim flow working end-to-end (claim-token handoff mechanism locked as
      device polling — see `01-hardware-firmware.md`)
- [ ] Dummy device↔backend round-trip working
- [ ] Spotify OAuth (PKCE) wired up in the web app
- [ ] `/spotify/*` backend proxy endpoints implemented (backend-proxy call shape is
      locked — see `01-hardware-firmware.md` — implement directly, no design decision
      needed here)
- [ ] Live Spotify playback control from the device, via the backend proxy
- [ ] Volume control via the backend proxy (no platform-specific handling needed)
- [ ] Playlist shortlist sync + picker
- [ ] Queue display (Music Queue screen live data)
- [ ] Album art fetch/cache/decode on-device (device fetches image bytes directly from
      the CDN URL the backend's proxied response returns — see `05-music-control.md`)
- [ ] Hype/Rest snapshot + timer + restore, full flow, real-triggered from Home only
      (per the locked Home-only rule in `02-hardware-ui.md`)
- [ ] Device Claim/Setup screen implemented against the locked backend behavior (claim
      polling, WiFi-first sequencing) in `01-hardware-firmware.md` once you've designed
      its layout in `02-hardware-ui.md`

**Exit criterion**: device can be claimed to an account, connect to Spotify via the web
app, and control real playback including a working Hype/Rest trigger from Home — no
more dummy data anywhere in the music path.

### Phase 3 — Workout logging

**Why third**: doesn't depend on music/Hype-Rest at all (they're explicitly independent
per `04-workout-logging.md`), so it's clean to build once the UI shell and backend
pattern are both already proven from Phases 1–2.

**Must decide before starting**: none remaining — all previously-open decisions are now
locked in `04-workout-logging.md`: full metric menu ships in v1 **including progression
suggestions** (not deferred), warmup sets are excluded from volume totals, e1RM formula
is Epley, and the encoder weight/reps in-place edit UX is concretely specified (lb-only,
±2.5 lb / ±1 rep per detent, defaults to template target or last entry).

**Tasks**:
- [ ] Template creation in web app
- [ ] Template sync to device
- [ ] Guided workout flow with logging (encoder weight/reps UX locked — see
      `04-workout-logging.md`)
- [ ] On-device stats screen (last session + PR)
- [ ] Offline local storage + sync-on-reconnect
- [ ] Server-side progress-analysis computation, scoped to whatever metric set was
      confirmed above

**Exit criterion**: a full workout session — template selection through guided logging
to stats — works standalone offline, and synced data is visible in the web app.

### Phase 4 — Everything else (power, polish, settings, enclosure, open-source)

**Why last**: none of this blocks any other phase, and several items (case design)
explicitly depend on a hardware freeze that only makes sense once the board isn't
changing anymore.

**Must decide before starting**: none blocking. Top-bar icon states (`02-hardware-ui.md`,
simplified to 2-state WiFi / 5-step battery fill), the formal nav-stack model
(`02-hardware-ui.md`), SoftAP re-entry policy (5 failed boot attempts → non-blocking
prompt, no auto-SoftAP; `01-hardware-firmware.md`), and WiFi network priority order
(most-recently-used; `01-hardware-firmware.md`) are all locked. **Settings — Network and
Device Claim/Setup screen layouts remain intentionally open** — you're designing both
yourself in Lopaka; this phase's Settings-screens task (below) just needs your design
before it can be implemented, which is a "waiting on your design," not a decision the
coding agent needs to make.

**Tasks**:
- [ ] Battery + TP4056 wiring complete
- [ ] Real current draw measured (screen on + WiFi idle, screen on + WiFi active)
- [ ] Runtime estimate recalculated and confirmed against target (**reconciled, locked**:
      **≥4 hrs is the pass/fail acceptance criterion**, measured under the worst-realistic
      case — screen on continuously + WiFi active continuously, i.e. an uninterrupted
      Guided Logging + live Spotify session, the actual worst case this device will see
      in normal gym use. "4–6+ hrs" from `00-project-overview.md` is the *expected best
      case* under lighter/mixed use — screen dim/sleep engaging between sets, WiFi idle
      between polls — not a separate target; both docs now describe the same range, just
      at different use-intensity points on it. This task fails only if the ≥4hr
      worst-case number isn't met.)
- [ ] **Hardware freeze milestone** — case design can start once this closes
- [ ] Screen dim/sleep power-saving behavior
- [ ] Lock state (timers keep running, input suppressed)
- [ ] Remaining on-device Settings screens (Theme, Home screen preference, Network)
- [ ] OTA partition scheme in place (no transport work yet)
- [ ] Case design (post hardware-freeze)
- [ ] README, build docs, cleanup for open-source release

---

## Key decisions & learnings (running log)

- **Architecture pivot**: moved from BLE + native app to WiFi + web app + Firebase.
  Rationale and full detail in `00-project-overview.md` and all other docs.
- **Backend shape**: thin Firebase Cloud Functions layer between device/web app and
  Firestore/Spotify, rather than direct device→Firestore/Spotify access — see
  `03-web-app.md`.
- **Spotify call shape locked as backend-proxy**: the device never calls Spotify
  directly and never holds any Spotify token, not even short-lived — every call is
  device → backend → Spotify. Chosen over a token-handoff model to keep all
  Spotify-specific error handling and API-change risk server-side. See
  `01-hardware-firmware.md`.
- **Volume control simplified**: backend-proxied Spotify API access removes the old
  Android/iOS platform-split problem entirely — see `05-music-control.md`.
- **Spotify device targeting**: no device picker needed, ever — Spotify's single-active-
  device model means every command implicitly targets whichever device is currently
  playing — see `05-music-control.md`.
- **Hype/Rest trigger is Home-only**: real Hype/Rest triggering only happens on the
  Home screen; every other screen treats the two buttons as Back/Info. Removes an
  earlier contradiction where Guided Logging was described as both an "active workout
  screen" (real trigger) and a Back/Info screen. See `02-hardware-ui.md`.
- **No separate "Now Playing" screen**: now-playing data (title/artist/progress) is one
  component rendered in two places — the Home screen's music block, and the fixed block
  atop Music Queue. Earlier drafts referred to "the Now Playing screen" in several docs;
  that language has been removed. See `02-hardware-ui.md` and `05-music-control.md`.
- **Phase order re-sequenced**: hardware/UI lock-in → backend+Spotify+Hype/Rest → workout
  logging → everything else (power/polish/enclosure), replacing the earlier
  0a/0b/1–6 numbering, to match actual build dependencies rather than doc-writing order.
- **Claim-token handoff locked as device polling** (`GET /device/claim/status`), chosen
  over SoftAP-local handoff since WiFi provisioning now always completes before the
  claim screen appears. See `01-hardware-firmware.md`, `03-web-app.md`,
  `02-hardware-ui.md`.
- **WiFi password persistence resolved**: device-side storage (on-device flash) was
  always locked and is unchanged; the web-app-push path for adding networks later is now
  explicitly transient — the backend relays the password once and never persists it in
  Firestore. See `01-hardware-firmware.md`, `03-web-app.md`.
- **Progress-analysis v1 scope locked as the full menu**, including progression
  suggestions (double progression model). Warmups excluded from volume, Epley is the
  fixed e1RM formula. **Revised**: both lb and kg are supported (default lb); a
  half-rep-increment toggle is available (default off); progression suggestions are
  auto-applied only if the account-wide `autoProgressionEnabled` toggle is on (default
  off — otherwise it's a review/accept prompt in the web app). See
  `04-workout-logging.md`, `03-web-app.md`.
- **Encoder weight/reps edit UX locked**: ±2.5 lb or ±0.5 kg per detent depending on
  unit preference; ±1 rep, or ±0.5 rep if half-rep increments are enabled. Defaults to
  template target or last entry (never zero). See `04-workout-logging.md`.
- **Screen layout for the 240×320 panel is authored directly by the user in Lopaka /
  TFT_eSPI** — not a coding-agent decision. (Supersedes an earlier draft of this doc
  that proposed a per-screen-category sizing rule; that rule has been removed since it's
  moot once every screen is hand-designed.) See `02-hardware-ui.md`.
- **Nav-stack model formalized**: screen-ID-only stack, Main Menu is the sole exception
  that remembers last selection, in-place edit modes are never stack entries, 8-entry
  fixed stack (tree depth is provably ≤7). This is the mechanism underneath the
  already-existing Home → Main Menu → (Music Queue / Workout Logs / Settings) flow in
  Part 3 of `02-hardware-ui.md`, not a change to it.
- **Settings — Network and Device Claim/Setup screens: layout intentionally left open**
  — the user is designing both directly in Lopaka. The backend-driven constraints these
  designs must satisfy (WiFi-first-then-claim sequencing, claim-status polling,
  deliberate two-step SoftAP re-entry, no on-device password entry) remain locked. See
  `02-hardware-ui.md`.
- **Top-bar icon states simplified**: WiFi icon is **2-state only** (connected/
  disconnected, no signal bars or transitional state); battery icon is a 5-step
  (0/25/50/75/100%) rectangle fill with **no color change** at any level. (Supersedes an
  earlier draft's 3-state WiFi icon and low-battery color change.) See
  `02-hardware-ui.md`.
- **Album art pipeline locked**: fetch-on-track-change only, ~300px CDN variant,
  TJpg_Decoder with scale-during-decode, RAM-only single-image cache (not persisted to
  flash). See `05-music-control.md`.
- **Hype/Rest stacking behavior revised**: only one snapshot slot exists — the
  **original pre-chain state is always the final restore target**, regardless of how
  many times Hype/Rest are pressed in sequence; a second press only swaps the active
  playlist/timer, it never re-snapshots. (Supersedes an earlier draft where the most
  recently pressed button owned the snapshot.) See `05-music-control.md`.
- **Runtime target reconciled**: ≥4 hrs (screen+WiFi always-on) is the locked pass/fail
  acceptance criterion; 4–6+ hrs is the expected figure under lighter/mixed use, not a
  separate conflicting target. See `06-implementation-plan.md`, `00-project-overview.md`.
- **Reconnect-Spotify reminder locked**: in-app banner only, 2 weeks before the 6-month
  refresh-token expiry; email explicitly out of scope for v1. See `03-web-app.md`.
- **Muscle-group taxonomy locked**: fixed 12-tag list — shoulders split into
  **shoulders-front/-side/-rear** rather than one combined tag (chest, back,
  shoulders-front, shoulders-side, shoulders-rear, biceps, triceps, quads, hamstrings,
  glutes, calves, core), multi-select in the web app, not free text. See
  `04-workout-logging.md`.
- **Display driver + wiring confirmed**: ST7789, full pin table locked, plus three
  non-default TFT_eSPI settings (`TFT_RGB_ORDER TFT_BGR`, `CGRAM_OFFSET`,
  `USE_HSPI_PORT`) required to get correctly-positioned rendering on this panel/MCU
  combination — not just performance tuning, the panel failed to render correctly
  without them. See `01-hardware-firmware.md`.
- **Lopaka export draw-order gotcha found**: a background layer (e.g. a selection
  pill's fill, the top-bar background graphic) exported to draw *after* the
  text/icons it's supposed to sit behind will silently erase that content —
  `pushImage()` repaints the whole region it covers. Fix is reordering that call to
  right after `fillScreen()`; better fix is putting background layers at the bottom
  of the Lopaka layer stack before exporting, for every future screen. See
  `02-hardware-ui.md`.
- **Analysis compute timing revised**: Cloud Functions now trigger **on write to each
  session** (not purely on-demand when the web app is opened), producing a rolling
  7-day "sore muscles" tonnage-per-muscle-group overview on the web app's Exercise
  section landing view, plus per-exercise/per-workout trend graphs on click-through.
  Needed so auto-applied progression suggestions are ready before the *next* session,
  not just whenever the web app happens to be opened. See `04-workout-logging.md`.
