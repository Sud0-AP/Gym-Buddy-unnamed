# 05: Settings—Brightness + Encoder Value Adjustment

**What to build:** Settings—Brightness screen wired with encoder-adjustable brightness value (0-100). `selection_index` represents brightness value. Encoder rotation increments/decrements value, screen updates in real-time showing slider position change. Hardware tester can navigate to Settings—Brightness, rotate encoder CW and see brightness value increment by 1 every 2 detents, rotate CCW and see it decrement.

**Blocked by:** 04: Settings—Main + Settings—Display + Hype-as-Back

**Status:** ready-for-agent

- [ ] Settings—Brightness screen wired from `Hardware/Lopaka Screens/Settings/settings_display_brightness.txt`
- [ ] Screen renders with Top Bar
- [ ] `selection_index` represents brightness value (0-100 range)
- [ ] Encoder CW rotation increments brightness by 1 every 2 detents
- [ ] Encoder CCW rotation decrements brightness by 1 every 2 detents
- [ ] Brightness value wraps at boundaries: 100 → CW → 0, 0 → CCW → 100 (or clamps at 0/100, per UI spec)
- [ ] Screen updates in real-time as encoder rotates (slider position changes visually)
- [ ] Display task posts `DISPLAY_REFRESH_NEEDED` on every brightness change
- [ ] Seam 4 interaction testing passes: start at 50, rotate CW 20 detents → value=61, rotate CCW 4 detents → value=59
- [ ] Seam 3 testing passes: photograph at brightness=25, 50, 75 → verify slider position matches value
- [ ] Hype button press returns to Settings—Display (Back action works)
