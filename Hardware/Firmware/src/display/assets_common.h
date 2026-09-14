#ifndef ASSETS_COMMON_H
#define ASSETS_COMMON_H

#include <Arduino.h>

// ===== COMMON COLOR CONSTANTS =====
extern const uint16_t COLOR_BG;
extern const uint16_t COLOR_ACCENT;
extern const uint16_t COLOR_ACCENT_NET;
extern const uint16_t COLOR_GREY_TRACK;
extern const uint16_t COLOR_GREY_INNER;
extern const uint16_t COLOR_TEXT_UNSEL;
extern const uint16_t COLOR_TEXT_SEL;

// ===== COMMON LAYOUT CONSTANTS =====
extern const uint16_t ITEM_Y_POSITIONS[3];
extern const uint16_t TEXT_Y_POSITIONS[3];
extern const uint16_t ICON_Y_POSITIONS[3];
extern const uint16_t THUMB_Y_POSITIONS[3];

// ===== TOP BAR ASSETS =====
extern const unsigned char image_topbar_back_bits[];
extern const unsigned char image_topbar_battery_case_bits[];
extern const unsigned char image_topbar_info_bits[];
extern const unsigned char image_topbar_network_connected_bits[];
extern const uint16_t image_topbar_margins_pixels[];

// ===== SCROLLBAR ASSETS =====
extern const unsigned char image_scrollbar_arrow_dn_large[];
extern const unsigned char image_scrollbar_arrow_up_large[];
extern const unsigned char image_scrollbar_arrow_dn_small[];
extern const unsigned char image_scrollbar_arrow_up_small[];
extern const unsigned char image_scrollbar_center_marker[];

// ===== SELECTION PILL ASSETS =====
extern const unsigned char image_menu_selection_pill_bits[];
extern const unsigned char image_settings_selection_pill_bits[];

#endif // ASSETS_COMMON_H
