#include "storage_task.h"
#include <Preferences.h>

QueueHandle_t g_storage_queue = nullptr;
static Preferences prefs;

// Phase 1 dummy defaults
static const uint32_t DUMMY_DEFAULTS[] = {
    [KEY_HYPE_DURATION_SEC] = 90,           // 1 min 30 sec
    [KEY_REST_DURATION_SEC] = 150,          // 2 min 30 sec
    [KEY_HYPE_PLAYLIST_INDEX] = 0,          // "Varjish"
    [KEY_REST_PLAYLIST_INDEX] = 1,          // "Super chill"
    [KEY_HYPE_EXERCISE_BASED_TIME] = 0,     // false
    [KEY_REST_EXERCISE_BASED_TIME] = 1,     // true
    [KEY_BRIGHTNESS] = 50,                  // 50%
};

static uint32_t storage_read_dummy(uint32_t key) {
    if (key < sizeof(DUMMY_DEFAULTS) / sizeof(DUMMY_DEFAULTS[0])) {
        Serial.printf("[STORAGE] NVS_READ key=%u → dummy value=%u\n", key, DUMMY_DEFAULTS[key]);
        return DUMMY_DEFAULTS[key];
    }
    Serial.printf("[STORAGE] NVS_READ key=%u → unknown key, returning 0\n", key);
    return 0;
}

static void storage_handle_event(const StorageEvent& evt) {
    switch (evt.type) {
        case StorageEventType::NVS_READ: {
            uint32_t value = storage_read_dummy(evt.key);
            // Phase 1: no response queue mechanism, just log
            // Future: implement response via queue or callback
            break;
        }
        case StorageEventType::NVS_WRITE: {
            // Phase 1: ignore writes (no persistence)
            Serial.printf("[STORAGE] NVS_WRITE key=%u, value=%u (ignored in Phase 1)\n",
                          evt.key, evt.value);
            break;
        }
        default:
            Serial.printf("[STORAGE] Unknown event type: %d\n", static_cast<int>(evt.type));
            break;
    }
}

static void storage_task_loop(void* pvParameters) {
    (void)pvParameters;
    Serial.println("[STORAGE] Storage task started on Core " + String(xPortGetCoreID()));

    StorageEvent evt;
    for (;;) {
        if (g_storage_queue != nullptr && xQueueReceive(g_storage_queue, &evt, portMAX_DELAY) == pdTRUE) {
            storage_handle_event(evt);
        }
    }
}

void storage_task_init() {
    // Initialize NVS
    bool nvs_ok = prefs.begin("gym-buddy", false);  // false = read/write mode
    if (nvs_ok) {
        Serial.println("[STORAGE] NVS initialized successfully (Preferences API)");
    } else {
        Serial.println("[STORAGE] NVS init failed — using dummy defaults only");
    }

    // Create queue
    g_storage_queue = xQueueCreate(8, sizeof(StorageEvent));
    if (g_storage_queue == nullptr) {
        Serial.println("[STORAGE] ERROR: Failed to create storage queue");
    } else {
        Serial.println("[STORAGE] Storage task initialized with 8-entry queue");
    }
}

void storage_task_start(UBaseType_t priority, BaseType_t core) {
    xTaskCreatePinnedToCore(
        storage_task_loop,
        "storage_task",
        4096,  // 4KB stack as specified in ticket
        nullptr,
        priority,
        nullptr,
        core
    );
}
