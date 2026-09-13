# 06: Settings—Hype&Rest + Storage Task NVS Init

**What to build:** Settings—Hype&Rest screen wired with encoder-selectable UI elements (selected/unselected orphan asset states identified and wired). Storage task initializes NVS at boot, handles `NVS_READ` queue requests by returning dummy defaults for Settings values. Hardware tester can navigate to Settings—Hype&Rest, rotate encoder to select different settings, see focused element visually distinct (selected state). Storage task serial logs show NVS init success and dummy default responses.

**Blocked by:** 05: Settings—Brightness + Encoder Value Adjustment

**Status:** ready-for-agent

- [ ] Settings—Hype&Rest screen wired from `Hardware/Lopaka Screens/Settings/settings_hype_rest.txt`
- [ ] Screen renders with Top Bar
- [ ] Encoder-selectable UI elements identified (duration settings, playlist selectors, etc.)
- [ ] Orphan assets for selected/unselected states identified per element and wired
- [ ] Encoder rotation moves selection between UI elements
- [ ] Selected element renders with selected-state orphan asset, unselected elements render with unselected-state asset
- [ ] Storage task (priority 0, 4KB stack) created and running
- [ ] Storage task initializes NVS at boot (`nvs_flash_init()` or Preferences.begin())
- [ ] Storage task listens for `NVS_READ` queue requests
- [ ] Storage task responds to `NVS_READ` with dummy default values (e.g. hype duration=120s, rest duration=180s)
- [ ] Storage task ignores `NVS_WRITE` requests (no persistence in Phase 1, Settings changes are RAM-only)
- [ ] Serial logs confirm NVS init success and dummy defaults returned on read requests
- [ ] Seam 3 testing passes: photograph Settings—Hype&Rest with different elements selected, compare vs reference
