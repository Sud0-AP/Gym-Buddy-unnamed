# 12: Full Navigation + Exit Criterion Verification

**What to build:** All 9 screens reachable and navigable via physical inputs. All 4 testing seams pass completely. Hardware tester executes full test suite: navigates Home → Main Menu → Settings → Display → Brightness → Hype 4x back to Home (nav stack depth verified), navigates Main Menu → Music → Hype back to Main Menu (selection memory verified), tests all encoder/button combinations per Seam 1/2/4 test cases, photographs all screens per Seam 3 (all match Lopaka references). Serial logs confirm nav stack behavior, input events, animation timing, global music buttons all work correctly. Phase 1 exit criterion met.

**Blocked by:** 11: Guided Logging Set Entry + Full Workout Flow

**Status:** ready-for-agent

## Full Navigation Coverage
- [ ] All 9 screens reachable: Home, Main Menu, Music Queue, Settings—Main, Settings—Display, Settings—Brightness, Settings—Hype&Rest, Guided Logging overview, Guided Logging set entry
- [ ] Every screen accessible via physical inputs only (no hardcoded screen transitions in test code)
- [ ] Nav stack depth never exceeds 8 entries
- [ ] Hype button acts as Back on all screens except Home (pops nav stack)
- [ ] Hype button on Home triggers dummy Spotify Hype behavior (track title change, active logo)
- [ ] Encoder rotation on Home pushes Main Menu onto nav stack
- [ ] Main Menu selection memory works: navigate Music → Hype back to Main Menu → Music still selected

## Seam 1: Input Task → Semantic Event Queue (100% coverage)
- [ ] Rotate encoder CW 2 detents → `ENCODER_CW` event in queue
- [ ] Rotate encoder CCW 2 detents → `ENCODER_CCW` event in queue
- [ ] Rotate encoder CW 1 detent (partial) → no event yet
- [ ] Rotate encoder CW 1 more detent → `ENCODER_CW` event posted
- [ ] Press Hype button → `BUTTON_HYPE_PRESS` event
- [ ] Press encoder push-button → `ENCODER_PRESS` event
- [ ] Press encoder push-button WHILE rotating CW → both `ENCODER_PRESS` and `ENCODER_CW` events in hardware order
- [ ] Press all 5 buttons rapidly (burst test) → all 5 events in 8-entry queue without overflow
- [ ] Serial logs confirm all event types posted correctly

## Seam 2: App-logic Task → Display Refresh + Nav Stack State (all transitions)
- [ ] Start at Home, send `ENCODER_CW` → Main Menu pushed (depth=1, screen_id=MAIN_MENU, selection_index=0)
- [ ] On Main Menu (selection_index=0), send `ENCODER_CW` → selection_index=1 (Music → Workout)
- [ ] On Main Menu (selection_index=2), send `ENCODER_CW` → selection_index=0 (wrap Settings → Music)
- [ ] On Main Menu (selection_index=1), send `ENCODER_PRESS` → Workout screen pushed (depth=2)
- [ ] On Settings—Brightness (selection_index=50), send `ENCODER_CW` → selection_index=51
- [ ] On Home screen, send `BUTTON_HYPE_PRESS` → dummy_music_state.trackTitle changed, `DISPLAY_REFRESH_NEEDED` posted, nav stack unchanged
- [ ] On Settings—Display, send `BUTTON_HYPE_PRESS` → nav stack popped (depth decreases by 1, Back action)
- [ ] Navigate Home → Main Menu → Settings → Display → Brightness, send `BUTTON_HYPE_PRESS` 4 times → nav stack depth=0, screen_id=HOME_SCREEN
- [ ] Navigate Main Menu → Music Queue, pop back to Main Menu → Main Menu selection_index=0 (Music item)
- [ ] On Settings—Display (not Home), send `BUTTON_PLAY_PAUSE_PRESS` → dummy_music_state.isPlaying toggled, `DISPLAY_REFRESH_NEEDED` posted, nav stack unchanged

## Seam 3: draw_screen() → Visual Output (all 9 screens)
- [ ] Photograph Home Screen → compare vs `home_combined.png` (album art checkerboard, Hype/Rest logos inactive)
- [ ] Photograph Main Menu (selection_index=0/1/2) → compare vs `menu_1/2/3.png` (selection pills, static frame 0 or animating)
- [ ] Photograph Settings—Main → compare vs `settings_main.png` (Top Bar, first item selected)
- [ ] Photograph Settings—Display → compare vs `settings_display.png` (Top Bar)
- [ ] Photograph Settings—Brightness (selection_index=75) → verify slider at 75% position (Top Bar)
- [ ] Photograph Settings—Hype&Rest → compare vs `settings_hype_rest.png`
- [ ] Photograph Music Queue → compare vs `music_queue.png` (Top Bar, track list, layer-order correct)
- [ ] Photograph Guided Logging overview → compare vs `log_workout_1.png`
- [ ] Photograph Guided Logging set entry (repCount=10, weight=135) → verify "10 reps @ 135 lb" displays correctly
- [ ] All photographs match Lopaka references (no layer-order erasure, no missing elements)

## Seam 4: Screen-Specific Interaction Logic
- [ ] Main Menu scrolling: rotate encoder through Music → Workout → Settings → Music (wrapping verified)
- [ ] Settings—Brightness value change: start at 50, rotate CW 20 detents → value=61, rotate CCW 4 detents → value=59
- [ ] Guided Logging rep entry: start at 8 reps, rotate CW 2 detents → rep=9, rotate CW 4 detents → rep=11, rotate CCW 6 detents → rep=8
- [ ] Guided Logging weight entry: start at 135 lb, rotate CW 2 detents → weight=137.5 lb, rotate CCW 2 detents → weight=135 lb
- [ ] Navigation depth test: Home → Main Menu → Settings → Display → Brightness (depth=4), press Hype 4 times → depth=0 at Home
- [ ] Animated icon state transition: Main Menu with Music selected (animating), rotate to Workout → Music icon freezes on frame 0, Workout starts animating
- [ ] Global music button interaction: on Settings—Brightness, press Play/Pause → isPlaying toggles, Settings screen stays visible (nav stack unchanged)

## Phase 1 Exit Criterion
- [ ] Every screen in the locked screen tree (9 screens) is reachable via physical inputs
- [ ] All screens render correctly (visually matching Lopaka references, no layer-order erasure)
- [ ] Navigation works per spec (Hype=Back outside Home, encoder rotate/press, nav stack push/pop, Main Menu selection memory)
- [ ] All dummy data renders correctly (track title, artist, workout name, set/rep counts, brightness value)
- [ ] Hype/Rest dual-mode behavior works (Home → dummy Spotify trigger with visual feedback, other screens → Back/Info)
- [ ] Global music buttons work everywhere (Previous/Play-Pause/Next mutate dummy state without exiting current screen)
- [ ] Animated selection icons work (200ms/frame, cancel on transition)
- [ ] All 4 testing seams pass (Input queue events, App-logic nav state, visual rendering, interaction sequences)
