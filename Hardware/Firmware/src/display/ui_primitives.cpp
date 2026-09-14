#include "ui_primitives.h"
#include "assets_common.h"

void draw_topbar() {
    // Layer 1: Margins/background frame drawn FIRST so it sits behind text/icons
    tft.pushImage(0, 0, 320, 21, image_topbar_margins_pixels);

    // Battery level fill inside case (75% default for Phase 1)
    tft.fillRect(296, 5, 12, 9, COLOR_ACCENT);

    // Battery case outline
    tft.drawBitmap(286, 2, image_topbar_battery_case_bits, 24, 16, COLOR_GREY_TRACK);

    // Time text
    tft.setTextColor(COLOR_ACCENT);
    tft.setTextSize(1);
    tft.drawString("12:23 PM", 75, 6);

    // Back icon
    tft.drawBitmap(10, 5, image_topbar_back_bits, 10, 8, COLOR_ACCENT);

    // Info icon
    tft.drawBitmap(45, 5, image_topbar_info_bits, 7, 8, COLOR_ACCENT);

    // Day & date text
    tft.drawString("Monday: 14th May", 140, 6);

    // Network connected icon
    tft.drawBitmap(252, 3, image_topbar_network_connected_bits, 15, 14, COLOR_ACCENT_NET);
}

void draw_scrollbar_thumb(uint8_t sel) {
    uint16_t ty = THUMB_Y_POSITIONS[sel];
    tft.fillRect(303, ty, 9, 15, COLOR_ACCENT);

    if (sel == 0) {
        tft.drawLine(304, 62, 310, 62, COLOR_GREY_INNER);
        tft.drawLine(304, 65, 310, 65, COLOR_GREY_INNER);
        tft.drawLine(304, 68, 310, 68, COLOR_GREY_INNER);
        tft.drawLine(304, 71, 310, 71, COLOR_GREY_INNER);
    } else if (sel == 1) {
        tft.drawBitmap(304, 109, image_scrollbar_center_marker, 7, 10, COLOR_GREY_TRACK);
    } else {
        tft.drawLine(304, 170, 310, 170, COLOR_GREY_INNER);
        tft.drawLine(304, 173, 310, 173, COLOR_GREY_INNER);
        tft.drawLine(304, 176, 310, 176, COLOR_GREY_INNER);
        tft.drawLine(304, 179, 310, 179, COLOR_GREY_INNER);
    }
}
