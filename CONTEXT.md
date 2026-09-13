# Gym Buddy — Domain Model

## Core Concepts

### Device
The ESP32-based hardware unit (XIAO ESP32S3: 8MB PSRAM, 512KB SRAM, 8MB Flash). Acts as the source of truth for an active workout session. Connects to WiFi for Spotify control and history sync, but **operates offline-first** — a full workout can be logged with zero network connection.

### Screen
A distinct UI state rendered on the 2.4" ST7789 display (240×320 landscape). Each screen has a unique `screen_id` and is drawn via `draw_screen(screen_id, state)`. Screens are navigable via encoder + buttons per the nav-stack model.

### Nav Stack
An 8-entry fixed array of `NavigationState` structs tracking screen history. Pushing a screen adds to the stack; popping (Back action) returns to the previous screen. Depth never exceeds 8 (documented limit in `02-hardware-ui.md`).

### NavigationState
```cpp
struct NavigationState {
  uint8_t screen_id;        // which screen
  uint8_t selection_index;  // generic "what's selected" (menu item, slider position, etc.)
  uint16_t scroll_offset;   // for future scrolling screens
};
```
Screen-agnostic: every screen interprets `selection_index` for its own selection mechanism (Main Menu items, Settings items, brightness value, etc.).

### Selection Index
The zero-based index of the currently selected item/value on a screen. Incremented/decremented by encoder rotation. Semantics vary per screen:
- **Main Menu**: which menu item (0 = Music, 1 = Workout, 2 = Settings)
- **Settings—Brightness**: brightness value (0–100)
- **Guided Logging exercise list**: which exercise in the current workout

### Encoder-Selectable UI Element
An on-screen interactive component (button, slider, list item) that responds to encoder rotation (move selection) and encoder press (confirm selection). Renders in **selected/unselected states** using orphan assets (different graphics per state). Examples: "Start Workout" button, "Adjust Rest Timer" control, Settings menu items. Selection pills (background highlight graphics) also vary per screen context — Main Menu pills look different from Settings pills.

### Detent Accumulator
Encoder rotation counter implementing 2-detent-per-selection sensitivity. Simple rollover logic: `detent_accum++` on CW, `detent_accum--` on CCW; post semantic event (`ENCODER_CW`/`ENCODER_CCW`) when `abs(detent_accum) >= 2`, then reset to 0.

### Semantic Input Event
High-level input abstraction decoded by the Input task from raw GPIO/expander state. Examples: `ENCODER_CW`, `ENCODER_CCW`, `ENCODER_PRESS`, `BUTTON_HYPE_PRESS`, `BUTTON_REST_PRESS`, `BUTTON_PREV_PRESS`, `BUTTON_PLAY_PAUSE_PRESS`, `BUTTON_NEXT_PRESS`. Posted to an 8-entry FreeRTOS queue consumed by App-logic task.

### Hype / Rest
Two physical buttons on the device (wired to PCF8574T expander P3 and P1). **On Home screen only**: trigger Spotify playlist swap + timer behavior (Hype = energetic playlist + configurable duration, Rest = calm playlist + rest timer). **On all other screens**: Hype acts as **Back** (pop nav stack), Rest acts as **Info** (context help, future feature).

**Home screen visual feedback**: Hype/Rest logos render in active/inactive states (orphan assets, not in Lopaka exports) — active state shows when that mode's timer is running, inactive otherwise. Likely shown on Combined variant only.

**Button press flow (Home screen)**:
1. Input task posts `BUTTON_HYPE_PRESS` or `BUTTON_REST_PRESS` to input queue
2. App-logic task checks `current_screen_id == HOME_SCREEN`
3. App-logic posts `SPOTIFY_HYPE` or `SPOTIFY_REST` command to Network task queue
4. Network task (Phase 2) executes backend-proxied Spotify call, posts `DISPLAY_REFRESH_NEEDED` on success
5. Display task redraws Home screen with updated music state + active timer

**Phase 1 behavior**: App-logic mutates dummy state directly (swap `trackTitle`, start fake timer), posts `DISPLAY_REFRESH_NEEDED` immediately (no real Spotify call).

### Lopaka Screen
A `.txt` + `.png` pair exported from the Lopaka TFT_eSPI GUI designer. The `.txt` contains C++ `draw()` code with `tft.pushImage()` / `tft.drawString()` calls; the `.png` is the visual reference. **User-authored ground truth** for what a screen renders — never edited by the coding agent. Wired into firmware by extracting the drawing code into a `draw_screen()` case.

