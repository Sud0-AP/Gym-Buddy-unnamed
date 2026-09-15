# Ticket #6: Settings—Hype&Rest + Storage NVS - Implementation Complete

## Summary

Implemented the complete Settings—Hype&Rest screen with 8 interactive elements (browse/edit modes) and created the Storage task for NVS initialization with Phase 1 dummy defaults. All navigation, rendering, and interaction logic has been wired into the firmware.

## What Was Delivered

### 1. Settings—Hype&Rest Screen
**Location:** `Hardware/Firmware/src/display/screens/settings_hype_rest.cpp`

- ✅ **8 interactive elements** in top-to-bottom tab order:
  - Hype: checkbox, minutes, seconds, playlist
  - Rest: checkbox, minutes, seconds, playlist
- ✅ **All Lopaka visual assets wired**: tickboxes (4 states), time arrows (2 states), playlist pills (2 states)
- ✅ **Two interaction modes**: browse (encoder moves selection) and edit (encoder adjusts values)
- ✅ **Value constraints**: 0-10 minutes (±1), 0-59 seconds (±5), playlist carousel (5 dummy names)

### 2. Storage Task
**Location:** `Hardware/Firmware/src/storage/`

- ✅ **NVS initialization** via Preferences API at boot
- ✅ **Queue-based event system** (NVS_READ / NVS_WRITE)
- ✅ **Dummy defaults** for Phase 1:
  - Hype: 1m 30s, "Varjish", exercise-based OFF
  - Rest: 2m 30s, "Super chill", exercise-based ON
  - Brightness: 50%
- ✅ **Priority 0, 4KB stack** as specified
- ✅ **Serial logging** for all operations
- ✅ **RAM-only changes** (no persistence in Phase 1)

### 3. App Logic Integration
**Location:** `Hardware/Firmware/src/app/app_task.cpp`

- ✅ **Global state struct** `g_hype_rest_state` with all field values + UI state
- ✅ **Navigation wiring**: Settings—Main → Hype & Rest (index 1) → Settings—Hype&Rest
- ✅ **Complete input handling**:
  - **Browse mode**: CW/CCW moves selection, press enters edit or toggles checkbox
  - **Edit mode**: CW/CCW adjusts value, press saves & exits

### 4. Display Integration
**Location:** `Hardware/Firmware/src/display/display_task.cpp`

- ✅ Added SCREEN_SETTINGS_HYPE_REST rendering case
- ✅ Full screen redraw on state changes (Phase 1 approach)

### 5. Main Task Spawn
**Location:** `Hardware/Firmware/src/main.cpp`

- ✅ Storage task initialization + spawn with correct priority

## Files Created

1. `Hardware/Firmware/src/display/screens/settings_hype_rest.cpp` - Screen implementation (~300 lines)
2. `Hardware/Firmware/src/storage/storage_task.h` - Storage interface
3. `Hardware/Firmware/src/storage/storage_task.cpp` - Storage implementation
4. `Hardware/Firmware/src/app/app_state.h` - Global state declarations

## Files Modified

1. `Hardware/Firmware/src/main.cpp` - Added storage init/spawn
2. `Hardware/Firmware/src/app/app_task.cpp` - Added HypeRestState + interaction logic (~100 lines)
3. `Hardware/Firmware/src/display/screens.h` - Added Hype&Rest declarations
4. `Hardware/Firmware/src/display/display_task.cpp` - Added rendering case

## How to Test on Hardware

### Build & Upload
```bash
cd Hardware/Firmware
pio run --target upload  # Build and flash to device
pio device monitor       # Watch serial logs
```

### Expected Serial Output
```
[STORAGE] NVS initialized successfully (Preferences API)
[STORAGE] Storage task initialized with 8-entry queue
[APP] Nav transition: screen_id=SETTINGS_HYPE_REST
[DISPLAY] Render Settings—Hype&Rest, selected_item=0, edit_mode=0
[APP] Hype&Rest selection: 1
[APP] Entered edit mode for item 1
[APP] Hype&Rest edit: adjusted value
[APP] Exited edit mode, value saved
```

