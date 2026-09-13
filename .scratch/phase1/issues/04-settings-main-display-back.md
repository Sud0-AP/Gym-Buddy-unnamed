# 04: Settings—Main + Settings—Display + Hype-as-Back

**What to build:** Settings—Main and Settings—Display screens wired from Lopaka exports with Top Bar. Encoder press on Main Menu (Settings selected) pushes Settings—Main onto nav stack. Hype button press on Settings screens pops nav stack (Back action). Hardware tester can navigate Home → Main Menu → Settings → Display, press Hype and return to Settings—Main, press Hype again and return to Main Menu. Main Menu selection memory works (Settings still selected after returning).

**Blocked by:** 03: Top Bar Helper + Main Menu Rendering

**Status:** ready-for-agent

- [ ] Settings—Main screen wired from `Hardware/Lopaka Screens/Settings/settings_main.txt`
- [ ] Settings—Display screen wired from `Hardware/Lopaka Screens/Settings/settings_display.txt`
- [ ] Both screens render with Top Bar (WiFi disconnected, dummy battery level)
- [ ] Encoder press on Main Menu with Settings selected (selection_index=2) pushes Settings—Main onto nav stack
- [ ] Encoder press on Settings—Main with Display selected pushes Settings—Display onto nav stack
- [ ] Hype button press on Settings—Display pops nav stack (returns to Settings—Main)
- [ ] Hype button press on Settings—Main pops nav stack (returns to Main Menu)
- [ ] Main Menu selection_index persists across nav pops (Settings still selected when returning from Settings submenu)
- [ ] Seam 2 testing passes: navigate Home → Main Menu → Settings → Display, press Hype twice → verify nav stack depth decrements correctly, Main Menu still shows Settings selected
- [ ] Seam 3 testing passes: photograph Settings—Main and Settings—Display, compare vs Lopaka references
- [ ] Encoder-selectable UI elements on Settings screens show selected/unselected states (orphan assets identified and wired)
