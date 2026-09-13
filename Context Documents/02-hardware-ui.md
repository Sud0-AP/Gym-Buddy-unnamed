# Hardware UI/UX — Design System, Interaction Model & Screen Behavior

This doc merges the visual design system, the global interaction patterns, and the
per-screen behavior spec into one reference. All interaction behavior below is carried
forward **unchanged in spirit** from the original BLE-based design (it was already fully
resolved through a screen-by-screen discussion) — only terminology and a small number of
screens change for the WiFi/Firebase pivot. Anything genuinely new or re-opened by the
pivot is marked **[NEW]** or **[OPEN]**.

---

## Part 1 — Design system

- **Palette**: near-black background, magenta/purple (`#c060e0`-ish / `0xE33F` in
  RGB565) as the primary accent for borders, headers, and selected states; white for
  primary readable text; dimmer purple/gray (`0x506C`/`0x6870`) for secondary/inactive
  text and unselected outlines.
- **Typeface**: monospace/pixel-style bitmap font throughout.
- **Layout convention**: landscape, header bar consistent across nearly every
  non-home/non-setup screen:
  - Left: contextual icons — back arrow (↩) and/or help (?)
  - Center: current time + date
  - Right: **connection status icon, battery level icon**. **[NEW]** The old Bluetooth
    icon is replaced with a WiFi icon. Battery icon unchanged in position, changed in
    rendering (see below).

**Top bar icon states (locked, simplified):**
- **WiFi icon**: **2 states only** — connected / disconnected. No separate
  "connecting/reconnecting" glyph and no signal-strength bars — a simple filled-vs-
  crossed-out WiFi glyph is enough; the device is either reachable or it isn't, and a
  third transitional state isn't worth the extra sprite for how rarely it's actually
  visible (a reconnect attempt is usually resolved in under a second).
- **Battery icon**: a battery-outline rectangle whose **fill width is one of 5 fixed
  steps — 0%, 25%, 50%, 75%, 100%** — driven by rounding `batteryPct` down to the
  nearest step (matches the existing `drawTopBar(..., batteryPct)` signature in Part 6,
  just changes how the fill renders). **No color change at any level** — fill is always
  the same color, only the rectangle's filled width changes. No charging animation in
  v1 — charging state, if shown at all, is a static plug glyph rather than an animated
  fill, to avoid spending an animation budget on the top bar.
- **Selection state convention**: currently-selected list item/button gets a solid
  rounded-rectangle magenta outline (vs. thin gray for unselected) — the visual language
  for "this is what the encoder will activate."
- **Icon boxes**: square placeholder slots next to menu items/exercise names — animated
  (sprite-frame loop) when selected, static frame 0 when not, per the fixed local icon
  set shipped with firmware.

**Display resolution — 240×320.** Changed from 240×280. **All screen layouts, including
how the extra vertical space is used on each screen, are authored directly by you in
Lopaka and exported as TFT_eSPI code** — this is not a decision for the coding agent to
make. The agent's job is to wire each exported screen's drawing code behind the
`draw_screen(screen_id, state)` interface (`01-hardware-firmware.md`) and drive it with
real/dummy state, not to design or adjust proportions itself. When a screen's Lopaka
export isn't available yet, the agent should ask for it rather than inventing a layout.

**Lopaka export gotcha (confirmed during Music Queue bring-up) — layer order does not
automatically mean draw order matches visual intent.** Any layer that is a full-region
background — a selection pill's background fill, the top bar's background/margin
graphic, a card background, anything drawn as a `pushImage()`/`fillRect()` covering a
rectangular area other elements sit inside of — must be exported so it draws **before**
those other elements, or its `pushImage()` call silently repaints over (erases) text and
icons that were already drawn in that region. Lopaka's canvas layer order doesn't
enforce this automatically; it's possible to end up with a background layer exported
*after* the foreground content it's supposed to sit behind, and the failure mode looks
like "text/icons just don't render" rather than an obvious visual glitch, which makes it
easy to misdiagnose as a font, color, or wiring problem instead of a draw-order problem.

**Two things to do about this going forward, for every screen:**
1. **When authoring in Lopaka**: put every full-region background layer (selection
   pills, top-bar backgrounds, card backgrounds, etc.) at the **bottom** of that screen's
   layer stack before exporting, so it's guaranteed to generate first.
