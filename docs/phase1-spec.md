# Phase 1: Hardware Bring-Up and Navigation Framework

## Problem Statement

The Gym Buddy device hardware has been proven at the component level (display driver confirmed, encoder decode working, IO expander wired), but there is no firmware that ties these components together into a navigable UI. The device cannot yet render multiple screens, respond to physical inputs with correct navigation behavior, or demonstrate the full screen tree specified in the design documents.

A developer working on Phase 2 features (WiFi provisioning, Spotify integration, workout sync) needs a working navigation framework with all screens reachable via physical inputs, using dummy data to prove the rendering and interaction patterns before real backend integration begins.

## Solution

Build a complete Arduino-ESP32 firmware (PlatformIO project structure) implementing the full FreeRTOS task architecture, semantic input abstraction, screen-agnostic navigation stack, and all 9 Lopaka-designed screens wired with dummy data. The device becomes fully navigable via encoder + 5 buttons, with every screen rendering correctly and responding to inputs per the locked UI specification.

## User Stories

1. As a firmware developer, I want a PlatformIO project structure at `Hardware/Firmware/`, so that I can build and upload the firmware with `pio run` and `pio upload`
2. As a firmware developer, I want TFT_eSPI, Wire, and Preferences libraries configured in `platformio.ini`, so that display, I2C expander, and NVS functionality are available
3. As a firmware developer, I want a `src/main.cpp` entry point that initializes all 5 FreeRTOS tasks, so that the task architecture is scaffolded before I implement task bodies
4. As a firmware developer, I want Display, Input, App-logic, Network, and Storage tasks running at priorities 3/2/1/1/0 with stack sizes 4KB/2KB/4KB/8KB/4KB, so that the system matches the locked task architecture
5. As a firmware developer, I want the Input task to decode rotary encoder rotation (GPIO D6/D7, single CHANGE-interrupt method) into semantic events with 2-detent-per-selection accumulation, so that encoder turns become `ENCODER_CW`/`ENCODER_CCW` events
6. As a firmware developer, I want the Input task to decode 5 button presses (PCF8574T at 0x20: Previous=P2, Play/Pause=P4, Next=P0, Hype=P3, Rest=P1) into semantic events, so that button presses become `BUTTON_PREV_PRESS`, `BUTTON_PLAY_PAUSE_PRESS`, etc.
7. As a firmware developer, I want the Input task to decode encoder push-button presses into `ENCODER_PRESS` events, so that encoder press is a distinct input action
8. As a firmware developer, I want the Input task to post semantic input events to an 8-entry FreeRTOS queue consumed by App-logic task, so that input and application logic are decoupled
9. As a firmware developer, I want the Input task to handle simultaneous encoder press+rotate by posting both events in hardware order, so that rare edge cases don't crash the input decoder
10. As a firmware developer, I want a `NavigationState` struct with `screen_id`, `selection_index`, and `scroll_offset` fields, so that nav stack entries are screen-agnostic
11. As a firmware developer, I want an 8-entry fixed nav stack array with push/pop operations, so that screen history is tracked correctly
12. As a firmware developer, I want the App-logic task to consume semantic input events and update nav stack state, so that encoder rotation increments `selection_index` and encoder press triggers screen transitions
13. As a firmware developer, I want Hype button press on Home screen to trigger dummy Spotify Hype behavior (mutate dummy state, post `DISPLAY_REFRESH_NEEDED`), so that Home screen Hype/Rest works in Phase 1
14. As a firmware developer, I want Hype button press on non-Home screens to pop the nav stack (Back action), so that Hype is the universal Back button outside Home
15. As a firmware developer, I want Rest button press on Home screen to trigger dummy Spotify Rest behavior, so that Rest mode works in Phase 1
16. As a firmware developer, I want Rest button press on non-Home screens to be a no-op (Info action placeholder), so that Rest behavior is reserved for future Phase 2 context help
17. As a firmware developer, I want the App-logic task to post `DISPLAY_REFRESH_NEEDED` events to the Display task queue when nav state changes, so that display refresh is event-driven not polled
18. As a firmware developer, I want the Display task to call `draw_screen(screen_id, state)` when it receives `DISPLAY_REFRESH_NEEDED`, so that screen rendering is abstracted behind one interface
19. As a firmware developer, I want the Network task to exist but run an idle loop (`while(1) vTaskDelay(1000)`), so that the task structure is complete without WiFi implementation
20. As a firmware developer, I want the Storage task to initialize NVS and handle `NVS_READ` queue requests by returning dummy defaults, so that Settings screens can render in Phase 1
21. As a firmware developer, I want the Storage task to ignore `NVS_WRITE` requests in Phase 1, so that Settings changes are RAM-only until Phase 2
22. As a firmware developer, I want a `dummy_state.h` header with realistic test data structs using camelCase field names matching Firestore schema, so that Phase 2 can swap dummy data for real state without touching `draw_screen()` implementations
23. As a firmware developer, I want a `draw_top_bar(wifi_connected, battery_level)` helper function that renders the shared header (2-state WiFi icon, 5-step battery icon), so that all screens can reuse the top bar without duplicating code
24. As a firmware developer, I want Top Bar rendering code extracted from the Music Queue Lopaka export, so that the helper function draws correctly
25. As a firmware developer, I want the Top Bar Lopaka code reviewed for layer-order gotchas (background before foreground) before wiring, so that full-region backgrounds don't silently erase icons
26. As a firmware developer, I want Top Bar rendered on every screen except Home (which has its own header), so that the UI is consistent
27. As a firmware developer, I want Main Menu screen wired from `Hardware/Lopaka Screens/Menu/menu_{1,2,3}.txt` with 3 selection states, so that Main Menu renders correctly with selection pill highlighting the selected item
28. As a firmware developer, I want Main Menu selection pills reviewed for layer-order (pill background before text/icons), so that selection pills don't erase menu item text
29. As a firmware developer, I want Main Menu to render different selection pill graphics per item (orphan assets per screen context), so that each menu item has its own visual style
30. As a firmware developer, I want Main Menu `selection_index` to persist across nav stack pops (stored in `NavigationState`), so that returning to Main Menu shows the last-selected item
31. As a firmware developer, I want encoder rotation on Main Menu to increment/decrement `selection_index` modulo 3 (wrapping), so that the user can scroll through Music/Workout/Settings items
32. As a firmware developer, I want encoder press on Main Menu to push the selected submenu screen onto the nav stack, so that selecting "Music" navigates to Music Queue
33. As a firmware developer, I want Settings—Main screen wired from `Hardware/Lopaka Screens/Settings/settings_main.txt`, so that the Settings menu renders correctly
34. As a firmware developer, I want Settings—Display screen wired from `Hardware/Lopaka Screens/Settings/settings_display.txt`, so that the Display settings submenu renders
35. As a firmware developer, I want Settings—Brightness screen wired from `Hardware/Lopaka Screens/Settings/settings_display_brightness.txt` with encoder-adjustable brightness value (0-100), so that the brightness slider responds to encoder rotation
36. As a firmware developer, I want Settings—Brightness `selection_index` to represent the brightness value (0-100 range), so that encoder rotation changes brightness
37. As a firmware developer, I want Settings—Brightness screen to show the brightness value visually updating as encoder rotates, so that the user sees real-time feedback
38. As a firmware developer, I want Settings—Hype&Rest screen wired from `Hardware/Lopaka Screens/Settings/settings_hype_rest.txt`, so that Hype/Rest configuration UI renders
39. As a firmware developer, I want all Settings screens to use encoder-selectable UI elements with selected/unselected states (orphan assets), so that the currently focused setting is visually distinct
40. As a firmware developer, I want Home Screen wired from `Hardware/Lopaka Screens/Home Screen/` (Combined variant only in Phase 1) with dummy music state (track title, artist, isPlaying, progressMs, durationMs) and dummy workout state (workout name, current exercise, set/rep counts), so that Home renders with realistic placeholder data
41. As a firmware developer, I want Home Screen album art placeholder rendered as a checkerboard pattern via `drawRect()` loop, so that the art box boundary is clearly visible without requiring TJpg_Decoder integration
42. As a firmware developer, I want Home Screen to show Hype/Rest logos in active/inactive states (orphan assets) based on dummy timer state, so that Hype/Rest visual feedback works
43. As a firmware developer, I want Home Screen to be the default screen on boot (nav stack starts with `screen_id = HOME_SCREEN`, `nav_stack_depth = 0`), so that the device always starts at Home
44. As a firmware developer, I want encoder rotation on Home Screen to push Main Menu onto the nav stack, so that the user can access the menu from Home
45. As a firmware developer, I want encoder press on Home Screen to trigger a context shortcut (future feature, no-op in Phase 1), so that the encoder press action is reserved for future shortcuts
46. As a firmware developer, I want Music Queue screen wired from `Hardware/Lopaka Screens/Music queue/music_queue.txt` with dummy track list data, so that the Music Queue renders with placeholder tracks
47. As a firmware developer, I want Music Queue screen reviewed for layer-order (the fix already applied during bring-up test), so that the rendering is correct
48. As a firmware developer, I want Guided Logging screens wired from `Hardware/Lopaka Screens/Log Workout/log_workout_{1-5}.txt` with dummy exercise list, set/rep entry state, so that the workout logging UI renders
49. As a firmware developer, I want Guided Logging set/rep entry to respond to encoder rotation (±1 rep or ±2.5 lb per 2 detents, matching locked encoder sensitivity), so that the user can adjust values with the encoder
50. As a firmware developer, I want Guided Logging to use encoder-selectable UI elements with selected/unselected states for "Start Workout", "Adjust" controls, exercise list items, so that the focused element is visually distinct
51. As a firmware developer, I want animated selection icons (from `Hardware/Lopaka Screens/Animations/`) to play at 200ms per frame (5 fps) when a Main Menu item is selected, so that the menu feels responsive
52. As a firmware developer, I want animated icons to show static frame 0 when not selected, so that only the selected item animates
53. As a firmware developer, I want the Display task to cancel animation immediately on screen transition (not wait for current frame to complete), so that input responsiveness beats animation smoothness
54. As a firmware developer, I want orphan PNG assets (Hype/Rest logos, selection pills, encoder-selectable UI states) identified per screen during implementation, so that I know which assets to convert and wire
55. As a firmware developer, I want orphan PNGs converted to C arrays via automated tool (e.g. `image2cpp`), so that `pushImage()` can render them
56. As a firmware developer, I want conditional rendering logic added to `draw_screen()` for state-dependent orphan assets (active/inactive Hype logos, selected/unselected UI elements), so that graphics change based on state
57. As a firmware developer, I want all Lopaka screen exports reviewed for layer-order before upload (code review: backgrounds before foreground), so that layer-order gotchas are caught before device testing
58. As a firmware developer, I want each screen photographed on-device after upload and compared against Lopaka `.png` reference, so that rendering correctness is visually verified
59. As a firmware developer, I want the 5 physical music buttons (Previous/Play-Pause/Next) to work globally on all screens (not just Home/Music Queue), so that music control is always accessible
60. As a firmware developer, I want music button presses to mutate dummy music state and post `DISPLAY_REFRESH_NEEDED` without exiting the current screen, so that music controls work mid-navigation
61. As a hardware tester, I want to press the encoder push-button on Main Menu and see the selected screen appear, so that I can confirm encoder press navigation works
62. As a hardware tester, I want to rotate the encoder on Main Menu and see the selection pill move to the next item after 2 detents, so that I can confirm 2-detent accumulation works
63. As a hardware tester, I want to press Hype on Settings—Brightness and return to the previous screen, so that I can confirm Hype-as-Back works outside Home
64. As a hardware tester, I want to press Hype on Home screen and see the track title change (dummy Hype playlist), so that I can confirm Hype Spotify trigger works
65. As a hardware tester, I want to rotate the encoder on Settings—Brightness and see the brightness value increment, so that I can confirm encoder-driven value adjustment works
66. As a hardware tester, I want to rotate the encoder on Guided Logging set entry and see the rep count increment by 1 every 2 detents, so that I can confirm encoder sensitivity in value-edit mode
67. As a hardware tester, I want to press Play/Pause on Settings—Display (not Home/Music Queue) and see dummy music state toggle without leaving Settings, so that I can confirm global music buttons work everywhere
68. As a hardware tester, I want to navigate from Home → Main Menu → Settings → Display → Brightness, then press Hype 4 times and return to Home, so that I can confirm nav stack depth and pop behavior
69. As a hardware tester, I want to navigate Main Menu → Music, return to Main Menu via Hype, and see Music still selected, so that I can confirm Main Menu selection memory works
70. As a hardware tester, I want to see the selected Main Menu item's icon animating smoothly at ~5fps, so that I can confirm animation frame timing works
71. As a hardware tester, I want to rotate encoder from Music to Workout on Main Menu and see the Music icon freeze on frame 0 and the Workout icon start animating, so that I can confirm animation cancellation works
72. As a hardware tester, I want to see the Top Bar WiFi icon as "disconnected" and battery icon at a dummy level on every screen except Home, so that I can confirm Top Bar helper renders correctly
73. As a hardware tester, I want to see the Home Screen album art as a checkerboard pattern in the correct aspect ratio, so that I can confirm album art placeholder positioning is correct
74. As a hardware tester, I want to see different selection pill graphics on Main Menu vs Settings—Main, so that I can confirm per-screen-context orphan assets wire correctly
75. As a hardware tester, I want to see Hype logo in "inactive" state on Home screen, press Hype, and see it change to "active" state, so that I can confirm Hype/Rest visual feedback works

