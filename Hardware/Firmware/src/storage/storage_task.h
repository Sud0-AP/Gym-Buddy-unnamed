#pragma once

#include <Arduino.h>

// Storage task queue events
enum class StorageEventType : uint8_t {
    NVS_READ = 0,
    NVS_WRITE,
};

struct StorageEvent {
    StorageEventType type;
    uint32_t key;      // Which setting to read/write
    uint32_t value;    // Value to write (ignored for reads)
};

// Storage settings keys (Phase 1 dummy defaults)
enum StorageKey : uint32_t {
    KEY_HYPE_DURATION_SEC = 0,
    KEY_REST_DURATION_SEC,
    KEY_HYPE_PLAYLIST_INDEX,
    KEY_REST_PLAYLIST_INDEX,
    KEY_HYPE_EXERCISE_BASED_TIME,
    KEY_REST_EXERCISE_BASED_TIME,
    KEY_BRIGHTNESS,
};

// Queue handle (exposed so other tasks can send requests)
extern QueueHandle_t g_storage_queue;

// Init and start
void storage_task_init();
void storage_task_start(UBaseType_t priority, BaseType_t core);
