# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

## 1. Repository Map

```
/Context Documents/          ← 7 planning docs (00–06) — READ-ONLY reference
/Hardware/
  /Lopaka Screens/           ← .txt (TFT_eSPI draw code) + .png per screen
                               USER-AUTHORED layouts — ground truth for what
                               draw_screen() renders; NEVER edit these files
  /Firmware/                 ← DESTINATION for all ESP32/Arduino firmware code
  /Unit Test/                ← Standalone Arduino sketches proving hardware works
                               (display init, encoder decode, expander wiring)
                               READ these before writing low-level init code
/Software/
  /Web App/                  ← DESTINATION for web app + Firebase backend code
```

**Coding destination rule (locked)**: all new firmware code goes in `Hardware/Firmware/`, all new web app code goes in `Software/Web App/`. The other three directories (`Context Documents/`, `Hardware/Lopaka Screens/`, `Hardware/Unit Test/`) are **reference-only** and must never be edited as part of implementation work.

---

## 2. Project Summary

A pocketable ESP32-based gym device with a 2.4" touchscreen that functions as a **standalone workout tracker** and **Spotify remote control**. The phone stays in the pocket — the device connects to WiFi directly and syncs to a Firebase-backed web app. Core features:

- **Hype/Rest**: snapshot/restore Spotify playback with configurable playlists and timers
- **Guided workout logging**: encoder-driven weight/reps entry, offline-first, syncs when connected
- **Music control**: play/pause/next/volume via backend-proxied Spotify Web API calls
- **Offline-first**: device is the source of truth for an active workout session; WiFi required only for Spotify and long-term history sync

Architecture: `Device ↔ WiFi ↔ Firebase Cloud Functions ↔ Firestore/Spotify`. The device **never talks to Spotify directly** — all Spotify calls are backend-proxied (locked call shape, see `Context Documents/01-hardware-firmware.md`).

**Guiding principles** (apply to all future work):
1. Device state survives disconnection — never design a feature that breaks if WiFi drops mid-workout
2. No text entry on-device — all template/library creation happens in the web app
3. Extensible, not over-abstracted — modular enough to swap a component without a full rewrite
4. Screen-based context, not gesture complexity — prefer "what screen am I on" logic over long-press/multi-press
5. Platform/API limits are real constraints, not bugs — design around them, don't fight them

---

## 3. Current Build Status

**Phase: 1 — Hardware bring-up completion + UI lock-in.**

### Phase 1 Implementation Tickets

Phase 1 has been broken down into **12 actionable vertical-slice tickets**, each delivering demoable end-to-end behavior. All tickets are published and ready for implementation.

