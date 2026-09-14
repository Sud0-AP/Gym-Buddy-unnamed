#include "../screens.h"
#include "../assets_common.h"
#include "../ui_primitives.h"
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

// Labels local to Main Menu
static const char* MENU_LABELS[3] = { "Workout Logs", "Music Queue", "Settings" };

static void draw_menu_item(uint8_t index, bool is_selected) {
    uint16_t pill_y = ITEM_Y_POSITIONS[index];
    uint16_t icon_y = ICON_Y_POSITIONS[index];
    uint16_t text_y = TEXT_Y_POSITIONS[index];

    if (is_selected) {
        // Draw the pill frame around this item
        tft.drawBitmap(20, pill_y, image_menu_selection_pill_bits, 260, 72, COLOR_ACCENT);
        tft.setTextColor(COLOR_TEXT_SEL);
    } else {
        // Clear pill area by drawing dark background in pill bounding box
        tft.fillRect(20, pill_y, 260, 72, COLOR_BG);
        tft.setTextColor(COLOR_TEXT_UNSEL);
    }

    // Animation icon placeholder (static frame 0)
    tft.drawRect(27, icon_y, 48, 48, COLOR_GREY_INNER);

    // Item label
    tft.setTextSize(2);
    tft.drawString(MENU_LABELS[index], 110, text_y);
}

void draw_main_menu_full(uint8_t selection_index) {
    if (selection_index > 2) selection_index = 0;

    // Fill screen with background
    tft.fillScreen(COLOR_BG);

    // Top Bar with margin lines, battery, network, date, time, back, info
    draw_topbar();

    // Scrollbar track (x=302, y=32, w=12, h=194)
    tft.fillRect(302, 32, 12, 194, COLOR_GREY_TRACK);
    tft.fillRect(304, 34, 7, 190, COLOR_GREY_INNER);

    // Scrollbar arrows at top and bottom
    tft.drawBitmap(301, 24, image_scrollbar_arrow_up_large, 14, 8, COLOR_GREY_TRACK);
    tft.drawBitmap(304, 30, image_scrollbar_arrow_up_small, 7, 4, COLOR_GREY_INNER);
    tft.drawBitmap(301, 226, image_scrollbar_arrow_dn_large, 14, 8, COLOR_GREY_TRACK);
    tft.drawBitmap(304, 224, image_scrollbar_arrow_dn_small, 7, 4, COLOR_GREY_INNER);

    // Draw all 3 items (unselected first, then selected)
    for (uint8_t i = 0; i < 3; i++) {
        draw_menu_item(i, i == selection_index);
    }

    // Draw scrollbar thumb
    draw_scrollbar_thumb(selection_index);
}

void draw_main_menu_update_selection(uint8_t old_index, uint8_t new_index) {
    if (old_index == new_index) return;
    if (old_index > 2) old_index = 0;
    if (new_index > 2) new_index = 0;

    // 1. Un-select old item (clear its pill and redraw text as unselected)
    draw_menu_item(old_index, false);

    // 2. Select new item (draw its pill and text as selected)
    draw_menu_item(new_index, true);

    // 3. Update scrollbar thumb: restore track over old thumb, draw new thumb
    uint16_t old_ty = THUMB_Y_POSITIONS[old_index];
    tft.fillRect(303, old_ty, 9, 15, COLOR_GREY_INNER);
    draw_scrollbar_thumb(new_index);
}

// Compatibility wrappers for direct calls
void draw_menu_1() { draw_main_menu_full(0); }
void draw_menu_2() { draw_main_menu_full(1); }
void draw_menu_3() { draw_main_menu_full(2); }
