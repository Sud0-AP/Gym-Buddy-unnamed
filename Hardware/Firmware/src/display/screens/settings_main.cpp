#include "../screens.h"
#include "../assets_common.h"
#include "../ui_primitives.h"
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

// Settings—Main menu item labels
static const char* SETTINGS_MAIN_LABELS[3] = { "Display", "Hype & Rest", "Pairing" };

static void draw_settings_main_item(uint8_t index, bool is_selected) {
    uint16_t pill_y = ITEM_Y_POSITIONS[index];
    uint16_t icon_y = ICON_Y_POSITIONS[index];
    uint16_t text_y = TEXT_Y_POSITIONS[index];

    if (is_selected) {
        tft.drawBitmap(10, pill_y, image_selection_pill_bits, 260, 72, COLOR_ACCENT);
        tft.setTextColor(COLOR_TEXT_SEL);
    } else {
        tft.fillRect(10, pill_y, 260, 72, COLOR_BG);
        tft.setTextColor(COLOR_TEXT_UNSEL);
    }

    // Animation icon placeholder (static frame 0)
    tft.drawRect(17, icon_y, 48, 48, COLOR_GREY_INNER);

    // Item label
    tft.setTextSize(2);
    tft.drawString(SETTINGS_MAIN_LABELS[index], 100, text_y);
}

void draw_settings_main(uint8_t selection_index) {
    if (selection_index > 2) selection_index = 0;

    tft.fillScreen(COLOR_BG);
    draw_topbar();

    // Scrollbar track and arrows
    tft.fillRect(302, 32, 12, 194, COLOR_GREY_TRACK);
    tft.fillRect(304, 34, 7, 190, COLOR_GREY_INNER);
    tft.drawBitmap(301, 24, image_scrollbar_arrow_up_large, 14, 8, COLOR_GREY_TRACK);
    tft.drawBitmap(304, 30, image_scrollbar_arrow_up_small, 7, 4, COLOR_GREY_INNER);
    tft.drawBitmap(301, 226, image_scrollbar_arrow_dn_large, 14, 8, COLOR_GREY_TRACK);
    tft.drawBitmap(304, 224, image_scrollbar_arrow_dn_small, 7, 4, COLOR_GREY_INNER);

    // Draw all 3 items
    for (uint8_t i = 0; i < 3; i++) {
        draw_settings_main_item(i, i == selection_index);
    }

    draw_scrollbar_thumb(selection_index);
}

void draw_settings_main_update_selection(uint8_t old_index, uint8_t new_index) {
    if (old_index == new_index) return;
    if (old_index > 2) old_index = 0;
    if (new_index > 2) new_index = 0;

    draw_settings_main_item(old_index, false);
    draw_settings_main_item(new_index, true);

    uint16_t old_ty = THUMB_Y_POSITIONS[old_index];
    tft.fillRect(303, old_ty, 9, 15, COLOR_GREY_INNER);
    draw_scrollbar_thumb(new_index);
}
