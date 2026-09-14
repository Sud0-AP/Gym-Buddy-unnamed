#include "display_task.h"
#include "../config/pins.h"
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#define COLOR_DARK_GREY 0x18C3 // RGB565 for #1A1A1A
#define COLOR_ACCENT    0xE33F // Project accent (magenta/purple in BGR mode)

QueueHandle_t g_display_queue = nullptr;
static TFT_eSPI tft = TFT_eSPI();

void display_task_render_boot_screen() {
    tft.fillScreen(COLOR_DARK_GREY);

    tft.setTextColor(TFT_WHITE, COLOR_DARK_GREY);
    tft.setTextDatum(TL_DATUM);
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

    DisplayEvent evt;
    for (;;) {
        if (xQueueReceive(g_display_queue, &evt, portMAX_DELAY) == pdTRUE) {
            switch (evt.type) {
                case DisplayEventType::DISPLAY_TEST_PATTERN:
                    display_task_render_boot_screen();
                    break;
                case DisplayEventType::DISPLAY_REFRESH_NEEDED:
                    // Future tickets will call draw_screen(screen_id, state)
                    break;
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
