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

// Direct variant wrappers
void draw_menu_1();
void draw_menu_2();
void draw_menu_3();