## Implementation Decisions

### Project Structure
- **PlatformIO project** at `Hardware/Firmware/` with `platformio.ini` + `src/main.cpp` + separate `.h`/`.cpp` per subsystem (not monolithic `.ino`)
- **Library dependencies** in `platformio.ini`: TFT_eSPI (with 4 non-default settings: `TFT_RGB_ORDER TFT_BGR`, `TFT_INVERSION_OFF`, `CGRAM_OFFSET`, `USE_HSPI_PORT`), Wire (I2C), Preferences (NVS wrapper)
- **Deferred to Phase 2**: ArduinoJson, WiFiClientSecure
- **Arduino-ESP32 core** framework, FreeRTOS via Arduino APIs

### FreeRTOS Task Architecture
- **5 tasks** created in `main.cpp`: Display (priority 3, 4KB stack), Input (priority 2, 2KB stack), App-logic (priority 1, 4KB stack), Network (priority 1, 8KB stack), Storage (priority 0, 4KB stack)
- **Display task**: Event-driven refresh (listens for `DISPLAY_REFRESH_NEEDED` on queue), calls `draw_screen(screen_id, state)`, manages 200ms animation timer for selected icons, cancels animation immediately on screen transition
- **Input task**: Decodes encoder (GPIO D6/D7) + 5 buttons (PCF8574T 0x20) + encoder push-button → semantic events (`ENCODER_CW`, `ENCODER_CCW`, `ENCODER_PRESS`, `BUTTON_HYPE_PRESS`, etc.) with 2-detent accumulator (simple counter, rollover on threshold), posts to 8-entry queue
- **App-logic task**: Consumes semantic events, manages 8-entry nav stack (`NavigationState` array), updates `selection_index`/`scroll_offset`, implements Hype/Rest dual-mode (Home screen → Spotify trigger, other screens → Back/Info), posts `DISPLAY_REFRESH_NEEDED` on state change
- **Network task**: Exists but idle (`while(1) vTaskDelay(1000)`) — scaffolded for Phase 2 WiFi/HTTP work
- **Storage task**: Init NVS at boot, handle `NVS_READ` queue (return dummy defaults for Settings), ignore `NVS_WRITE` (RAM-only in Phase 1)

