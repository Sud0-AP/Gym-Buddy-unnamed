# 08: Music Queue Rendering

**What to build:** Music Queue screen wired from Lopaka export with dummy track list data, layer-order verified (bring-up test fix still correct). Hardware tester navigates Home → Main Menu → Music (encoder press on Music item) and sees Music Queue render with dummy track list. **No animation yet** — Main Menu Music item still shows static frame 0.

**Blocked by:** 07: Home Screen + Dummy State + Global Music Buttons

**Status:** ready-for-agent

- [ ] Music Queue screen wired from `Hardware/Lopaka Screens/Music queue/music_queue.txt`
- [ ] Lopaka code reviewed for layer-order (background before foreground) — verify bring-up test layer-order fix is still correct
- [ ] Screen renders with Top Bar (WiFi disconnected, dummy battery level)
- [ ] Dummy track list data added to `dummy_state.h` (`DummyMusicQueueState` with array of tracks: title, artist per track)
- [ ] Music Queue renders dummy track list (3-5 placeholder tracks)
- [ ] Encoder press on Main Menu with Music selected (`selection_index=0`) pushes Music Queue onto nav stack
- [ ] Hype button press on Music Queue pops nav stack (returns to Main Menu)
- [ ] Seam 3 testing passes: photograph Music Queue, compare vs `music_queue.png` reference
- [ ] No silent erasure: verify Top Bar icons + track text all render correctly (no layer-order bugs)
- [ ] Main Menu Music item still shows static frame 0 (animation deferred to Ticket 9)
