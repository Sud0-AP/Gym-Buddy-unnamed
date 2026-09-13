# Project Overview — Gym Companion Device (v2: WiFi + Firebase Architecture)

## What this is

A small, pocketable, tactile hardware device used inside the gym as a **distraction-free
remote control for music (via Spotify) and a standalone workout tracker/logger**.

This is a **pivot** from the original design. The device no longer pairs with a native
phone app over BLE — instead it connects to **WiFi directly** and syncs to a
**web app + Firebase backend**, which proxies all Spotify Web API calls on the device's
behalf (the device itself never talks to Spotify directly — see `01-hardware-firmware.md`
for the locked call shape). See
`06-implementation-plan.md` for what carries over from the original BLE-based prototype
work (most hardware bring-up) and what needs redoing (display, firmware framework, all
connectivity).

The core design philosophy is unchanged: **the phone stays in the pocket/bag.**
Everything needed during a focused workout — music control, hype/rest timers, guided
workout logging, progress stats — lives on the device itself, usable standalone once
configured. Configuration and long-term history live in a web app reachable from any
device, not tied to one paired phone.

This is a personal project first, with an explicit long-term goal of becoming an
**open-source hardware + firmware + web app project** others can fork, extend, or
rebuild with different components.

---

## What changed vs. the original design (summary)

| Area | Original (v1) | Now (v2) |
|---|---|---|
| Device connectivity | BLE to a native phone app | WiFi, direct to internet |
| Spotify access | Phone app relays commands | Backend proxies all Spotify Web API calls; device never calls Spotify directly |
| Config/companion surface | Native iOS/Android app | Web app |
| Auth | Spotify OAuth only (app-side) | Google login (Firebase Auth) + separate Spotify OAuth (PKCE) |
| Backend/database | App's local + eventual sync target unspecified | Firebase (Firestore) via a thin Cloud Functions API layer |
| Firmware framework | ESP-IDF native | Arduino-ESP32 core (FreeRTOS still explicit) |
| Display | 1.83" 240×280 | 2.4" 240×320 (driver TBD: ILI9341 or ST7789) |
| Display library | Custom/vendor demo init | TFT_eSPI (reuses Lopaka-generated screen code) |
| Volume control | Phone system volume (Android silent / iOS HUD) | Spotify Connect volume via backend-proxied API call — see `05-music-control.md` |

Everything else — the physical button layout, the encoder-driven navigation model, the
Hype/Rest flagship feature, offline-first workout logging, the design system — carries
forward unchanged in spirit; see the individual docs for exact detail.

---

## Core hardware (unchanged from original bring-up, display swapped)

| Component | Part | Notes |
|---|---|---|
| MCU | Seeed Studio XIAO ESP32S3 | WiFi + BLE-capable radio (BLE unused in v2), runs Arduino-ESP32/FreeRTOS |
| Display | 2.4" SPI TFT, 240×320, driver TBD (ILI9341 or ST7789) | Landscape orientation, taller than the original 240×280 panel |
| Scroll input | M274 360° rotary encoder (with push button) | Primary menu navigation, native GPIO |
| Buttons | 5x push buttons | 3 fixed music controls + 2 dual-purpose (Hype/Rest), on I2C expander |
| Power switch | 3-way switch (2P2T/DPDT) | Off / On / Lock |
| IO expansion | PCF8574T I2C 8-bit GPIO expander | Confirmed working during initial hardware bring-up |
| Battery | NOVA 604060 LiPo, 3.7V, 2000mAh | 4–6+ hrs expected under mixed/lighter use; **≥4 hrs is the locked worst-case (screen+WiFi always-on) acceptance criterion — see `06-implementation-plan.md`** |
| Charging | TP4056 USB-C module | Single USB-C port |
| Enclosure | Custom 3D-printed case | Designed after hardware freeze |

Full detail: `01-hardware-firmware.md`.

---

## Core firmware

- **Arduino-ESP32 core**, not ESP-IDF native. FreeRTOS is already built into the Arduino
  ESP32 core, so the same explicit-task philosophy (display / input / network / app-logic
  / storage) still applies — just via Arduino APIs. **TFT_eSPI** is the display library,
  chosen specifically so the existing Lopaka-generated screen code can be reused with
  minimal adaptation.
- Device remains the **source of truth for an active workout session** — logging, timers,
  and templates work fully offline. WiFi/backend is required only for Spotify control and
  for syncing history to the cloud.