2. **When wiring an already-exported screen behind `draw_screen()`**: if any text or
   icon isn't appearing despite being present in the generated code, check whether a
   `pushImage()`/`fillRect()` call covering that same screen region appears *after* it
   in the function body — reorder that background call to immediately after
   `tft.fillScreen()` (or immediately before the first foreground element it sits behind)
   rather than assuming the missing content itself is the bug.

---

## Part 2 — Global input mapping

1. **Rotary encoder rotation (CW/CCW)**: moves selection cursor in a list/menu.
2. **Rotary encoder push (SELECT)**: activates the focused element.
3. **3 fixed music buttons** (reachable from every screen, always send the global music
   command regardless of what's on screen):
   - Button 1: Previous / rewind
   - Button 2: Play/Pause
   - Button 3: Next/skip
4. **Hype / Rest (context-dependent)**:
   - **On the Home screen only**: trigger real Hype/Rest behavior (see
     `05-music-control.md`). This is deliberately the *only* screen where these buttons
     do this — the device is designed to live on Home between actions (it shows both
     the workout overview and the music/now-playing block together), so the trigger is
     always reachable without navigating anywhere first. Typical flow: log a set in
     Guided Logging → Back to Home → press Rest.
   - **On every other screen** (Guided Logging, Exercise Stats, Music Queue, Settings,
     any menu/list screen): **Hype (left) = Back**, **Rest (right) = Info** — not
     encoder-reachable, deliberately, so Back/Info never require a long scroll on
     screens with long lists.
     - **Back**: navigates up one level (screen-specific destination, follows the
       nav-stack rules noted per-screen below).
     - **Info**: opens a transient overlay (no nav-stack entry) with short contextual
       help for the current screen; dismissed by pressing Info again or Back. Content is
       screen-specific and largely a content TODO, not blocking.

### Visual selection mechanisms (how the above renders)

- **Pattern A — bounding box / outline swap** (lists & menus): unselected = thin
  gray outline + dim text + static icon; selected = thick magenta outline + bright text
  + animated icon.
- **Pattern B — selection dot/ellipse fill** (header Back/Info icons): near-invisible
  when unselected, filled/bright when selected.
- **Pattern C — scrollbar thumb** (windowed lists, e.g. Main Menu): thumb position
  tracks `selectedIndex` directly, one fixed slot per item, no wraparound.
- **Pattern D — set marker** (Guided Logging): a horizontal box outline that shifts
  vertically between fixed row y-positions as the selected set changes.
- **Carousel pattern** (Choose Split, Choose Workout, and — as in-edit-mode feedback
  only — the Hype & Rest Playlist field): the selection pill stays visually fixed in one
  slot; the underlying list text scrolls behind it. Wraps around at list ends. On
  rotation, the pill does a brief directional nudge-and-spring-back (down for CW, up for
  CCW) as tactile acknowledgment that input registered.
- **Windowed-list-with-pinned-edge** (Music Queue): the pill moves between visible slots
  as you scroll; once it hits the top/bottom visible slot, it pins there and the list
  scrolls behind it instead. Clamps at real list ends — does not wrap.

### Navigation stack model (locked)

This is the mechanism underneath the flow already described in Part 3's screen tree and
the Home screen's behavior spec below — **Home is where the user rests by default;
rotating the encoder from Home always opens Main Menu, which branches to Music Queue,
Workout Logs (→ Choose Split → ... → Guided Logging), or Settings.** Nothing about that
changes; this section just names the underlying push/pop bookkeeping so a coding agent
implements one consistent mechanism instead of ad hoc per-screen logic:

- **The nav stack holds screen IDs only, not scroll/selection state**, with one
  documented exception: Main Menu, which explicitly remembers its last-selected item
  across a Back-in (per its own entry in Part 4 — "retraces last-selected state"). Every
  other screen resets to its default selection/scroll position on re-entry (first item
  selected, list scrolled to top) — this is deliberate, not an oversight: workout/menu
  screens are short-lived enough that remembering scroll position isn't worth the extra
  state, and a predictable "always starts at the top" is easier to navigate blind
  (no-look, mid-set usage) than a stack that sometimes remembers position.
- **Push**: any encoder-press or button-press that moves to a new screen pushes that
  screen onto the stack, *except* Home, which is never pushed — Home is the stack's
  implicit root and only ever reached by popping all the way back, or explicitly (Guided
  Logging's "Home screen" action, which is a **stack reset to `[Home]`**, not a push, so
  a subsequent Back from Home has nowhere to go, matching Home's documented "no Back
  action" behavior).
- **Pop (Back / Hype on non-Home screens)**: pops exactly one level, per the specific
  target already documented for that screen in Part 4. This is a plain LIFO pop, not a
  "smart" back that skips levels — the per-screen targets in Part 4 already encode the
  intended UX (e.g. Guided Logging's Back going to Workout Overview, not Choose Workout),
  so the stack itself needs no special-case logic beyond standard push/pop.
- **In-place edit modes are not stack entries** (Settings — Hype & Rest field edit,
  Guided Logging set edit): entering edit mode is a local state flag on the current
  screen, not a push. Hype/Back while editing exits edit mode and returns to the *same*
  screen's navigation state (documented per-screen as "cancels without committing/saving,
  returns to row/field navigation") — it does not pop the nav stack.
- **Depth**: the deepest chain in the current screen tree is Home → Main Menu → Choose
  Split → Choose Workout → Workout Overview → Guided Logging → Exercise Stats (7 levels).
  A fixed-size stack of **8 entries** is sufficient with no risk of overflow given the
  screen tree above; the app-logic task should treat a push past that depth as a bug
  (assert/log), not a case to handle gracefully, since the fixed screen tree makes it
  provably unreachable.

### Global rules (apply everywhere)

- **Text overflow**: every text element has a fixed-width bounding box; if a string
  doesn't fit, truncate with "…" and marquee-scroll to reveal the rest every few seconds.
- **Boundary clamping vs. wraparound**: fixed-slot list screens (Main Menu, Settings
  branches) clamp at the boundaries, no wraparound. Carousel-pattern screens (Choose
  Split/Workout) wrap around. Windowed lists with a pinned edge (Music Queue) clamp, no
  wrap.

---

## Part 3 — Screen tree

```
[DEVICE CLAIM / SETUP]  ── one-time, before any account is linked ── [NEW]
  │
  ▼
[HOME SCREEN] (Combined / Music Only / Workout Only)
  │
  ├── Encoder CW/CCW ────────► [MAIN MENU]
  │                              ├── 1. Workout Logs ──► [CHOOSE SPLIT]
  │                              │                         └── [CHOOSE WORKOUT]
  │                              │                               └── [WORKOUT OVERVIEW]
  │                              │                                     └── [GUIDED LOGGING]
  │                              │                                           ├── [EXERCISE STATS]
  │                              │                                           └── (Back / Finish)
  │                              ├── 2. Music Queue ───► [MUSIC QUEUE]
  │                              └── 3. Settings ──────► [SETTINGS MAIN]
  │                                                        ├── Display
  │                                                        │     ├── Brightness
  │                                                        │     ├── Theme (unmocked)
  │                                                        │     └── Home screen pref (unmocked)
  │                                                        ├── Hype & Rest
  │                                                        └── Network  ── renamed from
  │                                                              "Pairing", WiFi status/
  │                                                              network picker  [NEW/CHANGED]
```

---

## Part 4 — Per-screen behavior

Behavior below is carried forward unchanged from the resolved BLE-era spec except where
marked. Where a screen referenced "the app" or "phone connection," read that as "the web
app" / "backend connection" going forward — the interaction model itself doesn't change.

### Home screen (Combined / Workout only / Music only)

**Purpose**: default/idle screen; shows workout progress and/or music playback.

**Encoder rotate**: no selectable list on Home itself — either direction transitions to
Main Menu, always opening with Workout Logs pre-selected.

**Encoder press**: context shortcut — active workout → Guided Logging; no active workout
→ Music Queue.

**Buttons**: Hype/Rest always trigger real behavior here (never Back/Info). 3 fixed music
buttons behave as everywhere else.

**Exit points**: purely a root screen, no Back action — only forward via rotate/press.

**Mode indicator**: single generic "current mode" box shows whichever of HYPE/REST is
active or was last active, with icon/label/timer (ticking if active, static if not) — not
a fixed per-variant display.

**Components**: top bar; large album-art box (real synced Spotify art, see
`05-music-control.md`); exercise info + set list (logged sets show actual values, current
set highlighted with target range, future sets show target range as plain text); music
block (progress bar, title, artist); mode indicator box. Workout-only and music-only
variants are the same screen family with layout emphasis shifted and the
inapplicable-context content dropped (workout-only has no music block; music-only adds an
"Up next" 2-track preview of the same queue Music Queue shows in full).

---

### Main Menu

**Entry**: from Home (rotate) → always resets to Workout Logs selected. From any
submenu's Back → retraces last-selected state (only the Home→Menu path forces reset).

**Encoder rotate**: moves selection among the 3 items, clamps at boundaries.

**Encoder press**: Workout Logs → Choose Split; Music Queue → Music Queue; Settings →
Settings Main.

**Buttons**: Hype = Back → Home; Rest = Info overlay.

---

### Choose Split → Choose Workout → Workout Overview → Guided Logging → Exercise Stats

Unchanged carousel/list-select chain from the original spec:

- **Choose Split**: carousel select (wraps) among splits; press → Choose Workout for that
  split; Hype/Back → Main Menu.
- **Choose Workout**: carousel select (wraps) among that split's workouts; press →
  Workout Overview; Hype/Back → Choose Split.
- **Workout Overview**: cycles `[Start] ↔ [Change Workout] ↔ [Exercise 1] ↔ … ↔
  [Exercise N]` (wraps); Start → Guided Logging (full sequential flow from exercise 1);
  Change Workout → Choose Split; individual exercise row → Guided Logging focused on just
  that exercise, returns here when done; Hype/Back → Main Menu.
- **Guided Logging**: cycles `Home screen ↔ Stop ↔ Stats ↔ Set 1…Set N ↔ Previous ↔
  Next` (wraps). Press on a set row enters in-place edit (weight → reps → save, each step
  confirmed by encoder press, auto-advances to next pending set on save; Hype/Back
  mid-edit cancels without saving, returns to row navigation). Stats → Exercise Stats.
  Home screen → backgrounds the workout, goes Home. Stop → Workout Overview
  (session ends). Previous/Next → adjacent exercise. Hype/Back (outside edit) → Workout
  Overview.
- **Exercise Stats**: read-only (notes, last session, personal best); single "Done/Back"
  action or Hype/Back → returns to Guided Logging.

---

### Music Queue

**Entry**: Main Menu → Music Queue, or Home encoder-press shortcut when no workout active
— same screen either way.

**Components**: fixed "Now Playing" block (non-interactive) + scrollable list: Repeat row,
Shuffle row, then upcoming tracks — shuffle/repeat are two rows in the *same* selectable
list as the tracks, not a separate toggle UI.

**Encoder rotate**: windowed-list-with-pinned-edge scroll, clamps (Repeat is the topmost
stop, last queued track is bottommost, no wrap).

**Encoder press**: on Repeat/Shuffle → toggles it; on a track row → jumps playback to it.

**Buttons**: Hype/Back → retraces entry point (Home or Main Menu); Rest = Info overlay.

---

### Settings — Main / Display / Brightness / Hype & Rest

Unchanged from the original spec:

- **Settings Main**: fixed 3-slot list (Display / Hype & Rest / Network), same pattern as
  Main Menu, clamps at boundaries. Hype/Back → Main Menu.
- **Settings — Display**: fixed 3-slot list (Brightness / Theme / Home screen). Theme and
  Home-screen-preference sub-screens still unmocked.
- **Settings — Brightness**: live slider, encoder rotate adjusts brightness and updates a
  legibility preview in real time; encoder press has no distinct action (brightness
  applies live); Hype/Back → Settings — Display.
- **Settings — Hype & Rest**: two mirrored blocks (Hype, Rest), each with an "Exercise
  based time" checkbox, Minute/Second fields, and a Playlist field. Field-navigation
  model: rotate (out of edit) moves focus between fields; press enters edit mode; rotate
  (in edit) changes the value (checkbox flips, minute/second increments, playlist cycles
  the shortlist with the carousel pill-nudge feedback); press again commits and returns
  to navigation; Hype/Back while editing cancels without committing. When "Exercise based
  time" is ticked, that block's Minute/Second fields grey out and are skipped in focus
  order (duration instead comes from the per-exercise `target hype/rest seconds` field —
  see `04-workout-logging.md`). Hype/Back (outside edit) → Settings — Main.

---

### Settings — Network **[CHANGED from "Pairing"] — [OPEN, layout deferred]**

**Purpose**: WiFi status and network management, replacing the old BLE pairing screen.

**Layout: intentionally left open — you're designing this screen yourself in Lopaka.**
Not a blocker for the coding agent; implement whatever screen you hand over. The
following are **locked constraints the design needs to satisfy**, since they come from
already-locked backend/firmware behavior rather than visual design:

- Must be able to show current connection status and the saved-network list (data:
  `{ssid, priority}` per network, matching the `savedNetworks` schema in
  `03-web-app.md` — no passwords ever shown or entered on-device).
- The "add a network" action's primary path is the web-app-push flow (per the locked
  hybrid provisioning model), with manual SoftAP re-entry as a secondary/fallback
  action — **whatever UI you design for this, make SoftAP-entry require a deliberate
  two-step action (not a single accidental press)**, since entering SoftAP mode drops
  the device off its normal network.
- Needs some way to show device-claim status and trigger "forget device" (which clears
  the stored claim token and returns the device to the Device Claim/Setup screen).

### Device Claim / Setup screen **[NEW] — [OPEN, layout deferred]**

**Purpose**: one-time screen shown only before the device has a stored claim token (see
`01-hardware-firmware.md`). Not part of the normal navigable screen tree — no Back
target, since there's nothing to go back to yet.

**Layout: intentionally left open — you're designing this screen yourself in Lopaka.**
Locked constraints the design needs to satisfy:

- **Sequencing (locked): WiFi first, then claim.** The device cannot poll the
  claim-status endpoint (see `01-hardware-firmware.md`) without a working connection, so
  the SoftAP + captive-portal WiFi flow always completes first; this screen only appears
  once the device has working WiFi.
- Must show, in some form: a way to link the device to an account (QR code and/or a
  manually-enterable code — your call on exact presentation), and a status indicator
  reflecting the `GET /device/claim/status` poll result (pending → fulfilled).
- No encoder-selectable list needed — there's nothing to navigate to yet, and no
  meaningful Back target.

Not a blocker for the coding agent: implement whatever screen design you hand over
against the `GET /device/claim/status` polling behavior already locked in
`01-hardware-firmware.md`.

---

## Part 5 — Cross-cutting items still open

- ~~Top bar disconnected/low-battery state~~ — **resolved**, see the locked icon states
  under Part 1.
- ~~Navigation stack rules~~ — **resolved**, see the locked nav-stack model under Part 2.
- **Global music button behavior — mid-edit edge case (resolved, locked)**: the 3 fixed
  music buttons remain always-reachable and always send their command, **including**
  while a set is mid-edit in Guided Logging or a field is mid-edit in Settings — Hype &
  Rest. Pressing one does **not** exit edit mode or discard the in-progress edit — the
  edit state is untouched, the music command is sent to the backend in the background,
  and (if applicable) the now-playing data updates on whichever screen displays it next.
  Rationale: forcing an edit-mode exit as a side effect of an unrelated global button
  would violate the "screen-based context, not gesture complexity" principle and would
  be surprising during exactly the moment (mid-set, hands busy) this device is designed
  for.
- ~~"No active Spotify device" state~~ — **resolved**, no device-picker screen needed.
  Spotify only ever has one active playback device at a time account-wide; the device's
  commands (play/pause/next/volume/etc.) always target whichever device is currently
  active, with no device selection required on this end. The only edge case is **nothing
  playing anywhere** — see `05-music-control.md` for the inline (non-screen) handling of
  that state.

## Part 6 — Firmware coding model reference

```cpp
struct MenuState {
    uint8_t selectedIndex;
    uint8_t totalItems;
    bool isDirty;
    uint8_t animFrame;
};

void drawMainMenu(uint8_t selectedIndex, uint8_t animFrame) {
    tft.fillScreen(0x801);
    drawTopBar("12:23 PM", "14th May Monday", /*wifiConnected=*/true, /*batteryPct=*/85);
    drawMenuItem(0, "Workout Logs", 52, selectedIndex == 0, animFrame);
    drawMenuItem(1, "Music Queue", 125, selectedIndex == 1, animFrame);
    drawMenuItem(2, "Settings", 198, selectedIndex == 2, animFrame);
    int thumbY = 59 + (selectedIndex * 47);
    tft.fillRect(265, thumbY, 9, 15, 0xE33F);
}
```

Screens should be built as parametric drawing functions taking state (not hardcoded
per-variant), matching the pattern above. Only clear/redraw the animated icon's bounding
box on animation ticks, not the full screen, to preserve SPI bandwidth on this
MCU/display combination.
