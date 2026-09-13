// preview_music_queue.ino
//
// Standalone preview of the Music Queue screen exported from Lopaka.
// Uses the confirmed-working User_Setup.h (ST7789, XIAO ESP32S3).
//
// This is NOT wired into draw_screen(screen_id, state) yet -- that's the
// next Phase 1 step. This is just "does the exported screen look right
// on the real panel," using the dummy state Lopaka baked in.

#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// ---- Paste the full contents of music_queue.txt below this line ----
// (kept as a separate #include so this file stays short and you can swap
//  in a different screen's Lopaka export without touching setup()/loop())
#include "music_queue.h"
// ----------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(300);

  tft.init();
  tft.setRotation(1); // landscape, 320x240 -- matches the Lopaka canvas
                       // (Lopaka screens are typically authored at native
                       // WxH for the target rotation; if the layout looks
                       // rotated 90 degrees, try setRotation(3) instead)

  Serial.printf("Panel size: %d x %d\n", tft.width(), tft.height());

  drawmusic_queue();  // <-- the Lopaka-generated function, called once
}

void loop() {
  // Static preview -- nothing to update yet since this isn't wired to
  // draw_screen() or real state. Re-flash to redraw if needed.
}
