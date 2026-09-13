# 11: Guided Logging Set Entry + Full Workout Flow

**What to build:** Screen 4 (set/rep/weight entry) fully interactive — encoder rotation changes rep count (±1 per 2 detents) and weight (±2.5 lb per 2 detents). "Start Workout" button from overview screen now pushes first exercise's set entry screen onto nav stack. Full workout flow works: Start Workout → enter set 1 reps → encoder press confirms → enter set 1 weight → confirm → next set (or next exercise if sets complete) → workout complete screen when all exercises done. Hype-as-Back works mid-workout to return to overview.

**Blocked by:** 10: Guided Logging Screens (Overview + Mock Set Entry)

**Status:** ready-for-agent

- [ ] Screen 4 (set/rep/weight entry) now fully interactive (remove mock from Ticket 10)
- [ ] Set entry screen has two modes: rep entry mode (encoder changes reps) and weight entry mode (encoder changes weight)
- [ ] Encoder CW rotation in rep mode increments rep count by 1 every 2 detents
- [ ] Encoder CCW rotation in rep mode decrements rep count by 1 every 2 detents
- [ ] Encoder CW rotation in weight mode increments weight by 2.5 lb every 2 detents
- [ ] Encoder CCW rotation in weight mode decrements weight by 2.5 lb every 2 detents
- [ ] Encoder press in rep mode confirms reps → switches to weight mode
- [ ] Encoder press in weight mode confirms weight → advances to next set (or next exercise if all sets complete for current exercise)
- [ ] Screen updates in real-time as encoder rotates (rep count / weight value changes visually)
- [ ] "Start Workout" button on overview now functional: encoder press pushes first exercise's set 1 entry screen onto nav stack
- [ ] After completing all sets for an exercise, app-logic advances to next exercise's set 1 entry screen automatically
- [ ] After completing all exercises, app-logic pushes workout complete screen (screen 5) onto nav stack
- [ ] Workout complete screen shows summary: total sets completed, total volume (sets × reps × weight summed across exercises)
- [ ] Hype button press mid-workout pops nav stack (returns to overview, workout session is abandoned)
- [ ] Encoder press on workout complete screen pops nav stack (returns to Main Menu)
- [ ] Seam 4 interaction testing passes: start workout → enter reps (10) → confirm → enter weight (135 lb) → confirm → see "Set 2" prompt → enter reps/weight for set 2 → advance through 2-3 exercises → see workout complete screen
- [ ] Seam 4 edge case: start workout → press Hype mid-set-entry → verify returns to overview (session abandoned)
- [ ] Serial logs confirm workout progression: "Set 1 complete: 10 reps @ 135 lb", "Exercise 1 complete", "Workout complete: 3 exercises, 200 lb total volume"
