# Hardware + Firmware — Architecture

## Bill of materials

| Part | Role | Notes |
|---|---|---|
| Seeed Studio XIAO ESP32S3 | MCU + WiFi radio | ~11 usable native GPIO; some shared with SPI flash — budget carefully. BLE radio present on-chip but unused in v2. |
| 2.4" SPI TFT, 240×320, IPS | Display | Landscape orientation. **Driver confirmed on arrival: ST7789** (not ILI9341). Taller than the original 240×280 panel — see `02-hardware-ui.md` for layout implications. Full wiring + TFT_eSPI config now locked, see "Display bring-up" section below. |
| M274 360° rotary encoder w/ push button | Primary scroll/select input | Quadrature (A/B), native GPIO with interrupts — unchanged from original bring-up |
| 5x push buttons | Music controls (3 fixed) + Hype/Rest (2, context-dependent) | On I2C expander — unchanged |
| 3-way switch, 2P2T (DPDT), ON-OFF-ON | Off / On / Lock | Unchanged |
| PCF8574T | I2C 8-bit GPIO expander | Unchanged, confirmed working at address 0x20 |
| NOVA 604060 LiPo, 3.7V, 2000mAh, protected | Power | Unchanged |
| TP4056, USB-C variant | Charging | Unchanged |
| Enclosure | Custom 3D-printed case | Unchanged — designed after hardware freeze |

Pin budget, power system, and the confirmed pin mapping from the initial hardware bring-up are
unchanged from the original hardware doc **except** the display SPI pins, which need
re-verification once the new panel's driver chip is confirmed (ILI9341 and ST7789 have
different init requirements, though pinout can likely stay the same). Everything else —
encoder pins, expander pins, button mapping — carries forward as-is.

**Antenna**: no change. The XIAO's stock U.FL stick antenna covers WiFi at the same
short range it covered BLE; no separate antenna needed.

---

## Display bring-up (confirmed, Phase 1)

**Driver: ST7789** (240×320, native portrait dimensions — TFT_eSPI rotates to landscape
at runtime via `setRotation()`, not via `TFT_WIDTH`/`TFT_HEIGHT`).

**Confirmed wiring:**

| Display pin | XIAO pin | GPIO |
|---|---|---|
| RST  | D3  | GPIO4 |
| DC   | D1  | GPIO2 |
| CS   | D0  | GPIO1 |
| SCK  | D8  | GPIO7 |
| MISO | D9  | GPIO8 |
| MOSI | D10 | GPIO9 |
| LED (backlight) | 3V3 | — always on, no GPIO control in v1 |
| VCC | 3V3 | |
| GND | GND | |

This uses the XIAO ESP32S3's native hardware SPI pins (D8/D9/D10), not software SPI.

**TFT_eSPI `User_Setup.h` — confirmed-working settings beyond the driver/pin defines
above.** These four were not optional extras; without them the panel either stayed
blank white or showed corrupted/shifted content, so treat all four as required for
this specific panel + MCU combination, not just performance tuning:

- `TFT_RGB_ORDER TFT_BGR` — this panel's native colour order, despite ST7789 boards
  commonly defaulting to RGB elsewhere; confirmed by testing the project's actual
  accent color (`0xE33F`) and checking it rendered as magenta, not green.
- `TFT_INVERSION_OFF` — explicit, not just "leave default."
- `CGRAM_OFFSET` — **required fix for corrupted/shifted rendering.** This specific
  ST7789 panel's controller RAM window is larger than the visible glass; without this,
  content renders shifted/noisy rather than aligned to the real display area. If you
  ever swap to a different ST7789 panel and see similar corruption, check this first.
