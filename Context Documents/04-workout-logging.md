# Workout Logging — Model, Offline Behavior & Progress Analysis

## Division of responsibility (unchanged — locked)

- **Template/library creation happens exclusively in the web app.** The device has no
  text entry capability, so exercises, splits, workout templates, and target
  sets/reps/weight schemes are all authored on the web.
- **The device executes and logs against templates.** During a session it shows the
  plan, lets the user scroll/select/confirm via the encoder, and records actual
  performance (weight/reps per set) as numeric values.
- **Full history and long-term analytics live in Firestore.** The device only ever holds
  a small rolling cache (recent template(s) + last-session/PR data) so it can operate
  standalone.

## Workout selection & guided flow (unchanged — locked)

Choose Split → Choose Workout → Workout Overview → Guided Logging, exactly as specified
in `02-hardware-ui.md`. For each exercise: name, target muscles, full set breakdown
(warmup + work sets with target weight/rep range), encoder-editable current row
(weight → reps → confirm), Previous/Next between exercises, a Stats screen (last session
+ personal best), and Stop/Home controls.

### Encoder weight/reps in-place edit — concrete UX (locked, updated for lb/kg + half-rep)

Per `02-hardware-ui.md`'s field-navigation model (rotate to move focus, press to enter
edit, rotate to change value, press to commit): within a set row's edit mode, focus
order is **weight → reps → save**, matching the doc's existing "weight → reps → save"
description, now with concrete increments and starting values:

