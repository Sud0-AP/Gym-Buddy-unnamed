# 07: Home Screen + Dummy State + Global Music Buttons

**What to build:** `dummy_state.h` header with realistic camelCase test data structs. Home Screen (Combined variant) wired with dummy music state (trackTitle, artistName, isPlaying, progressMs, durationMs) and dummy workout state (workoutName, currentExercise, setCount, repCount). Album art placeholder rendered as checkerboard pattern. Hype/Rest logos in active/inactive states (orphan assets wired with conditional logic). Global music buttons (Previous/Play-Pause/Next) mutate dummy music state on all screens without exiting current screen. Home screen is default on boot.

**Blocked by:** 06: Settings—Hype&Rest + Storage Task NVS Init

**Status:** ready-for-agent

- [ ] `dummy_state.h` header created with `DummyMusicState`, `DummyWorkoutState`, `DummySettingsState` structs
- [ ] Field naming: camelCase matching Firestore schema (`trackTitle`, `artistName`, `isPlaying`, `progressMs`, `durationMs`, `workoutName`, `currentExercise`, `setCount`, `repCount`)
- [ ] Home Screen wired from `Hardware/Lopaka Screens/Home Screen/` (Combined variant only)
- [ ] Home Screen renders dummy music state (track title, artist, play/pause icon, progress bar)
- [ ] Home Screen renders dummy workout state (workout name, current exercise, set/rep counts)
- [ ] Album art placeholder rendered as checkerboard pattern via `drawRect()` loop (alternating filled/unfilled rectangles)
- [ ] Hype/Rest logos (orphan assets) identified and wired with conditional logic: active state when timer running, inactive otherwise
- [ ] Home screen is default on boot (`nav_stack_depth = 0`, `screen_id = HOME_SCREEN`)
- [ ] Hype button press on Home mutates `dummy_music_state.trackTitle` to "Hype Playlist Track", sets Hype logo to active, posts `DISPLAY_REFRESH_NEEDED`
- [ ] Rest button press on Home mutates `dummy_music_state.trackTitle` to "Rest Playlist Track", sets Rest logo to active, posts `DISPLAY_REFRESH_NEEDED`
- [ ] Global music buttons (Previous/Play-Pause/Next) mutate `dummy_music_state` on ALL screens (not just Home/Music Queue)
- [ ] Play/Pause toggles `dummy_music_state.isPlaying`
- [ ] Previous/Next change `dummy_music_state.trackTitle` to simulate track change
- [ ] Global music button presses post `DISPLAY_REFRESH_NEEDED` WITHOUT exiting current screen (nav stack unchanged)
- [ ] Seam 3 testing passes: photograph Home Screen, compare vs Lopaka reference (Combined variant)
- [ ] Seam 4 testing passes: press Hype on Home → verify track title changes + logo changes to active; press Play/Pause on Settings—Brightness → verify `isPlaying` toggles but Settings screen stays visible