### Navigation Model
- **NavigationState struct**: `uint8_t screen_id`, `uint8_t selection_index`, `uint16_t scroll_offset` — screen-agnostic, every screen interprets `selection_index` for its own selection mechanism
- **Nav stack**: Fixed 8-entry array, `nav_stack_depth` tracks current depth, push/pop operations, Main Menu `selection_index` persists across pops (stored in `NavigationState`), other screens reset `selection_index` to 0 on re-entry
- **Home screen**: Default on boot (`nav_stack_depth = 0`, `screen_id = HOME_SCREEN`), encoder rotate → push Main Menu, encoder press → context shortcut (no-op Phase 1), Hype/Rest → Spotify dummy trigger
- **Hype/Rest dual-mode**: Hype on Home → mutate `dummy_music_state.trackTitle`, start fake timer, post refresh; Hype elsewhere → pop nav stack (Back). Rest on Home → similar Spotify dummy trigger; Rest elsewhere → no-op (Info placeholder)

### Rendering Abstraction
- **`draw_screen(screen_id, state)` interface**: All rendering behind this function, takes `screen_id` enum + pointer to relevant state struct (dummy data in Phase 1)
- **`draw_top_bar(wifi_connected, battery_level)` helper**: Extracted from Music Queue Lopaka export, renders 2-state WiFi icon + 5-step battery icon, called by every screen except Home
- **Layer-order verification workflow**: Before uploading each screen, read Lopaka `.txt`, identify full-region backgrounds (top bar, selection pills, card backgrounds), verify those `pushImage()`/`fillRect()` calls appear immediately after `tft.fillScreen()`. After upload, photograph on-device and compare against Lopaka `.png` reference.
- **Screen wiring order** (simplest-to-hardest): Top Bar → Main Menu → Settings—Main → Settings—Display → Settings—Brightness → Settings—Hype&Rest → Home Screen → Music Queue → Guided Logging