- The BLE task from v1 is replaced by a **Network task**: owns WiFi connection lifecycle,
  first-boot SoftAP + captive portal, and HTTP calls to the backend — including all
  Spotify commands, which the backend proxies to the actual Spotify Web API (the device
  itself never calls Spotify).

Full detail: `01-hardware-firmware.md`.

---

## Core web app + backend

- **Auth**: Google login via Firebase Auth for account identity, plus a separate Spotify
  OAuth (Authorization Code + PKCE) connection tied to that account.
- **Backend**: a thin **Firebase Cloud Functions** layer between the device and
  Firestore/Spotify — the device holds one long-lived device token, the backend handles
  Spotify token refresh and validates/writes to Firestore. Nothing is self-hosted.
- **Database**: Firestore holds the full workout template library, full workout history,
  playlist shortlist, and all settings. The device only ever holds a small rolling cache
  for offline operation.
- **Spotify API constraints to design around (confirmed current as of Sep 2026)**:
  playback control, current playback state, queue, and playlist reads are all available
  in Developer Mode; the app owner needs an active Spotify **Premium** subscription
  (already required for playback control anyway); Developer Mode caps authenticated users
  at **5** (fine — no need to apply for extended quota for personal/family use); and
  **Spotify refresh tokens expire 6 months after original consent**, regardless of how
  often the access token is refreshed — the web app needs to prompt for Spotify
  reconnection periodically.

Full detail: `03-web-app.md`, `05-music-control.md`.

---

## Flagship feature: Hype / Rest (unchanged)

Two dedicated buttons. Pressing either:
1. Snapshots the current Spotify playback state (track, position, queue/context,
   shuffle/repeat state).
2. Starts a user-configured playlist and a user-configured countdown timer (or a
   per-exercise "exercise-based time" override, if enabled for that button).
3. On timer expiry, restores the exact prior playback snapshot.

Second-press behavior during an active Hype/Rest, and the "exercise-based time" opt-in,
are both user-configurable per button. Full detail: `05-music-control.md`.

---

## Guiding principles for all future work on this project

1. **Device state survives disconnection.** Never design a feature that breaks if WiFi
   drops mid-workout.
2. **No text entry on-device.** All template/library creation and WiFi credential entry
   happen off-device (web app or captive portal); the device only selects, scrolls, and
   logs numeric values via the encoder.
3. **Extensible, not over-abstracted.** Modular enough to swap a component (display
   panel, input method, backend) without a full rewrite — but no speculative abstraction.
4. **Screen-based context, not gesture complexity.** Prefer "what screen am I on" logic
   over long-press/multi-press disambiguation, given no-look, mid-set usage.
5. **Platform/API limits are real constraints, not bugs.** Spotify's Developer Mode rules
   (Premium requirement, 6-month refresh token expiry, 5-user cap) and any future API
   changes get documented and designed around, not fought.

---

## Phase plan (high level — see `06-implementation-plan.md` for live status and the
full per-phase task/decision breakdown)

Sequenced by build dependency, not by feature importance:

- **Phase 1 — Hardware bring-up completion + UI lock-in**: finish display bring-up on
  the new panel (mostly carries forward from the original prototype otherwise), migrate
  to Arduino-ESP32/FreeRTOS, and get every screen in the tree fully navigable via the
  real physical inputs using dummy data. Locks the UI/interaction layer before any real
  data exists, so later phases are "wire in real data," not "design UI while building
  features."
- **Phase 2 — Backend + Spotify + Hype/Rest, then wire to hardware**: web app, Firebase
  project + Cloud Functions, Spotify OAuth and the backend-proxy playback calls, then
  swap Phase 1's dummy data for the real thing — live playback control, queue, album
  art, and the full Hype/Rest snapshot/trigger/restore flow, real-triggered from Home.
  The flagship feature, proven out before workout logging is layered on top.
- **Phase 3 — Workout logging**: template creation in web app, sync to device, guided
  workout flow with logging, on-device stats, offline local storage, progress
  analysis/metrics (see `04-workout-logging.md`). Independent of music/Hype-Rest, so it
  comes after the flagship feature is solid rather than before.
- **Phase 4 — Everything else**: power system validation and the hardware-freeze
  milestone, screen dim/sleep and lock/off states, remaining on-device Settings
  screens, OTA groundwork, enclosure design, and open-source packaging. Grouped last
  because none of it blocks any other phase.
