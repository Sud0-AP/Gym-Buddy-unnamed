#include <Arduino.h>
#include "display/display_task.h"
#include "input/input_task.h"
#include "app/app_task.h"
#include "storage/storage_task.h"

#define FIRMWARE_VERSION "v0.1.0-phase1"

void setup() {
    Serial.begin(115200);
    // Allow USB CDC to initialize if connected
    delay(500);

    Serial.println();
    Serial.println("========================================");
    Serial.println("   GYM BUDDY FIRMWARE - " FIRMWARE_VERSION);
    Serial.println("   Target: Seeed Studio XIAO ESP32S3");
    Serial.println("========================================");

    // Initialize hardware, navigation stack, and queues
    storage_task_init();   // Initialize NVS first
    display_task_init();
    input_task_init();
    app_task_init();

    // Spawn FreeRTOS tasks (Display: priority 3, Input: priority 2, App-logic: priority 1, Storage: priority 0)
    display_task_start(3, 1);
    input_task_start(2, 1);
    app_task_start(1, 1);
    storage_task_start(0, 1);  // Priority 0 as specified in ticket

    Serial.println("[MAIN] Phase 1 tasks spawned successfully.");
}

void loop() {
    // Heartbeat every 5 seconds
    vTaskDelay(pdMS_TO_TICKS(5000));
    Serial.printf("[HEARTBEAT] Uptime: %lu ms, Free Heap: %lu bytes\n", millis(), esp_get_free_heap_size());
}