### Orphan Assets
`.png` files in `Hardware/Lopaka Screens/` folders that exist but **are not referenced in the exported `.txt` code**. These are state-dependent graphics (Hype/Rest active/inactive indicators, selection pills per-screen-context, encoder-selectable UI elements in selected/unselected states) that must be manually wired during screen implementation. Naming pattern exists but is inconsistent — resolve per-screen during implementation. Converted to C arrays via automated tool (e.g. `image2cpp`), then `pushImage()` calls added to `draw_screen()` with conditional logic based on state.

### Layer-Order Gotcha
Any full-region background graphic (top bar, selection pill, card background) **must draw BEFORE foreground content** (text, icons), or the `pushImage()` call silently erases the foreground. Fix: reorder background calls to immediately after `tft.fillScreen()`. Prevention: in Lopaka, put every full-region background layer at the **bottom** of the canvas layer stack before exporting.

### Top Bar
Shared header component drawn by most screens (Home screen has its own different header). Renders via `draw_top_bar(wifi_connected, battery_level)` helper function. Shows:
- **WiFi icon**: 2-state (connected / disconnected)
- **Battery icon**: 5-step (0% / 25% / 50% / 75% / 100% fill), no color change

### Animated Selection Icon
Sprite-frame loop for selected Main Menu items. Plays at **200ms per frame (5 fps)** when an item is selected; shows static frame 0 when not selected. Frames exported from `Hardware/Lopaka Screens/Animations/`.

### Dummy State
Phase 1 test data defined in `dummy_state.h`. Field names match Firestore schema exactly (camelCase per `03-web-app.md`), so Phase 2 can swap `#include "dummy_state.h"` for `#include "real_state.h"` without touching `draw_screen()` implementations. Examples:
```cpp
struct DummyMusicState {
  const char* trackTitle = "Shape of You";
  const char* artistName = "Ed Sheeran";
  bool isPlaying = true;
  uint32_t progressMs = 125000;
  uint32_t durationMs = 233000;
};
```

### Album Art Placeholder
**Phase 1**: checkerboard pattern via `drawRect()` loop — proves art-box positioning/scaling without requiring TJpg_Decoder.  
**Phase 2**: actual album art fetched on track change (~300px CDN variant), decoded via TJpg_Decoder, RAM-only cache (not persisted to flash).

## FreeRTOS Task Architecture

Five tasks, priorities high-to-low:

### Display Task (Priority 3)
- **Stack**: 4KB
- **Responsibility**: Render screens via `draw_screen(screen_id, state)`, manage animation frame timing (200ms/frame for selected icons), handle display refresh
- **Refresh strategy**: Event-driven — other tasks post `DISPLAY_REFRESH_NEEDED` to display queue when state changes; display task does NOT poll
- **Screen transition timing**: Cancel animation immediately on input (responsiveness beats smoothness)

### Input Task (Priority 2)
- **Stack**: 2KB
- **Responsibility**: Decode encoder (GPIO D6/D7, single CHANGE-interrupt method) + 5 buttons (via PCF8574T at 0x20: Previous=P2, Play/Pause=P4, Next=P0, Hype=P3, Rest=P1) + encoder push-button into semantic events
- **Queue**: 8-entry semantic input event queue for App-logic task
- **Encoder sensitivity**: 2 detents = 1 selection move (simple counter accumulator, Option A from Q13)
- **Simultaneous press+rotate**: Post both events in hardware order; App-logic decides validity (very rare edge case)

### App-logic Task (Priority 1)
- **Stack**: 4KB
- **Responsibility**: Consume semantic input events, manage nav stack (push/pop screens), update selection state, drive screen transitions, coordinate Display/Storage/Network tasks
- **Nav stack depth**: 8 entries max
- **Main Menu selection memory**: Persistent `selection_index` field in `NavigationState` survives nav pops

### Network Task (Priority 1)
- **Stack**: 8KB
- **Responsibility**: WiFi connection, HTTP client, backend-proxied Spotify calls, device claim polling, config sync
- **Phase 1 behavior**: Scaffolded but idle — task exists with empty loop `while(1) { vTaskDelay(1000); }`, no WiFi init yet

### Storage Task (Priority 0)
- **Stack**: 4KB
- **Responsibility**: NVS (non-volatile storage) reads/writes for settings persistence, workout session buffer
- **Phase 1 behavior**: Init NVS, handle `NVS_READ` queue requests (return dummy defaults for Settings screens), no `NVS_WRITE` path wired yet (Settings changes are RAM-only until Phase 2)

## Phase 1 Decisions

