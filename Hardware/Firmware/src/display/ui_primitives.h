#ifndef UI_PRIMITIVES_H
#define UI_PRIMITIVES_H

#include <Arduino.h>
#include <TFT_eSPI.h>

// Global TFT instance (defined in display_task.cpp)
extern TFT_eSPI tft;

// Shared rendering functions
void draw_topbar();
void draw_scrollbar_thumb(uint8_t selection_index);

#endif // UI_PRIMITIVES_H