- `USE_HSPI_PORT` — **required to avoid a driver-level failure**, not a performance
  setting. Default SPI port selection failed to initialize the panel correctly on this
  ESP32S3 + TFT_eSPI combination; switching to the alternate SPI port resolved it.
  Physical pins are unaffected (ESP32S3's GPIO matrix allows any pins on any SPI bus).

Also confirmed necessary just to get the library to pick up any config changes at
all: **verify which `User_Setup.h` is actually being compiled** before assuming a
rendering problem is a wiring or settings problem. Two failure modes to check first if
a change to `User_Setup.h` doesn't seem to take effect:
- A duplicate/stale TFT_eSPI install elsewhere on the machine (Library Manager install
  + a manual copy is a common cause).
- `User_Setup_Select.h` (in the TFT_eSPI library root) pointing at a different setup
  file than the one being edited.

The library's own `File > Examples > TFT_eSPI > Generic > Read_User_Setup` sketch
prints the actual compiled driver/pins to Serial and is the fastest way to confirm
which of the two failure modes (config not applied vs. genuine wiring/hardware issue)
you're looking at, before spending time on the other one.

The full confirmed-working `User_Setup.h` is kept alongside firmware source, not
reproduced inline here, since it's a drop-in file rather than something to hand-copy
from a doc.

---

## Firmware platform (locked)

**Arduino-ESP32 core**, not ESP-IDF native. FreeRTOS ships built into the Arduino ESP32
core, so the same task-separation philosophy as the original plan still applies — just
through Arduino APIs (`xTaskCreate`, queues, semaphores) instead of raw ESP-IDF calls.
This trades a small amount of low-level control for being able to reuse the existing
Lopaka-generated screen code via **TFT_eSPI**, which is the specific reason for this
framework choice.

**TFT_eSPI** is the display library. It's chosen because the Lopaka-exported screen code
(from the original device) targets a TFT_eSPI-shaped API surface, so screens can be
ported with adaptation rather than a full rewrite.

---

## FreeRTOS task architecture (starting point — refine during Phase 1)

- **Display task** — owns the SPI bus and all rendering, including the sprite-frame
  animation loop for the selected menu icon and any in-progress marquee/carousel
  animations. Nothing else touches the display directly; other tasks post "render this
  screen/state" events to it.
- **Input task** — polls the PCF8574T over I2C (buttons, encoder push, 3-way switch) and
  services native GPIO interrupts for the rotary encoder A/B channels. Debounces and
  translates raw input into semantic events (`ENCODER_CW`, `BUTTON_HYPE_PRESS`,
  `SWITCH_LOCK`, etc.) posted to a shared input event queue. Unchanged from the original
  plan, including the confirmed single-CLK-interrupt encoder decode method.
- **Network task** (replaces the old BLE task) — owns:
  - WiFi connection lifecycle (connect to saved network, reconnect on drop)
  - First-boot SoftAP + captive portal server for initial WiFi credential entry
  - HTTP client calls to the backend (Cloud Functions API) for config pull, log push,
    device claim status, **and all Spotify playback control/state** (see the locked
    call-shape decision below — the device never calls Spotify directly)
  - Never touches Firestore or the Spotify Web API directly — always goes through the
    backend layer (see `03-web-app.md`)
- **App logic / state machine task** — owns menu navigation state, current screen,
  Hype/Rest timer state, and workout session state. Consumes input events and network
  responses, decides what changes, posts render events to the display task and outbound
  requests to the network task. This is the "brain" task — unchanged in role from the
  original plan.
- **Storage task** (or protected module) — reads/writes on-device flash for settings, the
  cached workout template(s), and the pending sync queue.
- **Power management** — screen dim/sleep timers, lock-state handling, deep-sleep logic
  for the Off transition if hardware allows it.

This is a starting decomposition, not a rigid final spec — expect refinement once Phase 1
implementation begins.

---

## Menu navigation model (unchanged — locked)

- Rotary encoder rotation = primary scroll through any list/menu.
- Rotary encoder push = primary select/confirm action.
- 3 of the 5 push buttons are fixed, global music controls, reachable from any screen.
- The remaining 2 buttons (Hype/Rest) are context-dependent on the current screen: real
  Hype/Rest trigger **on the Home screen only**; every other screen (Guided Logging,
  Music Queue, Settings, menu/list screens) treats them as Back/Info. See
  `02-hardware-ui.md` for the full per-screen behavior spec.

---

## Lock / Off state handling (unchanged — locked)

- **Off** = hardware power cut, no firmware state to manage.
- **On** = normal operation, all tasks active.
- **Lock** = software state. Display blanks/sleeps. Input task keeps reading the switch
  position (to detect Lock → On) but the app logic task discards other input events while
  locked. Any in-progress timer (Hype, Rest) is explicitly exempted from this freeze —
  timers keep ticking, the network connection stays alive if present, workout state keeps
  progressing regardless of lock state.

---

## Offline-first behavior (unchanged in principle — sync target changed)

The device must be fully usable as a standalone workout tracker with **zero network
connection**. Practical implications:

- Workout state (current session, sets logged, timers) lives in RAM/flash on-device,
  never solely in the backend.
- A small local cache (recent workout template(s) + settings) is stored in on-device
  flash so a session can be started, guided, and logged with no connection at all.
- When WiFi becomes available, the device syncs newly logged data up to the backend
  (`04-workout-logging.md` / `03-web-app.md`) and can pull down updated
  templates/settings. Sync is "reconcile whenever connected," not blocking for any
  on-device action.
- Music-related features require a live connection (and, unlike BLE-to-phone, also
  require the backend to be reachable for token refresh) — should gracefully show as
  unavailable rather than error when disconnected.

## Data & storage (on-device)

No SD card. On-device flash (NVS or LittleFS/SPIFFS) stores:
- User settings (Hype/Rest durations, second-press behavior, home screen preference,
  theme, saved WiFi networks)
- Cached workout template(s): exercises, target sets/reps/weight, target
  hype/rest-seconds, last-session and PR data for the Stats screen
- Device claim token / backend auth credential (see WiFi & claiming flow below) —
  persisted so normal reboots never require re-scanning a QR code
- Pending sync queue: workout logs recorded while disconnected, held until the backend
  confirms receipt

Full workout history, the full template library, and all long-term analytics live in
Firestore — the device only ever holds a rolling working set.

---

## WiFi provisioning flow (locked — hybrid model)

1. **First-ever setup**: device boots with no saved network, starts a SoftAP + captive
   portal. User connects a phone/laptop to the device's AP; the captive portal serves a
   simple web form to enter an SSID + password. Device attempts to connect, saves the
   credential to flash on success.
2. **Adding/updating networks later**: once the device has been claimed to an account
   (see below) and has any working connection, additional networks can be entered in the
   **web app** and pushed down to the device via the backend, so the user doesn't need to
   re-enter SoftAP mode every time they want to add a network (e.g. a new gym's WiFi).
3. **Reconnection (locked)**: normal boot tries saved networks in **most-recently-used
   order**. If none are reachable, the device falls back to workout-only offline
   operation. It does **not** automatically re-enter SoftAP mode on a simple connection
   failure (that would risk accidentally dropping into setup mode away from home WiFi).
4. **SoftAP re-entry policy (locked)**: SoftAP mode is entered automatically only when
   **no networks are saved at all**. On repeated failure with saved networks present
   (5 consecutive failed boot-time connection attempts, across any combination of saved
   networks), the device does **not** auto-enter SoftAP — instead it shows a
   non-blocking prompt on the Home/top-bar area ("Can't reach saved WiFi — open Settings
   › Network to add a network") and continues in offline mode. SoftAP is otherwise only
   ever entered manually via that Settings action. This avoids the accidental-dropped-
   into-setup-mode risk while still surfacing the problem instead of failing silently.

**WiFi password persistence — two separate things, don't conflate them:**
- **On-device (locked, unchanged)**: the device's own saved-network list (SSID +
  password) is stored **persistently in on-device flash** — this is required for the
  device to reconnect with zero network/backend access, per the offline-first
  requirement. Nothing about this changes.
- **Backend/Firestore, for the web-app-push path only (resolved — transient, not
  persisted)**: when a network is pushed from the web app to an already-claimed device
  (see `03-web-app.md`), the backend relays the password to the device over that one
  request and does **not** keep a permanent copy in Firestore afterward — only
  `{ssid, priority}` is persisted server-side (matching the schema already in
  `03-web-app.md`), never the plaintext password. The device is the only place the
  password lives long-term.

## Device claiming / account linking flow (locked — QR primary, code fallback)

1. On first boot (or after a factory reset / "forget account" action), the device has no
   claim token and shows a **setup screen**: a QR code encoding a setup URL with a
   short-lived device-specific claim code, plus the same code rendered as on-screen text
   as a fallback if the user can't scan (no camera handy, etc.).
2. User opens the URL (or manually enters the code in the web app), signs in with Google
   if not already, and confirms linking that device to their account.
3. Backend issues a long-lived device token, which the device stores in flash and never
   shows the claim screen again on normal reboot. **Claim-token handoff (locked):
   device polling.** After showing the QR/code, the device polls
   `GET /device/claim/status?code={claimCode}` on a short interval (e.g. every 3s,
   reasonable client-side timeout/retry-backoff after ~10 min with no claim). Once the
   browser-side confirmation completes, the backend marks the claim fulfilled and the
   status endpoint returns the device token, which the device stores to flash and
   transitions off the setup screen. Chosen over SoftAP-local handoff because it reuses
   the same HTTP-polling pattern already used for `GET /device/config` (no separate
   local-network protocol to build/debug), and because the device already has a working
   WiFi connection by the time it reaches this screen (WiFi provisioning happens first —
   see the sequencing note in the Device Claim / Setup screen entry below), so there's no
   local-network constraint forcing a SoftAP-based mechanism.
4. **"Forget device"** (Settings) clears the stored token and returns the device to the
   claim-screen state — this is the same mechanism used if the device is ever
   re-purposed for a different account.

## Spotify OAuth + token handling on-device (locked — backend-mediated)

- The device **never** stores Spotify client credentials, a raw Spotify refresh token,
  or a Spotify access token of any kind — not even a short-lived one.
- Spotify OAuth (Authorization Code + PKCE) is performed **in the web app**, tied to the
  user's Google-authenticated account; the resulting Spotify refresh token is stored
  server-side (Firestore, access restricted to backend functions only).
- **Call shape (locked): backend proxy, not token handoff.** When the device needs to
  make a Spotify call, it calls the backend with its device token and the generic
  command (e.g. "next track," "set volume to N"); the backend attaches a fresh Spotify
  access token (refreshing via Spotify's token endpoint as needed — no client secret
  required under PKCE), makes the actual call to Spotify, and returns the result to the
  device. The device's only external relationship is to the backend — one host, one auth
  token, one error-handling path in firmware. Spotify-specific error codes, rate limits,
  and any future Spotify API changes are handled server-side (a backend deploy) rather
  than requiring a firmware OTA to patch. See `03-web-app.md` for the `/spotify/*` proxy
  endpoint surface and `05-music-control.md` for how commands map to it.
- Spotify refresh tokens expire **6 months** after original consent regardless of use;
  the web app surfaces a "reconnect Spotify" in-app banner starting 2 weeks before that
  window (locked — email is out of scope for v1; see `03-web-app.md`).
- **"Forget Spotify" / re-auth**: a web app action that revokes the stored connection and
  re-starts the PKCE flow; does not affect device claiming (a device stays linked to the
  account even if the Spotify connection is disconnected/reconnected).

---

## UI rendering — animated selection icons (unchanged)

Menu items' icons play a sprite-frame animation loop when selected, static frame 0 when
not. Timer-driven partial redraw of the active icon's region, not a full-screen redraw
per animation frame. See `02-hardware-ui.md` for the full visual pattern spec.

## OTA (future phase, unchanged plan)

Not in the initial phases. Standard ESP-IDF-style OTA partitioning is available under
Arduino-ESP32 too (`ota` library) — no special action needed now beyond not actively
working against it.

## Extensibility notes (for open-source goal, unchanged philosophy)

- Keep the display task's rendering calls behind a small internal interface
  (`draw_screen(screen_id, state)`) rather than scattering raw TFT_eSPI calls through
  business logic.
- Keep input events semantic (`ENCODER_CW`, `BUTTON_HYPE_PRESS`) rather than raw
  pin-level.
- Keep the network task's backend-calling code behind a small internal interface. There
  is no separate Spotify-calling code on-device to abstract (all Spotify calls are
  backend-proxied, see above) — the actual anticipated swap point is the backend/music
  service itself, which lives entirely server-side and can change without touching
  firmware at all.

---

## Open questions to resolve during implementation

All prior open decisions in this doc are now locked (claim-token handoff, WiFi password
persistence, SoftAP re-entry policy, network priority order, display driver chip + pin
mapping + TFT_eSPI config — see above). Nothing remains here that should block a coding
agent from writing code.
