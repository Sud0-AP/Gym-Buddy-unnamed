#pragma once

#include <TFT_eSPI.h>

// Global TFT instance (defined in display_task.cpp)
extern TFT_eSPI tft;

// Lopaka-generated drawing functions (exact copies from Lopaka exports)
void draw_topbar();              // Top bar (simplified for Phase 1)
void draw_menu_1();              // Main Menu with item 0 selected (Music Queue)
void draw_menu_2();              // Main Menu with item 1 selected (Workout Logs)
void draw_menu_3();              // Main Menu with item 2 selected (Settings)