### Project Structure
PlatformIO (`Hardware/Firmware/platformio.ini` + `src/main.cpp`), not monolithic `.ino`. Arduino-ESP32 core, FreeRTOS via Arduino APIs.

### Library Dependencies (Phase 1)
- **TFT_eSPI**: display driver (4 non-default settings required: `TFT_RGB_ORDER TFT_BGR`, `TFT_INVERSION_OFF`, `CGRAM_OFFSET`, `USE_HSPI_PORT`)
- **Wire**: I2C for PCF8574T IO expander
- **Preferences**: Arduino NVS wrapper (for Storage task init)
- **Deferred to Phase 2**: ArduinoJson, WiFiClientSecure

### Screen Wiring Order (Simplest-to-Hardest)
1. Top Bar (static header, simplest)
2. Main Menu (3 selection states, static layout)
3. Settings—Main (list, static items)
4. Settings—Display, Settings—Brightness, Settings—Hype&Rest (submenu patterns)
5. Home Screen (3 variants, dynamic text + album art placeholder)
6. Music Queue (scrolling list placeholder)
7. Guided Logging (multi-state, most complex)

### Home Screen Variant Selection
**Phase 1**: Always render **Combined** variant (music block + workout block).  
**Phase 2**: User preference setting (Settings → Display → Home screen preference) stored in NVS.

### Unit Test Reuse
Don't rewrite from scratch — the Unit Test sketches (`Hardware/Unit Test/display_bringup_test/`, `Hardware/Unit Test/preview_music_queue/`) already prove display init and encoder decode work. Extract and refactor later if needed, but start by copying proven code.

### Layer-Order Verification Workflow
**Before uploading each screen**: Read the Lopaka `.txt` export, identify full-region backgrounds (top bar, selection pills, card fills), verify those `pushImage()`/`fillRect()` calls appear immediately after `tft.fillScreen()`.  
**After uploading**: Visual check — photograph on-device render, compare against `.png` reference.

### 3-Way Switch (Power Management)
**Deferred** — not part of Phase 1. Will be addressed post-Phase-1 as part of power management work (Off / On / Lock positions).

## Relationships

- A **Screen** is drawn by `draw_screen(screen_id, state)` and lives on the **Nav Stack** as a `NavigationState` entry
- The **Input Task** decodes hardware into **Semantic Input Events** consumed by **App-logic Task**
- The **App-logic Task** mutates `NavigationState` and posts `DISPLAY_REFRESH_NEEDED` to **Display Task**
- **Hype/Rest** buttons have dual semantics: Spotify control on Home screen, Back/Info on all other screens
- **Lopaka Screens** are user-authored; firmware wires their drawing code behind `draw_screen()`
- **Dummy State** field names match **Firestore schema** exactly (Phase 2 compatibility)
- The **Layer-Order Gotcha** is a display driver quirk requiring backgrounds to draw before foreground
- **Album Art Placeholder** (Phase 1 checkerboard) proves layout before TJpg_Decoder integration (Phase 2)

## Non-Goals (Explicitly Out of Scope for Phase 1)

- WiFi connection / provisioning / reconnect logic
- Spotify API calls (backend-proxied or otherwise)
- Device claim handoff
- NVS write persistence for Settings changes
- Actual album art fetch/decode (TJpg_Decoder)
- Workout session sync to Firestore
- 3-way switch detection (power management)
- Real-time data (all dummy data in Phase 1)

## Edge Cases & Constraints

- **Nav stack overflow**: If depth would exceed 8, App-logic task refuses to push (user must pop first or return to Home to reset)
- **Encoder press+rotate simultaneity**: Mechanically rare but possible; Input task posts both events, App-logic determines if one is stale based on screen-transition timing
- **Screen transition mid-animation**: Display task cancels animation immediately (Option B from Q14) — input responsiveness beats visual smoothness
- **Album art box during Phase 1**: Checkerboard pattern shows boundary clearly for layout verification, doesn't require HTTP/TJpg integration
- **Main Menu selection memory**: Only Main Menu's `selection_index` persists across nav pops (per locked decision in `02-hardware-ui.md`); other screens reset to default `selection_index` on re-entry

## Phase 1 Exit Criterion

**Every screen in the tree** (except the 4 unmocked screens: Settings—Network, Device Claim/Setup, Settings—Theme, Settings—Home-screen-pref) is:
- Reachable via physical inputs (encoder + 5 buttons)
- Fully navigable per `02-hardware-ui.md` spec
- Rendering with **dummy data** (no real WiFi/Spotify/Firestore)
- Visually matching its Lopaka `.png` reference (layer order correct, no silent erasure)
