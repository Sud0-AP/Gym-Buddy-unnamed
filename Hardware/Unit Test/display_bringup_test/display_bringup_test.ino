// display_bringup_test.ino
// Phase 1 exit checks: "Display wired + rendering confirmed via TFT_eSPI"
//
// This is a standalone bring-up test -- NOT the app itself. It doesn't touch
// draw_screen() or any of the real screen tree yet; it just proves the panel
// is wired correctly and TFT_eSPI is configured correctly, one step at a time.
//
// Requires: TFT_eSPI library installed with User_Setup.h from this folder
// dropped in as described there.

#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("TFT_eSPI ST7789 bring-up test starting...");

  tft.init();
  tft.setRotation(1); // landscape -- try 1 or 3 if orientation looks wrong; 240x320
                       // panel in landscape should report width=320, height=240
                       // after this call

  Serial.printf("Reported size after rotation: %d x %d\n", tft.width(), tft.height());

  runTestSequence();
}

void loop() {
  // Cycle the test pattern every 3s so you can confirm nothing is a one-shot fluke
  static unsigned long last = 0;
  if (millis() - last > 3000) {
    last = millis();
    runTestSequence();
  }
}

void runTestSequence() {
  // 1. Solid fills -- confirms SPI wiring + no stuck pixels/color channel issues
  tft.fillScreen(TFT_RED);
  delay(400);
  tft.fillScreen(TFT_GREEN);
  delay(400);
  tft.fillScreen(TFT_BLUE);
  delay(400);
  tft.fillScreen(TFT_BLACK);

  // 2. Border rectangle at the extreme edges -- confirms the reported resolution
  //    actually matches the physical panel (a common failure mode is an off-by-
  //    one-panel-size where the last row/col is cut off or wraps)
  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_WHITE);

  // 3. Text at a few sizes/positions -- confirms font loading + coordinate system
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("ST7789 240x320 bring-up", 10, 10, 2);
  tft.drawString("Rotation: 1 (landscape)", 10, 30, 2);

  char buf[48];
  snprintf(buf, sizeof(buf), "Reported: %dx%d", tft.width(), tft.height());
  tft.drawString(buf, 10, 50, 2);

  // 4. The project's actual accent color, to sanity-check RGB order early --
  //    per 02-hardware-ui.md this should read as magenta/purple, not green/yellow
  tft.fillRect(10, 80, 60, 30, 0xE33F);
  tft.drawString("Accent 0xE33F", 80, 88, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("(should look magenta/purple,", 10, 130, 1);
  tft.drawString(" not green -- if wrong, flip", 10, 142, 1);
  tft.drawString(" TFT_RGB_ORDER in User_Setup.h)", 10, 154, 1);

  // 5. A small filled circle in each corner -- confirms no dead SPI byte /
  //    off-by-one causing corner artifacts, common with a bad CS/DC line
  int r = 8;
  tft.fillCircle(r, r, r, TFT_YELLOW);
  tft.fillCircle(tft.width() - r, r, r, TFT_YELLOW);
  tft.fillCircle(r, tft.height() - r, r, TFT_YELLOW);
  tft.fillCircle(tft.width() - r, tft.height() - r, r, TFT_YELLOW);

  Serial.println("Test sequence drawn.");
}
