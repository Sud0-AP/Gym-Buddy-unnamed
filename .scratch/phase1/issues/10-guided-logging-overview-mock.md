# 10: Guided Logging Screens (Overview + Mock Set Entry)

**What to build:** Guided Logging screens 1, 2, 3, 5 (overview, exercise list, workout in progress, stats summary) wired from Lopaka exports with dummy workout data, fully navigable. Screen 4 (set/rep entry) renders as a **mock** — static display showing "10 reps @ 135 lb" but NOT interactable yet (encoder rotation doesn't change values). Encoder-selectable UI elements (Start Workout button, exercise list items) in selected/unselected states (orphan assets wired). Hardware tester navigates Home → Main Menu → Workout → Guided Logging overview, sees exercise list, rotates encoder to select exercise, presses encoder to "start workout" (logs intent but stays on overview for now), navigates through screens 1→2→3→5.

**Blocked by:** 09: Animated Selection Icons

**Status:** ready-for-agent

- [ ] Guided Logging screen 1 (overview) wired from `Hardware/Lopaka Screens/Log Workout/log_workout_1.txt`
- [ ] Guided Logging screen 2 (exercise list) wired from `log_workout_2.txt`
- [ ] Guided Logging screen 3 (workout in progress) wired from `log_workout_3.txt`
- [ ] Guided Logging screen 5 (workout complete / stats summary) wired from `log_workout_5.txt`
- [ ] Guided Logging screen 4 (set/rep entry) wired from `log_workout_4.txt` as **MOCK ONLY** — renders static "10 reps @ 135 lb" placeholder, encoder rotation does NOT change values yet
- [ ] All screens render with Top Bar
- [ ] Dummy workout data added to `dummy_state.h`: workout name, exercise list (3-5 exercises with name, target sets/reps/weight)
- [ ] Encoder press on Main Menu with Workout selected (`selection_index=1`) pushes Guided Logging overview onto nav stack
- [ ] Overview screen shows workout name + "Start Workout" button (encoder-selectable)
- [ ] Encoder rotation on overview selects "Start Workout" button (selected/unselected orphan asset states wired)
- [ ] Encoder press on "Start Workout" logs intent to serial but stays on overview (full workout flow deferred to Ticket 11)
- [ ] Exercise list screen shows 3-5 dummy exercises, encoder rotation selects exercise, each renders with selected/unselected state (orphan assets wired)
- [ ] Encoder press on exercise list navigates to mock set entry screen (screen 4)
- [ ] Mock set entry screen renders "Exercise: Bench Press, Set 1, 10 reps @ 135 lb" (static text, not interactable)
- [ ] Hype button press on any Guided Logging screen pops nav stack (Back action)
- [ ] Seam 3 testing passes: photograph screens 1, 2, 3, 5 → compare vs `log_workout_{1,2,3,5}.png` references
- [ ] Screen 4 photograph matches `log_workout_4.png` layout but value editing NOT tested yet (deferred to Ticket 11)