### Navigation Path to Screen
1. Power on device (boots to Home)
2. Rotate encoder → enters Main Menu
3. Rotate to "Settings" (index 2)
4. Press encoder → enters Settings—Main
5. Rotate to "Hype & Rest" (index 1)
6. Press encoder → **Settings—Hype&Rest screen appears**

### Testing Checklist

#### Visual Verification (Seam 3)
Compare photograph against `Hardware/Lopaka Screens/Settings/settings_hype_rest.png`:
- [ ] Top bar renders correctly
- [ ] Both section headers ("Hype:", "Rest:") visible
- [ ] All 8 items render with correct layout
- [ ] Icons (clock, music) appear
- [ ] Selection arrows change between selected/unselected states
- [ ] Playlist pills swap between filled/outline based on selection
- [ ] Checkbox borders change color when selected

#### Interaction Testing (Seam 4)

**Browse Mode:**
- [ ] Encoder CW moves selection 0→1→2→...→7→0 (wraps)
- [ ] Encoder CCW moves selection 7→6→5→...→0→7 (wraps)
- [ ] Serial logs show: `[APP] Hype&Rest selection: N`
- [ ] Visual indicators update to show new selected item

**Checkboxes (items 0, 4):**
- [ ] Press on checkbox toggles it immediately (no edit mode)
- [ ] Visual: unchecked ↔ checked state
- [ ] Serial: `[APP] Toggled checkbox N`

**Time Fields (items 1, 2, 5, 6):**
- [ ] Press enters edit mode
- [ ] Serial: `[APP] Entered edit mode for item N`
- [ ] Label color changes (Minute(s)/Second(s) becomes accent color)
- [ ] CW increments value (minutes +1, seconds +5)
- [ ] CCW decrements value (minutes -1, seconds -5)
- [ ] Values clamp at limits (0-10 min, 0-59 sec)
- [ ] Press again saves & exits edit mode
- [ ] Serial: `[APP] Exited edit mode, value saved`

**Playlists (items 3, 7):**
- [ ] Press enters edit mode
- [ ] CW cycles through: Varjish → Super chill → Pump it up → Focus Mode → Beast Mode → (wrap)
- [ ] CCW cycles backward
- [ ] Text in pill updates on each change
- [ ] Press saves & exits

**Back Navigation:**
- [ ] Press Hype button (left) → returns to Settings—Main
- [ ] Settings state preserved (can return and see same values)

## Known Phase 1 Limitations

1. **No persistence**: All changes lost on reboot (RAM-only by design)
2. **Full redraws**: Screen flickers slightly on updates (no dirty-rect optimization yet)
3. **No playlist animation**: Text changes but no "nudge" effect implemented
4. **Dummy data**: 5 hardcoded playlists, not synced from backend

## Next Steps

1. **Test on hardware** - Complete checklist above
2. **Photograph screen** - Upload for visual verification vs Lopaka reference
3. **If issues found**:
   - Check serial logs for state transitions
   - Verify layer draw order if visual elements missing
   - Confirm `g_hype_rest_state.selected_item` reads correct
4. **Once verified** - Close GitHub Issue #6 via:
   ```bash
   gh issue close 6 --comment "Tested on hardware, all acceptance criteria passing"
   ```

## Architecture Notes

- **HypeRestState struct** duplicated in app_task.cpp and settings_hype_rest.cpp to avoid header dependencies - keep in sync
- **Edit mode** tracked in `g_hype_rest_state.edit_mode`, not nav stack
- **Encoder convention**: CW=increment/next, CCW=decrement/previous (consistent with brightness)
- **Dummy defaults** match screenshot state from ticket

## Questions or Issues?

If the build fails or behavior doesn't match expectations:
1. Check serial output for error messages
2. Verify PlatformIO environment is set up correctly
3. Ensure `Preferences` library is available (Arduino-ESP32 core)
4. Review this summary's serial log expectations against actual output
