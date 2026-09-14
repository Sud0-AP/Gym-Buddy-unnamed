# 01: Project Scaffold + Display + Input Pipeline

**What to build:** PlatformIO project builds and uploads successfully to XIAO ESP32S3. Device boots, Display task initializes TFT_eSPI with the 4 required non-default settings (TFT_RGB_ORDER TFT_BGR, TFT_INVERSION_OFF, CGRAM_OFFSET, USE_HSPI_PORT), renders a solid color screen with "Hello World" text. Input task decodes encoder rotation (GPIO D6/D7) and 5 buttons (PCF8574T at 0x20) into semantic events with 2-detent accumulation, posts to 8-entry FreeRTOS queue. Serial monitor logs each event as it's posted.

Hardware tester can rotate encoder 2 detents and see `ENCODER_CW` logged, press Hype button and see `BUTTON_HYPE_PRESS` logged. Build system, display driver, and input pipeline proven end-to-end.

**Blocked by:** None (can start immediately)

**Status:** completed

- [x] PlatformIO project structure created at `Hardware/Firmware/` with `platformio.ini` + `src/main.cpp`
- [x] Libraries configured: TFT_eSPI (with 4 non-default settings), Wire, Preferences
- [x] Display task initializes TFT_eSPI correctly, renders solid color + "Hello World" text on boot
- [x] Input task decodes encoder rotation with 2-detent accumulator (simple counter, rollover at threshold)
- [x] Input task decodes 5 button presses (PCF8574T 0x20) into semantic events
- [x] Input task posts semantic events to 8-entry FreeRTOS queue
- [x] Serial monitor (115200 baud) logs each semantic event as it's posted
- [x] Seam 1 testing passes: rotate encoder CW 2 detents → `ENCODER_CW` event logged, press Hype → `BUTTON_HYPE_PRESS` logged
- [x] Hardware test: encoder rotation + all 5 buttons + encoder push-button produce correct logged events
