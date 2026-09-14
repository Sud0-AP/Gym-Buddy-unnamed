#pragma once

#include <TFT_eSPI.h>

// Global TFT instance (defined in display_task.cpp)
extern TFT_eSPI tft;

// Top bar drawing (includes background margins, battery, network, time, date, back, info)
void draw_topbar();

// Full Main Menu render (clears screen and draws everything — use on screen entry)
void draw_main_menu_full(uint8_t selection_index);

// Fast in-place selection change (no fillScreen, zero flicker — only updates moved items)
void draw_main_menu_update_selection(uint8_t old_index, uint8_t new_index);

// Settings screens (full renders with Top Bar)
void draw_settings_main(uint8_t selection_index);
void draw_settings_display(uint8_t selection_index);

// Fast in-place selection updates for Settings screens (zero flicker)
void draw_settings_main_update_selection(uint8_t old_index, uint8_t new_index);
void draw_settings_display_update_selection(uint8_t old_index, uint8_t new_index);

// Settings—Brightness screen (brightness value as selection_index 0-100)
void draw_settings_brightness(uint8_t brightness_value);
void draw_settings_brightness_update_value(uint8_t old_value, uint8_t new_value);

// Direct variant wrappers
void draw_menu_1();
void draw_menu_2();
void draw_menu_3();
