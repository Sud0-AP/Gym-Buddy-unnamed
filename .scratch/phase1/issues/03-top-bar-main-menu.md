# 03: Top Bar Helper + Main Menu Rendering

**What to build:** `draw_top_bar()` helper extracted from Music Queue Lopaka export, reviewed for layer-order. Main Menu screen wired from Lopaka exports with 3 selection states, selection pills rendering correctly (layer-order verified, orphan assets for per-item pills wired). Display task calls `draw_screen(MAIN_MENU, state)` on `DISPLAY_REFRESH_NEEDED` event. Device renders Main Menu on screen when nav stack transitions from Home. Encoder rotation increments selection_index modulo 3, selection pill moves to highlight selected item.

Hardware tester can navigate Home → Main Menu, rotate encoder, see selection pill move every 2 detents. **Animated icons NOT included yet** — Main Menu items show static frame 0 only in this ticket.

**Blocked by:** 02: App-logic Task + Nav Stack (Home → Main Menu)

**Status:** ready-for-agent

- [ ] `draw_top_bar(wifi_connected, battery_level)` helper function extracted from Music Queue Lopaka export
- [ ] Top Bar code reviewed for layer-order (background before foreground)
- [ ] Top Bar renders 2-state WiFi icon + 5-step battery icon correctly
- [ ] Main Menu screen wired from Lopaka exports (menu_1.txt, menu_2.txt, menu_3.txt)
- [ ] Main Menu selection pills reviewed for layer-order (pill background before text/icons)
- [ ] Orphan assets for per-item selection pills identified and wired (Main Menu pills differ per item)
- [ ] Display task calls `draw_screen(screen_id, state)` on `DISPLAY_REFRESH_NEEDED` event
- [ ] Main Menu renders on screen when nav stack transitions from Home
- [ ] Encoder rotation on Main Menu increments selection_index modulo 3 (wrapping Music→Workout→Settings→Music)
- [ ] Selection pill moves to highlight selected item
- [ ] Seam 3 testing passes: photograph Main Menu at selection_index 0/1/2, compare vs Lopaka references
- [ ] Seam 4 testing passes: rotate encoder through all 3 items, verify wrapping behavior
- [ ] Main Menu items show static frame 0 (animation deferred to Ticket 9)
