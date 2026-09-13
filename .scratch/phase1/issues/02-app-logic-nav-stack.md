# 02: App-logic Task + Nav Stack (Home → Main Menu)

**What to build:** App-logic task consumes semantic events from the 8-entry queue, manages an 8-entry nav stack with NavigationState structs (screen_id, selection_index, scroll_offset). Device boots to Home screen (dummy solid color). Rotating encoder pushes Main Menu onto nav stack. Serial monitor logs nav stack depth and screen_id after each transition.

Hardware tester can rotate encoder on Home and confirm Main Menu is reachable (logged to serial but not rendered yet — Display task calls draw_screen() in next ticket).

**Blocked by:** 01: Project Scaffold + Display + Input Pipeline

**Status:** ready-for-agent

- [ ] NavigationState struct defined: uint8_t screen_id, uint8_t selection_index, uint16_t scroll_offset
- [ ] 8-entry fixed nav stack array with push/pop operations
- [ ] App-logic task consumes semantic events from Input task queue
- [ ] Home screen is default on boot: nav_stack_depth = 0, screen_id = HOME_SCREEN
- [ ] ENCODER_CW event on Home screen pushes Main Menu onto nav stack (depth = 1, screen_id = MAIN_MENU, selection_index = 0)
- [ ] Serial monitor logs nav stack state after each transition: depth, screen_id, selection_index
- [ ] Seam 2 testing passes for Home → Main Menu transition: rotate encoder on Home → see "nav_stack_depth=1, screen_id=MAIN_MENU" logged
