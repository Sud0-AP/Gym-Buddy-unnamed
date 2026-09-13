# 09: Animated Selection Icons

**What to build:** Animated selection icons extracted from Animations folder as C arrays. Display task manages 200ms animation timer, selected Main Menu item loops through sprite frames at 5fps, unselected items show static frame 0. Animation cancels immediately on screen transition. Hardware tester navigates Home → Main Menu, sees Music item icon animating at ~5fps, rotates encoder to Workout and sees Music icon freeze on frame 0 immediately, Workout icon starts animating.

**Blocked by:** 08: Music Queue Rendering

**Status:** ready-for-agent

- [ ] Animated selection icon sprite frames extracted from `Hardware/Lopaka Screens/Animations/` as C arrays
- [ ] Each Main Menu item (Music, Workout, Settings) has its own sprite frame array
- [ ] Display task manages 200ms animation timer (5 fps)
- [ ] Display task tracks `current_frame_index` per Main Menu item
- [ ] Selected Main Menu item (`selection_index` matches item) loops through sprite frames: frame 0 → 1 → 2 → ... → N → 0 (repeat)
- [ ] Unselected Main Menu items render static frame 0
- [ ] Frame index increments every 200ms (timer callback in Display task)
- [ ] Animation cancels immediately on screen transition: Display task sets `current_frame_index = 0` for all items when screen_id changes
- [ ] `draw_screen(MAIN_MENU, state)` uses `current_frame_index` to select which sprite frame to `pushImage()` for each item
- [ ] Seam 4 testing passes: navigate Home → Main Menu → see Music icon animating smoothly (~5 frames/second), rotate encoder to Workout → verify Music icon freezes on frame 0 instantly, verify Workout icon starts animating from frame 0
- [ ] No animation stutter or lag during encoder rotation (frame timer continues independently of input events)
