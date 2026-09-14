#ifndef SCREENS_H
#define SCREENS_H

#include <Arduino.h>

// ===== MAIN MENU =====
void draw_main_menu_full(uint8_t selection_index);
void draw_main_menu_update_selection(uint8_t old_index, uint8_t new_index);

// ===== SETTINGS — MAIN =====
void draw_settings_main(uint8_t selection_index);
void draw_settings_main_update_selection(uint8_t old_index, uint8_t new_index);

// ===== SETTINGS — DISPLAY =====
void draw_settings_display(uint8_t selection_index);
void draw_settings_display_update_selection(uint8_t old_index, uint8_t new_index);

// ===== SETTINGS — BRIGHTNESS =====
void draw_settings_brightness(uint8_t brightness_value);
void draw_settings_brightness_update_value(uint8_t old_value, uint8_t new_value);

// Compatibility wrappers for direct menu calls
void draw_menu_1();
void draw_menu_2();
void draw_menu_3();

#endif // SCREENS_H