**Ticket Locations**:
- **GitHub Issues**: [#1-#12](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues) — each labeled `ready-for-agent` with proper blocking dependencies
- **Local Files**: `.scratch/phase1/issues/01-*.md` through `12-*.md` — full acceptance criteria per ticket
- **Master Reference**: `docs/agents/TICKETS.md` — consolidated overview with quick summaries, implementation strategy, and cross-references

**Implementation Workflow**:

1. **Start with Ticket #1** (Project Scaffold + Display + Input Pipeline) — it has no blockers
2. **Work linearly** #1 → #2 → #3 → ... → #12 — each ticket blocks the next
3. **Per-ticket workflow**:
   - Read the ticket (GitHub Issue or local `.md` file)
   - Implement acceptance criteria
   - Test on hardware (serial monitor + photographs)
   - Mark checkboxes complete
   - Move to next ticket
4. **Reference `docs/agents/TICKETS.md`** for quick "what it delivers" summaries without opening full tickets

**Ticket Overview** (see `TICKETS.md` for details):

| # | Title | What It Delivers |
|---|-------|------------------|
| 01 | Project Scaffold + Display + Input | PlatformIO builds, Display shows "Hello World", Serial logs semantic events |
| 02 | App-logic + Nav Stack | Nav stack manages Home → Main Menu (logged, not rendered yet) |
| 03 | Top Bar + Main Menu | Main Menu renders on screen, encoder moves selection pill |
| 04 | Settings—Main/Display + Hype-as-Back | Settings screens navigable, Hype=Back works |
| 05 | Settings—Brightness | Encoder adjusts brightness 0-100 in real-time |
| 06 | Settings—Hype&Rest + Storage NVS | Final Settings screen + Storage task NVS init |
| 07 | Home Screen + Dummy State + Global Music | Home Screen (Combined) + global music buttons work everywhere |
| 08 | Music Queue | Music Queue renders with dummy track list |
| 09 | Animated Selection Icons | Main Menu items animate at 5fps, cancel on transition |
| 10 | Guided Logging (Mock) | Logging screens 1,2,3,5 + screen 4 mock (not interactive yet) |
| 11 | Guided Logging (Full Flow) | Screen 4 interactive + full workout start→complete flow |
| 12 | Exit Criterion Verification | All 9 screens reachable, all 4 testing seams pass |

**Testing Checkpoints** (per `docs/phase1-spec.md`):
- **Seam 1 (Input → Queue)**: After #1 — verify semantic events log correctly
- **Seam 2 (App-logic → Nav State)**: After #2, expanded #4-#11 — verify nav stack transitions
- **Seam 3 (Rendering → Visual)**: After #3, every screen — photograph vs Lopaka `.png` references
- **Seam 4 (Interaction Logic)**: After #3, every interactive screen — verify encoder/button behavior

**Phase 1 Exit Criterion** (verified by Ticket #12):
1. Every screen (9 total) reachable via physical inputs
2. All screens render correctly (match Lopaka references, no layer-order bugs)
3. Navigation works per spec (Hype=Back outside Home, encoder rotate/press, nav stack, selection memory)
4. All dummy data renders correctly
5. Hype/Rest dual-mode works (Home → Spotify dummy trigger, elsewhere → Back/Info)
6. Global music buttons work everywhere (mutate dummy state without exiting screen)
7. Animated selection icons work (200ms/frame, cancel on transition)
8. All 4 testing seams pass

**Current Status**: Phase 1 has been fully planned and broken into tickets. No tickets completed yet — start with #1.

**What works** (carried over from pre-pivot prototype):
- Rotary encoder wired to native GPIO (D6/D7), direction decode confirmed (single CHANGE-interrupt method)
- IO expander (PCF8574T) confirmed at address 0x20
- 5 push buttons wired via expander: Previous=P2, Play/Pause=P4, Next=P0, Hype=P3, Rest=P1
- Display driver confirmed: **ST7789**, full wiring + TFT_eSPI config locked in `Context Documents/01-hardware-firmware.md`
- First Lopaka screen (Music Queue) rendering correctly on-device

**Next steps** (Phase 1 remaining):
- Arduino-ESP32 migration (from ESP-IDF native)
- `draw_screen(screen_id, state)` interface scaffolded
- Full screen tree navigable with dummy data via physical inputs
- 3-way switch detection (deferred pre-pivot, still outstanding)

**Check `Context Documents/06-implementation-plan.md` at the start of every session** for the current phase checklist and what's actually done vs. still open. Update that doc (specifically the "Current status" section and the per-phase task checkboxes) as work progresses, and note any findings that correct or add to the other context docs in the "Key decisions & learnings log" at the bottom.

**Lopaka screen inventory** (cross-referenced against `Context Documents/02-hardware-ui.md` Part 3 screen tree and Part 4 per-screen spec):

Screens with exported Lopaka code (.txt + .png pairs):
- ✅ **Home Screen** (3 variants: Combined, Music Only, Workout Only) — `Hardware/Lopaka Screens/Home Screen/`
- ✅ **Main Menu** (3 selection states) — `Hardware/Lopaka Screens/Menu/menu_{1,2,3}.txt`
- ✅ **Music Queue** — `Hardware/Lopaka Screens/Music queue/music_queue.txt`
- ✅ **Guided Logging** (Workout Overview + in-session) — `Hardware/Lopaka Screens/Log Workout/log_workout_{1–5}.txt`
- ✅ **Settings — Main** — `Hardware/Lopaka Screens/Settings/settings_main.txt`
- ✅ **Settings — Display** — `Hardware/Lopaka Screens/Settings/settings_display.txt`
- ✅ **Settings — Brightness** — `Hardware/Lopaka Screens/Settings/settings_display_brightness.txt`
- ✅ **Settings — Hype & Rest** — `Hardware/Lopaka Screens/Settings/settings_hype_rest.txt`
- ✅ **Top Bar** (standard header used across screens) — `Hardware/Lopaka Screens/Top Bar/topbar.txt`
- ✅ **Animations** (icon sprite frames for selected menu items) — `Hardware/Lopaka Screens/Animations/`

Screens still unmocked/undesigned (USER must design these in Lopaka before they can be implemented):
- ⏸️ **Settings — Network** (renamed from "Pairing"; see `Context Documents/02-hardware-ui.md` Part 4)
- ⏸️ **Device Claim/Setup** (one-time screen before device has a claim token; see `Context Documents/02-hardware-ui.md` Part 4)
- ⏸️ **Settings — Theme** (mentioned in Settings — Display submenu; layout not yet designed)
- ⏸️ **Settings — Home screen preference** (mentioned in Settings — Display submenu; layout not yet designed)
- ⏸️ **Choose Split** (carousel pattern; referenced in Part 4 but no Lopaka export found yet)
- ⏸️ **Choose Workout** (carousel pattern; referenced in Part 4 but no Lopaka export found yet)
- ⏸️ **Exercise Stats** (per-exercise history/PR screen reached from Guided Logging; no Lopaka export found yet)

When a screen's Lopaka export isn't available yet, **stop and ask the user to design it** rather than inventing a layout yourself — screen layout is user-authored, not something a coding agent should make up.

---

## 4. Locked Decisions Reference

Every decision marked "locked/confirmed/resolved" across the 7 context docs, organized by subsystem. **Never re-propose or second-guess anything listed here.** One to two lines each, plus which doc has full detail.

### Display & Hardware
- **Display: ST7789, 240×320, landscape** — wiring table + 4 required non-default TFT_eSPI settings (`TFT_RGB_ORDER TFT_BGR`, `TFT_INVERSION_OFF`, `CGRAM_OFFSET`, `USE_HSPI_PORT`) locked in `01-hardware-firmware.md`. Without these the panel fails to render correctly.
- **Top bar icons: 2-state WiFi (connected/disconnected), 5-step battery (0/25/50/75/100% fill, no color change)** — see `02-hardware-ui.md` Part 1.
- **Encoder: single CHANGE-interrupt decode method** — confirmed working in Unit Test, see `Hardware/Unit Test/` and `01-hardware-firmware.md`.
- **IO expander: PCF8574T at 0x20, button mapping locked** — Previous=P2, Play/Pause=P4, Next=P0, Hype=P3, Rest=P1; see `01-hardware-firmware.md`.

### Firmware Task Architecture
- **Arduino-ESP32 core, not ESP-IDF native** — FreeRTOS via Arduino APIs; TFT_eSPI for display; see `01-hardware-firmware.md`.
- **Task split: display / input / network / app-logic / storage** — responsibilities fully scoped in `01-hardware-firmware.md`.
- **`draw_screen(screen_id, state)` abstraction** — all rendering behind this interface, not scattered raw TFT_eSPI calls; see `01-hardware-firmware.md`.
- **Semantic input events** — `ENCODER_CW`, `BUTTON_HYPE_PRESS`, etc., not raw pin-level; see `01-hardware-firmware.md`.

### WiFi & Device Claiming
- **WiFi provisioning: hybrid model** — first-boot SoftAP + captive portal; additional networks pushed from web app afterward; see `01-hardware-firmware.md`.
- **Reconnection: most-recently-used order** — no auto-SoftAP on simple failure (5 failed boots → non-blocking prompt instead); see `01-hardware-firmware.md`.
- **WiFi password persistence: two separate things** — on-device flash (unchanged, required for offline reconnect) vs. backend relay (transient, never persisted in Firestore); see `01-hardware-firmware.md`, `03-web-app.md`.
- **Claim-token handoff: device polling** — `GET /device/claim/status`, 3s interval, WiFi-first-then-claim sequencing; see `01-hardware-firmware.md`, `03-web-app.md`.

### Spotify & Backend Proxy
- **Device never calls Spotify directly** — all Spotify interactions are backend-proxied (`device → backend → Spotify`); the device never holds any Spotify credential; see `01-hardware-firmware.md`, `05-music-control.md`.
- **Backend: thin Firebase Cloud Functions layer** — device holds one long-lived device token, backend handles Spotify token refresh and Firestore writes; see `03-web-app.md`.
- **Volume control: simplified** — backend-proxied `PUT /me/player/volume`, no Android/iOS platform split; see `05-music-control.md`.
- **Spotify device targeting: no picker needed** — Spotify's single-active-device model means every command implicitly targets whichever device is currently playing; see `05-music-control.md`.
- **Reconnect-Spotify reminder: in-app banner only, 2 weeks before 6-month expiry** — email out of scope for v1; see `03-web-app.md`.
- **Album art: fetch-on-track-change, ~300px CDN variant, TJpg_Decoder, RAM-only cache** — not persisted to flash; see `05-music-control.md`.

### Hype/Rest Behavior
- **Hype/Rest trigger: Home screen only** — real Hype/Rest behavior only happens on Home; every other screen treats those two buttons as Back/Info; see `02-hardware-ui.md` Part 2.
- **Stacking: one snapshot slot** — original pre-chain state is always the final restore target; a second press swaps playlist/timer but never re-snapshots; see `05-music-control.md`.
- **Second-press behavior: user-configurable per button** — reset timer / reset+reshuffle / cancel-restore-immediately; see `05-music-control.md`.
- **Exercise-based time: opt-in per button** — sources duration from per-exercise `targetHypeSeconds`/`targetRestSeconds` field instead of fixed Settings duration; see `05-music-control.md`, `04-workout-logging.md`.

### UI Navigation & Icons
- **Nav-stack model: screen-ID-only, Main Menu remembers last selection, in-place edits are not stack entries, 8-entry fixed stack** — see `02-hardware-ui.md` Part 2.
- **Home screen: default/idle, no Back action, rotate → Main Menu, encoder press → context shortcut** — see `02-hardware-ui.md` Part 4.
- **Global music buttons: always reachable, including mid-edit** — pressing one does not exit edit mode; see `02-hardware-ui.md` Part 5.
- **Selection mechanisms: 5 distinct patterns** — bounding box swap, dot fill, scrollbar thumb, set marker, carousel; see `02-hardware-ui.md` Part 2.
- **Animated selection icons: sprite-frame loop when selected, static frame 0 when not** — see `02-hardware-ui.md` Part 1.

### Workout Logging & Metrics
- **Encoder weight/reps edit UX: ±2.5 lb or ±0.5 kg per detent (depending on `unitPreference`), ±1 rep or ±0.5 rep (if `halfRepIncrementEnabled`)** — defaults to template target or last entry, never zero; see `04-workout-logging.md`.
- **Units: lb/kg toggle (default lb), half-reps toggle (default off)** — both account-wide, web-app-only settings; device just reads from synced config; see `04-workout-logging.md`.
- **Warmup sets excluded from volume totals** — standard convention; see `04-workout-logging.md`.
- **e1RM formula: Epley** — `weight × (1 + reps/30)`; see `04-workout-logging.md`.
- **Progression: double progression model, `autoProgressionEnabled` toggle (default off)** — when off, suggestions are review prompts in web app; when on, auto-applied to template target; see `04-workout-logging.md`.
- **Plateau detection: 5 sessions with no e1RM improvement** — surfaced as nudge in web app; see `04-workout-logging.md`.
- **Analysis compute timing: on write to each session** — Cloud Functions trigger, not purely on-demand; produces rolling 7-day "sore muscles" tonnage-per-muscle-group overview; see `04-workout-logging.md`.
- **Muscle-group taxonomy: fixed 12-tag list** — shoulders split into front/side/rear, multi-select in web app; see `04-workout-logging.md`.

### Cross-Doc Consistency Rules
- **Generic command layer** — music commands stay generic ("next track," "set volume"), not shaped around Spotify's exact response payloads; see `05-music-control.md`.
- **Offline-first: first-class requirement** — device must run an entire guided workout session with zero network connection; see `01-hardware-firmware.md`, `04-workout-logging.md`.
- **No separate "Now Playing" screen** — now-playing data is one component rendered in two places (Home screen music block, Music Queue fixed block); see `02-hardware-ui.md`, `05-music-control.md`.
- **Sync model: polling, not real-time push** — device polls `GET /device/config` on wake, on reconnect, and every 5 minutes while idle-and-connected; see `03-web-app.md`.

---

## 5. Lopaka Screen Wiring Workflow

**How to turn an exported Lopaka screen into working firmware**:

1. **Locate the .txt/.png pair** in `Hardware/Lopaka Screens/` — the .txt file contains TFT_eSPI draw code, the .png is the visual reference.
2. **Check the layer-order gotcha** (confirmed during Music Queue bring-up, documented in `Context Documents/02-hardware-ui.md` Part 1): any layer that is a full-region background — selection pill fill, top bar background/margin graphic, card background — **must draw BEFORE the foreground content it sits behind**, or its `pushImage()` call silently erases text/icons. Lopaka's canvas layer order doesn't enforce this automatically. **Fix**: reorder that background call to immediately after `tft.fillScreen()` (or immediately before the first foreground element it sits behind) if the export got it wrong. **Better fix for future screens**: put every full-region background layer at the **bottom** of that screen's Lopaka layer stack before exporting.
3. **Wire the drawing code behind `draw_screen(screen_id, state)`** in `Hardware/Firmware/` — don't hardcode per-variant; screens should be parametric drawing functions taking state.
4. **Drive it with the per-screen state struct** — either real synced data or dummy data for Phase 1 bring-up.
5. **Cross-check against `Context Documents/02-hardware-ui.md` Part 4's behavior spec** for that screen — encoder rotate/press actions, button mappings, entry/exit points, selection patterns.

**Screens without Lopaka exports yet** (Settings—Network, Device Claim/Setup, Theme, Home-screen-pref, Choose Split, Choose Workout, Exercise Stats) are **waiting on user design**, not something to invent. Stop and ask the user to design them in Lopaka before implementing.

---

## 6. What's Still Genuinely Open

**Explicit user-owned items** (design work, not coding decisions):
- Settings — Network screen layout (constraints locked in `02-hardware-ui.md` Part 4, but visual design is user's)
- Device Claim/Setup screen layout (constraints locked in `02-hardware-ui.md` Part 4, but visual design is user's)
- Settings — Theme screen (mentioned in Settings — Display submenu, not yet designed)
- Settings — Home screen preference screen (mentioned in Settings — Display submenu, not yet designed)
- Choose Split/Choose Workout screens (carousel pattern specified, but no Lopaka exports yet)
- Exercise Stats screen (behavior specified in Part 4, but no Lopaka export yet)

**Implementation-level items a coding agent should resolve itself** (not blockers, just routine work):
- Firestore security rules (schema is locked in `03-web-app.md`, so rules can be written directly in Phase 2)
- Exact art-box pixel dimensions on the 240×320 Home screen (resolved automatically once the Lopaka re-mock is done; needed to pick TJpg_Decoder scale factor)

Everything else across all 7 docs is **locked** — if something isn't covered by the locked decisions above or the open items here, **go re-read the actual source doc** (named in section 4 above) rather than guessing. If neither resolves it, **stop and ask the user** rather than assuming.

---

## 7. Cross-Doc Consistency Rules

- **Hype/Rest Home-only vs. Back/Info elsewhere** — see section 4 above and `02-hardware-ui.md` Part 2.
- **Device never touches Spotify/Firestore directly** — all Spotify calls backend-proxied, all Firestore writes happen via Cloud Functions; see `03-web-app.md`.
- **Nav-stack model** — see section 4 above and `02-hardware-ui.md` Part 2.
- **One-snapshot-slot Hype/Rest stacking** — see section 4 above and `05-music-control.md`.
- **2-state WiFi icon / 5-step battery icon with no color change** — see section 4 above and `02-hardware-ui.md` Part 1.
- **Generic (non-Spotify-shaped) command layer** — see section 4 above and `05-music-control.md`.

---

## 8. Coding Conventions

- **Firmware framework: Arduino-ESP32 core** — FreeRTOS via Arduino APIs (`xTaskCreate`, queues, semaphores), not raw ESP-IDF.
- **Display library: TFT_eSPI** — with 4 required non-default settings for this panel/MCU (see section 4 above and `01-hardware-firmware.md`).
- **`draw_screen(screen_id, state)` abstraction** — all rendering behind this interface; screens are parametric drawing functions, not hardcoded per-variant.
- **Semantic input events** — `ENCODER_CW`, `BUTTON_HYPE_PRESS`, `SWITCH_LOCK`, etc., not raw pin-level.
- **Backend: Firebase Cloud Functions as thin proxy** — never hand the device a Spotify access token; always proxy the call.
- **Generic command layer for music** — commands stay generic ("next track," "play playlist X"), not shaped around Spotify's exact response payloads.
- **Offline-first** — device is the source of truth for an active workout session; WiFi required only for Spotify and long-term history sync.

---

## 9. Explicit Self-Instruction

**When something isn't covered by the locked decisions in section 4**:
1. **Go re-read the actual source doc** — section 4 names which of the 7 is authoritative for each subsystem.
2. **Check `Hardware/Unit Test/` for prior art** before writing new low-level init code — the standalone sketches there already prove specific hardware quirks work (encoder decode, expander wiring, display init sequence). Don't rediscover from scratch what's already solved.
3. If neither resolves it, **stop and ask the user** rather than assuming — this matches the project's own "must decide before starting" convention in `Context Documents/06-implementation-plan.md`.

**When asked to implement a screen**:
1. Check whether its Lopaka export exists in `Hardware/Lopaka Screens/` (see section 3 above for the inventory).
2. If it doesn't exist and it's one of the 4 unmocked screens (Settings—Network, Device Claim/Setup, Theme, Home-screen-pref), **stop and ask the user to design it in Lopaka first** — screen layout is user-authored ground truth, not something a coding agent invents.
3. If the export exists, follow the wiring workflow in section 5 above.

**When updating `Context Documents/06-implementation-plan.md`**:
- Update the "Current status" section at the top to reflect where Phase 1 actually is.
- Check off tasks as they're completed (`- [x]`).
- Add any findings that correct or extend the other docs to the "Key decisions & learnings log" at the bottom, with a one-line summary + which doc it updates.

---

## 10. Phase 1 Hardware Bring-Up Checklist (Current Phase)

From `Context Documents/06-implementation-plan.md`, Phase 1 section — this is what's actively being worked on.

**Implementation is now ticket-driven.** Do not use this checklist to guide day-to-day work — use the tickets instead (see Section 3 and `docs/agents/TICKETS.md`). This checklist maps to tickets as follows:

**Already done**:
- [x] Display driver confirmed (ST7789) + wiring + TFT_eSPI config locked
- [x] Display rendering confirmed via bring-up test and first real Lopaka screen (Music Queue)

**Remaining** (mapped to tickets):
- [ ] Arduino-ESP32 project structure set up — **Ticket #1**
- [ ] FreeRTOS tasks scaffolded: display, input, network, app-logic, storage — **Ticket #1**
- [ ] `draw_screen(screen_id, state)` interface defined — **Ticket #3**
- [ ] Lopaka screens wired behind `draw_screen()` with dummy data — **Tickets #3–#11**
- [ ] Animated selection icon working in the render loop — **Ticket #9**
- [ ] All 5 buttons + encoder driving real navigation per `02-hardware-ui.md` — **Tickets #2–#12**
- [ ] 3-way switch state detection — **Deferred** (post-Phase-1, power management)

**Exit criterion**: every screen in the tree is reachable and fully navigable with dummy data, using only the physical inputs, matching `02-hardware-ui.md` exactly. Verified by **Ticket #12**.

**Before starting Phase 2**: both remaining open decisions are already locked (device polling for claim-token handoff, transient relay only for WiFi passwords). Phase 2 can start immediately once Ticket #12 exits.

---

## 11. Tool Documentation

### Tool: Artifact

Render an HTML or Markdown file to an Artifact — a default-private web page hosted on claude.ai that the user can later choose to share with their teammates. Use this when communicating visually would be clearer than terminal text. Publishing proactively is fine for your own work-product — artifacts start private. The exception is content that could mislead or cause harm if shared onward: anything imitating a real organization, person, or record, or content the user framed as sensitive. Build those as files, and let the user decide whether they get a URL.

A finished deliverable with an audience — a report for a team, a plan other people will follow, a document meant as a reference — is not fully delivered while it lives only in terminal scrollback or a local file. Finishing such work includes publishing it as an artifact and handing the user the link, so they have a private page ready to share when they choose.

**Before writing the file — HTML and Markdown alike — you MUST load the `artifact-design` skill** to calibrate how much design investment this particular request warrants. Format is part of that decision: choose Markdown because the deliverable calls for it, never for speed. Then write the content to a file (via Write/Edit) and call Artifact with its path.

**Key capabilities**: Multi-file support (`files` map for separate CSS/JS/data/images), update existing (`url` parameter), runtime capabilities (shared database, file uploads, user identity via `artifact-capabilities` skill), comments & replies (`action: "comments"/"reply"`).

**Critical constraints**: Self-contained only (strict CSP blocks external hosts except Google Fonts), 16MB page limit, theme-aware (design for light and dark), responsive (no horizontal body scroll), favicon required (1-2 emoji, keep stable), title (short noun phrase 2-4 words, distinctive).

**Common actions**:
- Publish: `Artifact({file_path, description, favicon, title?})`
- Update: `Artifact({file_path, url, description, favicon})`
- List artifacts: `Artifact({action: "list", scope: "mine"})`
- Comments: `Artifact({action: "comments", url})`
- Reply: `Artifact({action: "reply", url, thread_id, text})`
- Database ops: `Artifact({action: "read_db"/"write_db", url, db_op, ...})`

---

### Tool: Workflow

Execute a workflow script that orchestrates multiple subagents deterministically. Workflows run in the background — this tool returns immediately with a task ID, and a `<task-notification>` arrives when the workflow completes. Use `/workflows` to watch live progress.

A workflow structures work across many agents — to be comprehensive (decompose and cover in parallel), to be confident (independent perspectives and adversarial checks before committing), or to take on scale one context can't hold (migrations, audits, broad sweeps). The script is where you encode that structure: what fans out, what verifies, what synthesizes.

**ONLY call this tool when the user has explicitly opted into multi-agent orchestration.** Workflows can spawn dozens of agents and consume a large amount of tokens; the user must request that scale, not have it inferred. Explicit opt-in means one of:
- User included keyword "ultracode" in their prompt (system-reminder confirms it)
- Ultracode is on for the session (system-reminder confirms it)
- User directly asked you to run a workflow or use multi-agent orchestration ("use a workflow", "fan out agents", "orchestrate this with subagents")
- User invoked a skill/slash command whose instructions tell you to call Workflow
- User asked to run a specific named or saved workflow

For any other task — even one that would benefit from parallelism — do NOT call this tool. Use the Agent tool for individual subagents, or describe what a workflow could do and ask the user whether to run it.

**Key patterns**:
- **pipeline()** (DEFAULT): items flow through stages independently, no barrier between stages
- **parallel()**: barrier — awaits all before continuing (use sparingly, only when genuinely needed)
- **agent()**: spawn subagent, optionally with JSON schema for structured output
- **phase()**: group agents under progress titles
- **log()**: emit narrator messages to user
- **workflow()**: run another workflow inline as a sub-step

**Quality patterns** (compose freely): Adversarial verify (spawn N independent skeptics, each prompted to REFUTE), Judge panel (N independent attempts → parallel judges → synthesize), Loop-until-dry (keep spawning finders until K consecutive rounds return nothing new), Multi-modal sweep (parallel agents each searching different way), Completeness critic (final agent asks "what's missing?").

**Default to pipeline().** Only use parallel() as a barrier when stage N genuinely needs ALL prior-stage results together (dedup across full set, early-exit if count is zero, stage N references "the other findings" for comparison).

**Script structure**: Every script must begin with `export const meta = {name, description, phases}` (pure literal). Pass the script inline via `script` — it's auto-persisted to a file. To iterate, edit that file and re-invoke with `{scriptPath}`.

**Resume**: Pass `resumeFromRunId` to resume after pause/kill/edit — unchanged agent() calls return cached results instantly.

**This session has the default workflow size guideline: medium — keep workflows under 15 agents.** This is a guideline, not a hard limit — follow it unless the user's prompt calls for a different scale.

---

## 12. Agent skills

### Issue tracker

Issues for this repo are tracked in **GitHub Issues** using the `gh` CLI. See `docs/agents/issue-tracker.md` for command conventions and wayfinding operations.

### Triage labels

Default triage label vocabulary: `needs-triage`, `needs-info`, `ready-for-agent`, `ready-for-human`, `wontfix`. See `docs/agents/triage-labels.md` for the mapping.

### Domain docs

**Single-context layout.** Before exploring the codebase, read the relevant Context Documents (`00-project-overview.md` through `06-implementation-plan.md`) in `Context Documents/` and consult CLAUDE.md's locked decisions reference (section 4). See `docs/agents/domain.md` for consumer rules and file structure.

### Phase 1 tickets

**Ordered implementation reference.** See `docs/agents/TICKETS.md` for the consolidated Phase 1 ticket overview — quick summaries, implementation strategy, testing checkpoints, and cross-references. Use this as the master reference when implementing Phase 1 work. Each ticket also exists as:
- **GitHub Issue**: [#1-#12](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues) with `ready-for-agent` label
- **Local file**: `.scratch/phase1/issues/01-*.md` through `12-*.md` with full acceptance criteria

### Artifact tool

Use the Artifact tool to publish finished deliverables (specs, reports, reference docs) as private web pages on claude.ai that can later be shared. **Always load the `artifact-design` skill BEFORE writing any artifact** (HTML or Markdown) to calibrate design investment — format is part of that decision, never a speed shortcut. Artifacts start private; the user chooses whether to share.

Key constraints:
- HTML only (no external CDN scripts/stylesheets except Google Fonts)
- 16MB page size limit, inline all CSS/JS, embed assets as data: URIs
- Theme-aware (design for both light and dark)
- Responsive (no horizontal body scroll)
- Set `<title>` and `favicon` (emoji) — keep stable across redeploys
- Update existing artifacts with `url` parameter (find with `action: "list"`)

See the Artifact tool documentation above for full details on multi-file artifacts, runtime capabilities, database, comments, and resume.

### Workflow tool

Use the Workflow tool to orchestrate multi-agent work (parallel reviews, pipeline transforms, adversarial verification). **Only call this when the user explicitly opts in** — "use a workflow", "ultracode", or invokes a skill that tells you to use it. Workflows can spawn many agents and consume significant tokens.

Key patterns:
- **pipeline()** (default): items flow through stages independently, no barrier between stages
- **parallel()**: barrier — awaits all before continuing (use sparingly)
- **agent()**: spawn a subagent, optionally with JSON schema for structured output
- **phase()**: group agents under progress titles
- **log()**: emit narrator messages to user

See the Workflow tool documentation above for quality patterns (adversarial verify, judge panel, loop-until-dry), resume behavior, and composability examples.