### Dummy Data Strategy
- **`dummy_state.h` header**: Defines `DummyMusicState`, `DummyWorkoutState`, `DummySettingsState` structs with realistic test data
- **Field naming**: camelCase matching Firestore schema exactly (`trackTitle`, `artistName`, `isPlaying`, `progressMs`, `durationMs`, `workoutName`, `currentExercise`, `setCount`, `repCount`) — Phase 2 swaps `#include "dummy_state.h"` for `#include "real_state.h"` without touching `draw_screen()` bodies
- **Home screen variant**: Always render **Combined** variant in Phase 1 (music block + workout block), user preference deferred to Phase 2
- **Album art placeholder**: Checkerboard pattern via `drawRect()` loop (alternating filled/unfilled rectangles in a grid), no TJpg_Decoder yet

### Orphan Assets (State-Dependent Graphics)
- **Identified per screen during implementation**: Hype/Rest active/inactive logos (Home screen), selection pills (Main Menu vs Settings—Main have different graphics), encoder-selectable UI elements (Start Workout button, adjust controls, list items) in selected/unselected states
- **Conversion workflow**: Use automated tool (e.g. `image2cpp` web tool) to convert each orphan PNG to C array, add `pushImage()` calls to `draw_screen()` with conditional logic (`if (hype_active) pushImage(hype_active_logo) else pushImage(hype_inactive_logo)`)
- **Naming pattern inconsistency**: Resolve per-screen during implementation (user guides which PNG goes where), no universal pattern enforced

