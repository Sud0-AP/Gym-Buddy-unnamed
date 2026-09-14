# Phase 1 Implementation Tickets

**Status**: 12 tickets, ordered by dependency  
**Location**: GitHub Issues [#1-#12](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues) + Local files in `.scratch/phase1/issues/`

---

## Ticket Overview

| # | Title | Blocked By | Files |
|---|-------|------------|-------|
| 01 | Project Scaffold + Display + Input Pipeline | None | [Issue #1](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/1) · [Local](../../.scratch/phase1/issues/01-project-scaffold-display-input.md) |
| 02 | App-logic Task + Nav Stack (Home → Main Menu) | #1 | [Issue #2](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/2) · [Local](../../.scratch/phase1/issues/02-app-logic-nav-stack.md) |
| 03 | Top Bar Helper + Main Menu Rendering | #2 | [Issue #3](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/3) · [Local](../../.scratch/phase1/issues/03-top-bar-main-menu.md) |
| 04 | Settings—Main + Settings—Display + Hype-as-Back | #3 | [Issue #4](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/4) · [Local](../../.scratch/phase1/issues/04-settings-main-display-back.md) |
| 05 | Settings—Brightness + Encoder Value Adjustment | #4 | [Issue #5](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/5) · [Local](../../.scratch/phase1/issues/05-settings-brightness.md) |
| 06 | Settings—Hype&Rest + Storage Task NVS Init | #5 | [Issue #6](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/6) · [Local](../../.scratch/phase1/issues/06-settings-hyperest-storage.md) |
| 07 | Home Screen + Dummy State + Global Music Buttons | #6 | [Issue #7](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/7) · [Local](../../.scratch/phase1/issues/07-home-screen-dummy-state.md) |
| 08 | Music Queue Rendering | #7 | [Issue #8](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/8) · [Local](../../.scratch/phase1/issues/08-music-queue.md) |
| 09 | Animated Selection Icons | #8 | [Issue #9](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/9) · [Local](../../.scratch/phase1/issues/09-animated-icons.md) |
| 10 | Guided Logging Screens (Overview + Mock Set Entry) | #9 | [Issue #10](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/10) · [Local](../../.scratch/phase1/issues/10-guided-logging-overview-mock.md) |
| 11 | Guided Logging Set Entry + Full Workout Flow | #10 | [Issue #11](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/11) · [Local](../../.scratch/phase1/issues/11-guided-logging-full-flow.md) |
| 12 | Full Navigation + Exit Criterion Verification | #11 | [Issue #12](https://github.com/Sud0-AP/Gym-Buddy-unnamed/issues/12) · [Local](../../.scratch/phase1/issues/12-full-navigation-exit-criterion.md) |

---

## Quick Reference per Ticket

### 01: Project Scaffold + Display + Input Pipeline
**Start**: Immediately (no blockers)  
**Delivers**: PlatformIO builds, uploads, boots. Display shows "Hello World". Serial logs semantic input events from encoder + 5 buttons.  
**Key Acceptance**:
- PlatformIO project at `Hardware/Firmware/` with TFT_eSPI (4 non-default settings), Wire, Preferences
- Display task renders solid color + text
- Input task decodes encoder (2-detent accumulator) + buttons → semantic events → 8-entry queue
- Seam 1 testing passes: rotate encoder 2 detents → `ENCODER_CW` logged

---

### 02: App-logic Task + Nav Stack (Home → Main Menu)
**Start**: After #1  
**Delivers**: Nav stack manages screen transitions. Encoder rotation on Home pushes Main Menu (logged to serial, not rendered yet).  
**Key Acceptance**:
- `NavigationState` struct: `screen_id`, `selection_index`, `scroll_offset`
- 8-entry nav stack with push/pop
- Home screen default on boot
- Encoder rotation on Home → Main Menu pushed to stack (depth=1 logged)

---

### 03: Top Bar Helper + Main Menu Rendering
**Start**: After #2  
**Delivers**: Main Menu renders on screen with 3 selection states. Encoder rotation moves selection pill. Top Bar helper extracted.  
**Key Acceptance**:
- `draw_top_bar()` helper (2-state WiFi, 5-step battery)
- Main Menu wired from Lopaka exports (layer-order verified, orphan pills wired)
- Display task calls `draw_screen()` on `DISPLAY_REFRESH_NEEDED`
- Encoder rotation increments `selection_index` mod 3 (wrapping)
- Photograph matches Lopaka reference
- **Note**: Animated icons NOT included (static frame 0 only)

---

### 04: Settings—Main + Settings—Display + Hype-as-Back
**Start**: After #3  
**Delivers**: Settings screens navigable. Hype button pops nav stack (Back action). Main Menu selection memory works.  
**Key Acceptance**:
- Settings—Main + Settings—Display wired with Top Bar
- Encoder press on Main Menu (Settings selected) → pushes Settings—Main
- Hype on Settings—Display → returns to Settings—Main → returns to Main Menu
- Main Menu still shows Settings selected after return (selection memory)

---

### 05: Settings—Brightness + Encoder Value Adjustment
**Start**: After #4  
**Delivers**: Brightness screen with encoder-adjustable value (0-100). Screen updates in real-time as encoder rotates.  
**Key Acceptance**:
- `selection_index` = brightness value (0-100)
- Encoder CW/CCW increments/decrements by 1 every 2 detents
- Screen updates immediately (slider position changes)
- Seam 4 testing: start at 50, rotate CW 20 detents → 61, CCW 4 detents → 59

---

### 06: Settings—Hype&Rest + Storage Task NVS Init
**Start**: After #5  
**Delivers**: Settings—Hype&Rest with encoder-selectable elements (selected/unselected states). Storage task initializes NVS, returns dummy defaults.  
**Key Acceptance**:
- Encoder-selectable UI elements with orphan asset states wired
- Storage task (priority 0, 4KB) initializes NVS at boot
- Storage task handles `NVS_READ` → returns dummy defaults (e.g. hype=120s, rest=180s)
- Storage task ignores `NVS_WRITE` (RAM-only Phase 1)

---

### 07: Home Screen + Dummy State + Global Music Buttons
**Start**: After #6  
**Delivers**: `dummy_state.h` with camelCase structs. Home Screen (Combined variant) with music + workout data. Album art checkerboard. Hype/Rest logos (active/inactive). Global music buttons work everywhere.  
**Key Acceptance**:
- `DummyMusicState`, `DummyWorkoutState` structs (camelCase: `trackTitle`, `artistName`, etc.)
- Home Screen wired (music block + workout block + checkerboard album art)
- Hype/Rest logos conditional (active when timer running, inactive otherwise)
- Home is default on boot
- Hype on Home → track title changes + logo activates
- Global music buttons (Previous/Play-Pause/Next) mutate state on ALL screens, don't exit screen

---

### 08: Music Queue Rendering
**Start**: After #7  
**Delivers**: Music Queue renders with dummy track list. Layer-order verified. No animation yet.  
**Key Acceptance**:
- Music Queue wired from Lopaka export (layer-order reviewed)
- Renders with Top Bar + dummy track list (3-5 tracks)
- Encoder press on Main Menu (Music selected) → pushes Music Queue
- Photograph matches reference (no layer-order bugs)
- Main Menu Music item still shows static frame 0

---

### 09: Animated Selection Icons
**Start**: After #8  
**Delivers**: Main Menu selected item animates at 5fps (200ms/frame). Animation cancels immediately on screen transition.  
**Key Acceptance**:
- Sprite frames extracted from `Animations/` as C arrays
- Display task manages 200ms timer, tracks `current_frame_index` per item
- Selected item loops frames, unselected shows frame 0
- Animation cancels immediately when screen_id changes
- Seam 4 testing: navigate to Main Menu → see Music animating, rotate to Workout → Music freezes on frame 0, Workout starts animating

---

### 10: Guided Logging Screens (Overview + Mock Set Entry)
**Start**: After #9  
**Delivers**: Guided Logging screens 1,2,3,5 fully navigable. Screen 4 (set entry) renders as MOCK (not interactive yet).  
**Key Acceptance**:
- Screens 1 (overview), 2 (exercise list), 3 (in progress), 5 (complete) wired with Top Bar
- Screen 4 (set entry) renders "10 reps @ 135 lb" STATIC (encoder doesn't change values yet)
- Dummy workout data in `dummy_state.h` (3-5 exercises)
- "Start Workout" button encoder-selectable (orphan states wired)
- Encoder press on "Start Workout" logs intent but stays on overview (flow deferred to #11)
- Exercise list encoder-selectable (selected/unselected states)

---

### 11: Guided Logging Set Entry + Full Workout Flow
**Start**: After #10  
**Delivers**: Screen 4 fully interactive (encoder changes reps/weight). "Start Workout" pushes set entry. Full flow: enter set → next set → next exercise → complete screen.  
**Key Acceptance**:
- Screen 4 has rep mode (encoder changes reps ±1/2 detents) and weight mode (±2.5 lb/2 detents)
- Encoder press in rep mode → switches to weight mode
- Encoder press in weight mode → advances to next set/exercise
- "Start Workout" now functional: pushes first exercise set 1 entry
- After all sets for exercise → advances to next exercise automatically
- After all exercises → pushes workout complete screen (shows total sets, volume)
- Hype mid-workout → returns to overview (session abandoned)
- Seam 4 testing: start workout → enter reps/weight → advance through 2-3 exercises → see complete screen

---

### 12: Full Navigation + Exit Criterion Verification
**Start**: After #11  
**Delivers**: All 9 screens reachable. All 4 testing seams pass. Phase 1 complete.  
**Key Acceptance**:
- All 9 screens reachable via physical inputs: Home, Main Menu, Music Queue, Settings (Main/Display/Brightness/Hype&Rest), Guided Logging (overview/set entry)
- Nav stack depth never exceeds 8
- Hype=Back on all screens except Home (where it triggers Spotify Hype)
- Main Menu selection memory works
- **Seam 1 (Input)**: 100% coverage — all event types, 2-detent accumulation, burst test
- **Seam 2 (App-logic)**: All nav transitions, Hype/Rest dual-mode, global music buttons
- **Seam 3 (Rendering)**: All 9 screens photographed, match Lopaka references, no layer-order bugs
- **Seam 4 (Interaction)**: Encoder scrolling/wrapping, value adjustment, navigation depth, animation transitions, global music interaction
- **Phase 1 Exit Criterion**: Every criterion from spec met (see local ticket for full checklist)

---

## Implementation Strategy

**Linear flow**: Work #1 → #2 → #3 → ... → #12 in order. Each ticket is a vertical slice delivering demoable end-to-end behavior.

**Per-ticket workflow**:
1. Read the ticket (GitHub Issue or local `.md` file)
2. Implement acceptance criteria
3. Test on hardware (serial monitor + photographs)
4. Mark checkboxes complete
5. Move to next ticket

**Testing checkpoints**:
- **Seam 1 (Input)**: After #1 — verify semantic events log correctly
- **Seam 2 (App-logic)**: After #2, expanded in #4-#11 — verify nav stack state transitions
- **Seam 3 (Rendering)**: After #3, every screen thereafter — photograph vs Lopaka references
- **Seam 4 (Interaction)**: After #3, every interactive screen — verify encoder/button behavior

**Orphan assets**: Identified per-screen during implementation (user guides which PNG goes where). Convert via `image2cpp` tool, wire with conditional logic in `draw_screen()`.

**Hardware**: XIAO ESP32S3 + ST7789 display + rotary encoder (D6/D7) + PCF8574T expander (5 buttons) + encoder push-button

---

## Cross-References

- **Phase 1 Spec**: [`docs/phase1-spec.md`](../phase1-spec.md) — Full technical specification (75 user stories, implementation decisions, testing seams)
- **Domain Model**: [`CONTEXT.md`](../../CONTEXT.md) — 29 locked decisions from grilling interview
- **Context Docs**: [`Context Documents/`](../../Context%20Documents/) — 7 locked planning docs (hardware, UI, web app, workout logging, music control, implementation plan)
- **Lopaka Screens**: [`Hardware/Lopaka Screens/`](../../Hardware/Lopaka%20Screens/) — User-authored screen layouts (9 screens exported as `.txt` + `.png` pairs)
- **Unit Tests**: [`Hardware/Unit Test/`](../../Hardware/Unit%20Test/) — Reference implementations (display init, encoder decode)

---

## Phase 1 Exit Criterion

Phase 1 is **complete** when:
1. Every screen in the locked screen tree (9 screens) is reachable via physical inputs
2. All screens render correctly (visually matching Lopaka references, no layer-order erasure)
3. Navigation works per `02-hardware-ui.md` spec (Hype=Back outside Home, encoder rotate/press, nav stack push/pop, Main Menu selection memory)
4. All dummy data renders correctly (track title, artist, workout name, set/rep counts, brightness value)
5. Hype/Rest dual-mode behavior works (Home → dummy Spotify trigger with visual feedback, other screens → Back/Info)
6. Global music buttons work everywhere (Previous/Play-Pause/Next mutate dummy state without exiting current screen)
7. Animated selection icons work (200ms/frame, cancel on transition)
8. All 4 testing seams pass (Input queue events, App-logic nav state, visual rendering, interaction sequences)

All exit criteria verified by Ticket #12.