- **Weight field**: starting value is the target weight for that set (from the
  template) if this is the first edit of that row this session, otherwise the
  previously-entered value for that row. Increment per encoder detent depends on the
  account's `unitPreference` (see below): **±2.5 lb**, or **±0.5 kg**. Holding the
  encoder button down while rotating is *not* used for a coarser increment in v1 (no
  long-press disambiguation, per the project's stated interaction philosophy) — the
  default is always the template target or last entry, never zero, keeping the
  realistic number of detents low regardless of unit.
- **Reps field**: starting value is the target reps (template) or previous entry, same
  rule as weight. Increment per detent is **±1 rep** by default, or **±0.5 rep** if
  `halfRepIncrementEnabled` is on (see below) — half reps display as e.g. "8.5" and are
  stored as-is (the analysis math in this doc already operates on plain numeric reps, so
  a 0.5 value needs no special-casing there).
- **Save**: press on the reps field's confirm commits the row and auto-advances to the
  next pending set, per the existing spec.
- **Units and half-reps (locked, both user-configurable, both web-app-only toggles —
  no on-device UI needed for either, they're account-level settings the device just
  reads from its synced config)**:
  - `unitPreference: "lb" | "kg"` (Firestore field, see `03-web-app.md`) — **default
    "lb"**. Changing it changes the increment used for every future edit; it does not
    retroactively convert previously-logged weights (those stay recorded in whatever
    unit was active when logged — the web app's history view should show the unit each
    entry was logged in, not silently convert old data).
  - `halfRepIncrementEnabled: bool` — **default false** (whole reps only). When
    enabled, the reps field's increment becomes ±0.5 instead of ±1; when disabled
    (default), reps only ever land on whole numbers.

## Rest timing (unchanged — locked, explicitly separate from Hype/Rest music)

No automatic rest-between-sets timer tied to logging a set — deliberate, since real gym
sessions have unpredictable social/equipment gaps. Music-backed rest support is manual,
via the Rest button (`05-music-control.md`).

## Offline behavior (unchanged — locked)

The device must run an entire guided workout session, log all sets, and show stats with
**zero network connection**, using the on-device cache. When a connection is available,
logged data syncs up to the backend (`POST /device/logs`, see `03-web-app.md`), and
updated templates/settings sync down. First-class requirement, not a fallback path.

## Data needed on-device per cached template (unchanged — locked)

- Template structure: split name, workout name, ordered exercise list
- Per exercise: name, target muscle groups, warmup/work set targets (weight range, rep
  range), and — new field, confirmed during the original UI discussion —
  `targetHypeSeconds` / `targetRestSeconds` for the "exercise-based time" Hype/Rest
  option
- Per exercise: last-session actuals and personal-best actuals (enough to render Stats
  offline)

---

## Progress analysis & metrics (locked — v1 scope)

The web app's history view is the natural place for this — the device only ever needs
the *inputs* to these computations (last session + PR, already scoped above), not the
computations themselves. **All metrics below, including progression suggestions, ship
in v1** — this was the one genuinely subjective call in this doc and it's been made:
ship the full menu rather than cutting it down.

### Core computed metrics (per exercise, per session)

- **Session volume**: Σ(weight × reps) across all work sets for that exercise in a
  session. **Warmup sets are excluded from volume totals** (locked — standard
  convention; warmup weights aren't a meaningful training-stimulus signal and including
  them would distort week-over-week comparisons).
- **Tonnage trend**: session volume plotted over time, per exercise and rolled up per
  target-muscle-group per week.
- **Estimated 1-rep max (e1RM)**: computed per top set. **Formula locked: Epley**
  (`weight × (1 + reps/30)`) — simple, reasonably accurate in the sub-10-rep range this
  app's set/rep ranges mostly cover, and avoids the need for a formula-choice setting in
  v1. (Brzycki remains a plausible v2 addition if you want to compare formulas later,
  but there's no user-facing toggle for it now.) e1RM trend over time is the primary
  "am I getting stronger" signal, since it normalizes for rep variation.
- **Personal records**: **multiple PR types**, all tracked from day one — heaviest-
  weight-for-any-reps, best-e1RM, and best-volume-in-a-session — rather than a single
  "PR" value, since lifters commonly care about more than one of these.

### Progression signals (locked — in scope for v1)

- **Plateau detection**: flags an exercise where e1RM hasn't improved over the last
  **5 sessions** (locked — midpoint of the originally proposed 4–6 range, simple to
  reason about) despite consistent training frequency. Surfaced as a nudge in the web
  app's history view — the device never pushes this unprompted.
- **Progression suggestion (double progression model) — in scope for v1 (locked,
  revised behavior gated by an `autoProgressionEnabled` toggle):**
  - **Algorithm (unchanged)**: once the user hits the top of a work set's target rep
    range for all sets at a given weight in a session, the next session's target for
    that exercise becomes weight-up-one-increment (2.5 lb / 0.5 kg, matching the
    account's unit) with reps reset to the bottom of the range. If the top of the range
    wasn't hit, next session's target stays exactly the same as this session's — the
    "if I was able to do it, bump it a little; if not, keep it" behavior described in
    plain terms.
  - **`autoProgressionEnabled: false` (default)**: the computed suggestion is a
    **review item** — surfaced in the web app's per-exercise history view as a "Ready to
    progress?" prompt with the suggested new weight/rep-range. The user accepts or
    ignores it there; accepting updates that exercise's template target, which then
    syncs down to the device as normal. Nothing changes on-device automatically.
  - **`autoProgressionEnabled: true`**: the computed suggestion **is applied
    automatically** to that exercise's effective next-session target — no web app visit
    required. Concretely: the daily analysis run (see "Where these run" below) writes
    the new target directly to that exercise's template, which syncs to the device via
    the normal `GET /device/config` poll, so Guided Logging simply shows the updated
    target next time with no separate suggestion/approval step anywhere.
  - This toggle is account-wide (applies to every exercise), not per-exercise — keeps
    the setting simple and matches how it was described (a single on/off preference).
- **Weekly volume per muscle group**: aggregate tonnage across all exercises tagged with
  a given target muscle, useful for eyeballing whether training volume is balanced or
  drifting for a given muscle group week over week.

### Where these run (locked, revised — daily/on-session-write, not on-demand)

Computed **server-side** (Cloud Functions), **triggered on write to
`users/{uid}/sessions/{sessionId}`** — i.e. right after each logged session syncs up,
not lazily when the web app happens to be opened. This is a change from a purely
on-demand model: since progression suggestions can now auto-apply
(`autoProgressionEnabled: true`) and need to be ready by the *next* session (which might
happen the same day the previous one synced), computing at write-time rather than
read-time is necessary, not just a nice-to-have. The device's MCU/flash budget is still
intentionally kept for offline session logging, not analytics — this Cloud Function
work is entirely server-side and doesn't change the device's role.

**What this computes and where it's shown (locked):**
- **Per-session, per-exercise**: session volume, e1RM, PR checks (all three PR types),
  next-session target (progression algorithm above) — written back to that exercise's
  history entry and, if `autoProgressionEnabled`, to its template target.
- **"Sore muscles" overview**: a rolling **7-day tonnage-per-muscle-group** rollup
  (using the locked muscle taxonomy below), shown as a simple bar/heatmap on the **web
  app's Exercise section main/landing view** — at a glance, which muscle groups have
  had recent volume vs. which haven't, to help gauge what's due for training. Recomputed
  as part of the same write-triggered function (a new session's tonnage rolls into the
  7-day window immediately, no separate daily cron needed).
- **Per-exercise / per-workout progress view**: clicking into a specific exercise or
  workout in the web app shows its trend graphs (tonnage, e1RM) plotted per session/day
  over time, plus the plateau-detection nudge and (if `autoProgressionEnabled` is off)
  the "Ready to progress?" review prompt described above.

---

## Open questions to resolve during implementation

All product-scope decisions in this doc are now locked (full metric menu including
progression suggestions, warmups excluded from volume, Epley formula, encoder edit UX
with lb/kg + half-rep support, auto-progression toggle behavior — see above). One
implementation-level item remains, which is routine authoring work rather than a design
decision:

- **Muscle-group taxonomy**: a fixed tag list, authored per exercise in the web app,
  is needed for the weekly-volume-per-muscle-group rollup and the "sore muscles"
  overview. **Locked list** (shoulders split into front/side/rear per your feedback,
  rather than one combined "shoulders" tag): `chest`, `back`, `shoulders-front`,
  `shoulders-side`, `shoulders-rear`, `biceps`, `triceps`, `quads`, `hamstrings`,
  `glutes`, `calves`, `core`. Each exercise's `targetMuscles` field (see
  `03-web-app.md` schema) is one or more tags from this fixed list — a multi-select in
  the web app's exercise editor, not free text (consistent with the no-text-entry-on-
  device philosophy extending to keeping the web app's own data clean for reliable
  rollups).