### Animated Selection Icons
- **Frame rate**: 200ms per frame (5 fps), managed by Display task timer
- **Frame selection**: Selected item → loop through sprite frames, unselected → static frame 0
- **Animation cancellation**: On screen transition, immediately set frame index to 0 and switch screens (don't wait for current 200ms frame to complete)
- **Sprite loading**: Extract frames from `Hardware/Lopaka Screens/Animations/` as C arrays, index by `current_frame_index`

### Input Handling
- **Encoder sensitivity**: 2 detents = 1 selection move, simple counter accumulator (`detent_accum++` on CW, `detent_accum--` on CCW, post event when `abs(detent_accum) >= 2`, then reset to 0)
- **Encoder press+rotate edge case**: Input task posts both events in hardware order, App-logic decides if one is stale (e.g. screen transition makes subsequent press invalid)
- **Global music buttons**: Previous/Play-Pause/Next work on all screens (not just Home/Music Queue), mutate `dummy_music_state`, post `DISPLAY_REFRESH_NEEDED`, do NOT exit current screen

### Unit Test Reuse
- **Display init sequence**: Copy from `Hardware/Unit Test/display_bringup_test/` into Display task init (proven TFT_eSPI setup + 4 non-default settings)
- **Encoder decode logic**: Reference `Hardware/Unit Test/` encoder sketch for single CHANGE-interrupt method (already proven on this hardware)

### Out of Scope (Deferred to Phase 2)
- WiFi provisioning (SoftAP + captive portal, network list push from web app)
- Spotify API calls (backend-proxied or otherwise)
- Device claim handoff (polling `GET /device/claim/status`)
- NVS write persistence for Settings changes
- Album art fetch/decode (TJpg_Decoder + HTTP + RAM cache)
- Workout session sync to Firestore
- 3-way switch detection (power management: Off/On/Lock positions)
- Real-time data (all dummy in Phase 1)
- Home screen variant user preference (Combined only in Phase 1)
- Settings—Network screen (waiting on Lopaka design)
- Device Claim/Setup screen (waiting on Lopaka design)
- Settings—Theme screen (waiting on Lopaka design)
- Settings—Home-screen-pref screen (waiting on Lopaka design)
- Choose Split/Choose Workout screens (carousel pattern, waiting on Lopaka designs)
- Exercise Stats screen (per-exercise history, waiting on Lopaka design)

## Testing Decisions

### What Makes a Good Test
- **Test external behavior, not implementation details**: A test should verify what the user sees or what the device outputs (semantic events in a queue, screen rendered correctly, nav stack depth after a sequence of inputs), not internal state like `detent_accum` value or task loop counters.
- **Test at the highest seam possible**: Prefer testing Input task → queue output over testing raw GPIO state, prefer testing App-logic → nav stack state over testing internal selection logic.
- **Use existing test patterns**: Follow the style already established in `Hardware/Unit Test/` — standalone Arduino sketches with explicit pass/fail output over serial.

### Testing Seams

#### **Seam 1: Input Task → Semantic Event Queue**
**What to test**: Inject raw GPIO/expander states (simulate encoder rotation, button presses, encoder push-button) → verify correct semantic events appear in input queue with correct 2-detent accumulation.

**Test cases**:
- Rotate encoder CW 2 detents → expect `ENCODER_CW` event in queue
- Rotate encoder CCW 2 detents → expect `ENCODER_CCW` event in queue
- Rotate encoder CW 1 detent (partial) → expect no event yet
- Rotate encoder CW 1 more detent → expect `ENCODER_CW` event
- Press Hype button → expect `BUTTON_HYPE_PRESS` event
- Press encoder push-button → expect `ENCODER_PRESS` event
- Press encoder push-button WHILE rotating CW → expect both `ENCODER_PRESS` and `ENCODER_CW` events in hardware order
- Press all 5 buttons rapidly (burst test) → expect all 5 events in 8-entry queue without overflow

**Prior art**: `Hardware/Unit Test/` encoder decode sketch (extends to full Input task with queue output)

**Implementation**: Standalone test sketch or test mode in main firmware with serial output logging events as they're posted to the queue.

#### **Seam 2: App-logic Task → Display Refresh + Nav Stack State**
**What to test**: Inject semantic events (from a test queue or mock) → verify correct nav stack state changes, `selection_index` updates, and `DISPLAY_REFRESH_NEEDED` posts. Test Hype/Rest dual-mode behavior.

**Test cases**:
- Start at Home, send `ENCODER_CW` → expect Main Menu pushed to nav stack (depth = 1, `screen_id = MAIN_MENU`, `selection_index = 0`)
- On Main Menu (`selection_index = 0`), send `ENCODER_CW` → expect `selection_index = 1` (Music → Workout)
- On Main Menu (`selection_index = 2`), send `ENCODER_CW` → expect `selection_index = 0` (wrap Settings → Music)
- On Main Menu (`selection_index = 1`), send `ENCODER_PRESS` → expect Workout screen pushed (depth = 2)
- On Settings—Brightness (`selection_index = 50`), send `ENCODER_CW` → expect `selection_index = 51`
- On Home screen, send `BUTTON_HYPE_PRESS` → expect `dummy_music_state.trackTitle` changed, `DISPLAY_REFRESH_NEEDED` posted (no nav stack change)
- On Settings—Display, send `BUTTON_HYPE_PRESS` → expect nav stack popped (depth decreases by 1, Back action)
- Navigate Home → Main Menu → Settings → Display → Brightness, send `BUTTON_HYPE_PRESS` 4 times → expect nav stack depth = 0, `screen_id = HOME_SCREEN`
- Navigate Main Menu → Music Queue, pop back to Main Menu → expect Main Menu `selection_index` = 0 (Music, the item we entered from)
- On any screen except Home, send `BUTTON_PLAY_PAUSE_PRESS` → expect `dummy_music_state.isPlaying` toggled, `DISPLAY_REFRESH_NEEDED` posted, nav stack unchanged (global music button)

**Prior art**: None yet — new for Phase 1. Unit-level test (no rendering, just state checks).

**Implementation**: Standalone test sketch or test mode with serial output logging nav stack state after each injected event.

#### **Seam 3: `draw_screen(screen_id, state)` → Visual Output**
**What to test**: Call `draw_screen()` with known state structs → photograph on-device output → compare against Lopaka `.png` references. Test layer-order correctness, orphan asset wiring, Top Bar helper, animation frame selection.

**Test cases** (one per screen):
- `draw_screen(HOME_SCREEN, dummy_home_state)` → photograph → compare vs `Hardware/Lopaka Screens/Home Screen/home_combined.png` (Combined variant)
  - Verify album art checkerboard renders in correct position/aspect ratio
  - Verify Hype/Rest logos render in inactive state (dummy state has no active timer)
- `draw_screen(MAIN_MENU, {selection_index: 0})` → photograph → compare vs `menu_1.png` (Music selected)
  - Verify selection pill renders around Music item, not overlapping text
  - Verify Music icon is animating (frame > 0 if photographed mid-animation)
- `draw_screen(MAIN_MENU, {selection_index: 1})` → photograph → compare vs `menu_2.png` (Workout selected)
- `draw_screen(MAIN_MENU, {selection_index: 2})` → photograph → compare vs `menu_3.png` (Settings selected)
- `draw_screen(SETTINGS_MAIN, {selection_index: 0})` → photograph → compare vs `settings_main.png`
  - Verify Top Bar renders (WiFi disconnected, battery at dummy level)
  - Verify first Settings item is visually selected (selected state orphan asset)
- `draw_screen(SETTINGS_DISPLAY, {selection_index: 0})` → photograph → compare vs `settings_display.png`
- `draw_screen(SETTINGS_BRIGHTNESS, {selection_index: 75})` → photograph → verify brightness slider at 75% position
  - Verify Top Bar renders
- `draw_screen(SETTINGS_HYPE_REST, {selection_index: 0})` → photograph → compare vs `settings_hype_rest.png`
- `draw_screen(MUSIC_QUEUE, dummy_music_queue_state)` → photograph → compare vs `music_queue.png`
  - Verify layer-order fix from bring-up test is still correct (no silent erasure)
  - Verify Top Bar renders
- `draw_screen(GUIDED_LOGGING_OVERVIEW, dummy_workout_state)` → photograph → compare vs `log_workout_1.png`
- `draw_screen(GUIDED_LOGGING_EXERCISE, {selection_index: 0, repCount: 10, weight: 135})` → photograph → verify rep count displays "10", weight displays "135 lb"

**Prior art**: `Hardware/Unit Test/preview_music_queue/` (extends to all 9 screens)

**Implementation**: Standalone test sketch per screen, serial prompt "Press enter when photographed", move to next screen. Photographs stored externally (not automated comparison in Phase 1).

#### **Seam 4: Screen-Specific Interaction Logic**
**What to test**: Simulate user interaction sequences (scrolling through encoder-selectable elements, changing values, multi-step navigation) → verify UI responds correctly at each step.

**Test cases**:
- **Main Menu scrolling**: Start at Music (`selection_index = 0`), rotate encoder CW 2 detents → verify Workout selected (`selection_index = 1`), rotate CW 2 more detents → verify Settings selected (`selection_index = 2`), rotate CW 2 more detents → verify Music selected again (`selection_index = 0`, wrapping)
- **Settings—Brightness value change**: Start at 50, rotate encoder CW 2 detents → verify value = 51 and slider position updates, rotate CW 20 detents (10 selection moves) → verify value = 61, rotate CCW 4 detents → verify value = 59
- **Guided Logging rep entry**: Start at 8 reps, rotate encoder CW 2 detents → verify rep count = 9, rotate CW 4 detents → verify rep count = 11, rotate CCW 6 detents → verify rep count = 8 (back to start)
- **Guided Logging weight entry**: Start at 135 lb, rotate encoder CW 2 detents → verify weight = 137.5 lb (±2.5 lb per selection move), rotate CCW 2 detents → verify weight = 135 lb
- **Settings menu navigation depth**: Navigate Home → Main Menu → Settings → Display → Brightness, verify at each step that `nav_stack_depth` increments and `screen_id` is correct, then press Hype 4 times and verify depth decrements back to 0 at Home
- **Music Queue scrolling (if scrolling implemented in Phase 1)**: If Music Queue has scrollable track list, rotate encoder to move selection through visible items, verify `scroll_offset` updates when selection reaches top/bottom of viewport
- **Animated icon state transition**: On Main Menu with Music selected (animating), rotate encoder to Workout → verify Music icon freezes on frame 0 immediately (animation cancelled), verify Workout icon starts animating from frame 0
- **Global music button interaction**: On Settings—Brightness screen, press Play/Pause → verify `dummy_music_state.isPlaying` toggles BUT Settings—Brightness screen remains visible (no nav stack change), verify display refreshes to show updated music state if Home screen music block were visible (no visible change on Settings screen itself, but state mutated correctly)

**Prior art**: None yet — new testing pattern for Phase 1.

**Implementation**: Interactive test mode in firmware with serial logging, or automated test sequences with assertion checks at each step. Photograph key states for visual confirmation.

### Testing Tools & Environment
- **Hardware**: XIAO ESP32S3 with wired encoder, 5 buttons, PCF8574T expander, ST7789 display
- **Serial monitor**: 115200 baud, log test results and nav stack state
- **Camera/phone**: For photographing on-device renders to compare vs Lopaka `.png` references
- **Test execution**: Manual (run test sketch, interact via physical inputs, observe serial output + photograph screen)

### Testing Coverage Target
- **Seam 1 (Input)**: 100% of semantic event types (`ENCODER_CW`, `ENCODER_CCW`, `ENCODER_PRESS`, 5 button events), 2-detent accumulation edge cases (partial detent, rollover), burst test (8-entry queue saturation)
- **Seam 2 (App-logic)**: All nav stack transitions (Home → Main Menu → each submenu, Back action at each depth), Hype/Rest dual-mode on Home vs non-Home, Main Menu selection memory, global music buttons
- **Seam 3 (Rendering)**: All 9 screens with at least one state variant each, layer-order correctness, Top Bar on all screens except Home, orphan assets (Hype/Rest logos, selection pills, UI element states)
- **Seam 4 (Interaction)**: Encoder scrolling/wrapping on all menu screens, value adjustment on Settings—Brightness and Guided Logging, navigation depth limits (8-entry stack), animation state transitions

## Out of Scope

- **Automated visual testing**: Phase 1 uses manual photograph comparison; automated pixel-diff testing is future work
- **Performance profiling**: Stack high-water mark measurement (`uxTaskGetStackHighWaterMark()`) and task timing analysis are optional Phase 1 tasks, not required for exit criterion
- **Edge case hardening**: Input debounce tuning, encoder mechanical bounce handling, queue overflow handling beyond the 8-entry cap are Phase 2 refinements unless they cause observable failures in Phase 1
- **WiFi/Network functionality**: All Network task behavior (WiFi init, HTTP client, Spotify calls, device claim polling, config sync) is Phase 2 work
- **Persistent storage writes**: NVS write path for Settings changes is Phase 2; Phase 1 Settings are RAM-only
- **Power management**: 3-way switch detection (Off/On/Lock) is deferred to post-Phase-1 work
- **Unmocked screens**: Settings—Network, Device Claim/Setup, Settings—Theme, Settings—Home-screen-pref, Choose Split, Choose Workout, Exercise Stats (all waiting on user Lopaka designs)
- **Scrolling lists beyond Music Queue**: If other screens need scrolling (e.g. long exercise list in Guided Logging), implement only if time allows; fixed-size lists are acceptable for Phase 1
- **Error handling UI**: No error screens or error state rendering in Phase 1 (e.g. "WiFi failed", "NVS corrupted"); those are Phase 2 concerns

## Further Notes

### Phase 1 Exit Criterion
Phase 1 is **complete** when:
1. Every screen in the locked screen tree (9 screens: Home, Main Menu, Music Queue, Settings—Main, Settings—Display, Settings—Brightness, Settings—Hype&Rest, Guided Logging overview, Guided Logging exercise entry) is reachable via physical inputs (encoder + 5 buttons)
2. All screens render correctly (visually matching Lopaka `.png` references, no layer-order erasure)
3. Navigation works per `02-hardware-ui.md` spec (Hype=Back outside Home, encoder rotate/press, nav stack push/pop, Main Menu selection memory)
4. All dummy data renders correctly (track title, artist, workout name, set/rep counts, brightness value)
5. Hype/Rest dual-mode behavior works (Home screen → dummy Spotify trigger with visual feedback, other screens → Back/Info)
6. Global music buttons work everywhere (Previous/Play-Pause/Next mutate dummy state without exiting current screen)
7. Animated selection icons work (200ms/frame, cancel on transition)
8. All 4 testing seams pass (Input queue events, App-logic nav state, visual rendering, interaction sequences)

### Iteration Strategy
- **Start with simplest screens first** (Top Bar → Main Menu → Settings tree) to prove the framework before tackling complex screens (Home with album art, Guided Logging with multi-state)
- **Wire one screen at a time**: implement `draw_screen()` case, review Lopaka code for layer-order, upload to device, photograph, compare vs `.png`, fix any issues before moving to next screen
- **Identify orphan assets per screen**: as each screen is wired, ask user "which PNGs in this folder are orphan assets and where do they render?" rather than guessing from inconsistent naming
- **Test interaction logic incrementally**: after each screen is wired, run Seam 4 interaction tests for that screen before moving to the next

### Design Consistency Notes
- **Selection pills vary per screen context**: Main Menu pills look different from Settings pills (confirmed user design decision); don't try to unify them
- **Encoder-selectable UI elements**: Every interactive element (menu items, settings, buttons, sliders) has selected/unselected visual states; ensure orphan assets wire correctly for each
- **Top Bar everywhere except Home**: Home screen has its own header design (different from the shared Top Bar); don't force Top Bar onto Home

### ADR Reference
- **Hype/Rest dual-mode button behavior** (hard-to-reverse, surprising without context, real trade-off): If an ADR is created for this decision, reference it here. Decision: Hype/Rest buttons are context-sensitive (Spotify on Home, Back/Info elsewhere) to save physical buttons. Consequence: Single decision point in App-logic input handler, less discoverable than dedicated Back button but hardware-constrained. Alternatives: dedicated Back button (requires 6th button), long-press encoder for Back (conflicts with press-to-select).

### Cross-Reference to Locked Context Docs
- `Context Documents/01-hardware-firmware.md` — Task architecture, display driver config, encoder/expander wiring
- `Context Documents/02-hardware-ui.md` — Screen tree, nav stack model, input mappings, per-screen behavior
- `Context Documents/06-implementation-plan.md` — Phase 1 checklist (update as tasks complete)
- `CONTEXT.md` — Domain glossary (Device, Screen, Nav Stack, NavigationState, Semantic Input Event, Hype/Rest, Lopaka Screen, Orphan Assets, etc.)
