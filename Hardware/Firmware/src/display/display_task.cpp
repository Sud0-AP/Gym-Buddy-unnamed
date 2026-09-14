#include "display_task.h"
#include "screens.h"
#include "ui_primitives.h"
#include "../config/pins.h"
#include "../app/app_task.h"
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#define COLOR_DARK_GREY 0x18C3 // RGB565 for #1A1A1A
#define COLOR_ACCENT    0xE33F // Project accent (magenta/purple in BGR mode)

QueueHandle_t g_display_queue = nullptr;
TFT_eSPI tft = TFT_eSPI();  // Global TFT instance (extern declared in screens.h)

void display_task_render_boot_screen() {
    // Reset all TFT state that might have leaked from other screens
    tft.setTextSize(1);       // Lopaka menu uses setTextSize(2) — reset to default
    tft.setTextDatum(TL_DATUM);
    tft.fillScreen(COLOR_DARK_GREY);

    tft.setTextColor(TFT_WHITE, COLOR_DARK_GREY);
    tft.drawString("Hello World!", 20, 30, 4);

    tft.setTextColor(COLOR_ACCENT, COLOR_DARK_GREY);
    tft.drawString("Gym Buddy - Phase 1 Bring-Up", 20, 80, 2);

    tft.setTextColor(TFT_WHITE, COLOR_DARK_GREY);
    tft.drawString("Display: ST7789 240x320 (320x240)", 20, 110, 2);
    tft.drawString("Rotate encoder or press buttons", 20, 140, 2);
    tft.drawString("to view input events on Serial.", 20, 160, 2);

    // Accent test rectangle
    tft.fillRect(20, 195, 120, 25, COLOR_ACCENT);
    tft.setTextColor(TFT_WHITE, COLOR_ACCENT);
    tft.drawString("Ready", 60, 200, 2);

    Serial.println("[DISPLAY] Boot test pattern rendered.");
}

static void display_task_loop(void* pvParameters) {
    Serial.println("[DISPLAY] Display task started on Core " + String(xPortGetCoreID()));

    display_task_render_boot_screen();

    // Track active screen and selection to enable flicker-free in-place updates
    uint8_t rendered_screen_id = SCREEN_HOME;
    uint8_t rendered_selection_index = 0;

    DisplayEvent evt;
    for (;;) {
        if (xQueueReceive(g_display_queue, &evt, portMAX_DELAY) == pdTRUE) {
            switch (evt.type) {
                case DisplayEventType::DISPLAY_TEST_PATTERN:
                    rendered_screen_id = SCREEN_HOME;
                    display_task_render_boot_screen();
                    break;
                case DisplayEventType::DISPLAY_REFRESH_NEEDED: {
                    const NavigationState& nav_state = app_get_current_nav_state();

                    // Always reset text state before rendering any screen
                    tft.setTextSize(1);
                    tft.setTextDatum(TL_DATUM);

                    if (nav_state.screen_id == SCREEN_MAIN_MENU) {
                        if (rendered_screen_id != SCREEN_MAIN_MENU) {
                            // First time entering Main Menu: full screen redraw
                            draw_main_menu_full(nav_state.selection_index);
                            Serial.printf("[DISPLAY] Full render Main Menu, selection=%u\n",
                                          nav_state.selection_index);
                        } else if (rendered_selection_index != nav_state.selection_index) {
                            // Already on Main Menu, only selection changed: fast in-place update (NO screen clear, zero flicker)
                            draw_main_menu_update_selection(rendered_selection_index, nav_state.selection_index);
                            Serial.printf("[DISPLAY] Fast selection update %u -> %u\n",
                                          rendered_selection_index, nav_state.selection_index);
                        }
                        rendered_screen_id = SCREEN_MAIN_MENU;
                        rendered_selection_index = nav_state.selection_index;
                    } else if (nav_state.screen_id == SCREEN_SETTINGS_MAIN) {
                        if (rendered_screen_id != SCREEN_SETTINGS_MAIN) {
                            // First time entering Settings—Main: full screen redraw
                            draw_settings_main(nav_state.selection_index);
                            Serial.printf("[DISPLAY] Full render Settings—Main, selection=%u\n", nav_state.selection_index);
                        } else if (rendered_selection_index != nav_state.selection_index) {
                            // Already on Settings—Main, only selection changed: fast in-place update (zero flicker)
                            draw_settings_main_update_selection(rendered_selection_index, nav_state.selection_index);
                            Serial.printf("[DISPLAY] Fast Settings—Main selection update %u -> %u\n",
                                          rendered_selection_index, nav_state.selection_index);
                        }
                        rendered_screen_id = SCREEN_SETTINGS_MAIN;
                        rendered_selection_index = nav_state.selection_index;
                    } else if (nav_state.screen_id == SCREEN_SETTINGS_DISPLAY) {
                        if (rendered_screen_id != SCREEN_SETTINGS_DISPLAY) {
                            // First time entering Settings—Display: full screen redraw
                            draw_settings_display(nav_state.selection_index);
                            Serial.printf("[DISPLAY] Full render Settings—Display, selection=%u\n", nav_state.selection_index);
                        } else if (rendered_selection_index != nav_state.selection_index) {
                            // Already on Settings—Display, only selection changed: fast in-place update (zero flicker)
                            draw_settings_display_update_selection(rendered_selection_index, nav_state.selection_index);
                            Serial.printf("[DISPLAY] Fast Settings—Display selection update %u -> %u\n",
                                          rendered_selection_index, nav_state.selection_index);
                        }
                        rendered_screen_id = SCREEN_SETTINGS_DISPLAY;
                        rendered_selection_index = nav_state.selection_index;
                    } else if (nav_state.screen_id == SCREEN_SETTINGS_BRIGHTNESS) {
                        if (rendered_screen_id != SCREEN_SETTINGS_BRIGHTNESS) {
                            // First time entering Settings—Brightness: full screen redraw
                            draw_settings_brightness(nav_state.selection_index);
                            Serial.printf("[DISPLAY] Full render Settings—Brightness, brightness=%u\n", nav_state.selection_index);
                        } else if (rendered_selection_index != nav_state.selection_index) {
                            // Already on Settings—Brightness, only value changed: fast in-place update (zero flicker)
                            draw_settings_brightness_update_value(rendered_selection_index, nav_state.selection_index);
                            Serial.printf("[DISPLAY] Fast Settings—Brightness value update %u -> %u\n",
                                          rendered_selection_index, nav_state.selection_index);
                        }
                        rendered_screen_id = SCREEN_SETTINGS_BRIGHTNESS;
                        rendered_selection_index = nav_state.selection_index;
                    } else {
                        // Returning to Home or other unhandled screen
                        rendered_screen_id = nav_state.screen_id;
                        rendered_selection_index = nav_state.selection_index;
                        Serial.printf("[DISPLAY] Screen %s not yet implemented\n",
                                      screen_id_to_string(nav_state.screen_id));
                        display_task_render_boot_screen();
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void display_task_init() {
    if (g_display_queue == nullptr) {
        g_display_queue = xQueueCreate(8, sizeof(DisplayEvent));
    }

    tft.init();
    tft.setRotation(1); // Landscape (320x240)
    Serial.printf("[DISPLAY] ST7789 Initialized. Dimensions: %d x %d\n", tft.width(), tft.height());
}

void display_task_start(UBaseType_t priority, BaseType_t core) {
    xTaskCreatePinnedToCore(
        display_task_loop,
        "display_task",
        4096,
        nullptr,
        priority,
        nullptr,
        core
    );
}
