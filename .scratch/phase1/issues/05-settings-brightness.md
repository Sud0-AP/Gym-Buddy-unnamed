# 05: Settings—Brightness + Encoder Value Adjustment

**What to build:** Settings—Brightness screen wired with encoder-adjustable brightness value (0-100). `selection_index` represents brightness value. Encoder rotation increments/decrements value, screen updates in real-time showing slider position change. Hardware tester can navigate to Settings—Brightness, rotate encoder CW and see brightness value increment by 1 every 2 detents, rotate CCW and see it decrement.

**Blocked by:** 04: Settings—Main + Settings—Display + Hype-as-Back

**Status:** completed

- [x] Settings—Brightness screen wired from `Hardware/Lopaka Screens/Settings/settings_display_brightness.txt`
- [x] Screen renders with Top Bar
- [x] `selection_index` represents brightness value (0-100 range)
- [x] Encoder CW rotation increments brightness (or decrements per hardware polarity)
- [x] Encoder CCW rotation decrements brightness (or increments per hardware polarity)
- [x] Brightness value clamps at boundaries: fixed at 0 and 100
- [x] Screen updates in real-time as encoder rotates (slider position changes visually)
- [x] Display task posts `DISPLAY_REFRESH_NEEDED` on every brightness change
- [x] Seam 4 interaction testing passes: encoder adjusts value, action-highlighted +/- buttons
- [x] Seam 3 testing passes: verified on hardware with user
- [x] Hype button press returns to Settings—Display (Back action works)
