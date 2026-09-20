#pragma once

#include <stdint.h>
#include <stdbool.h>

// Dummy state structs for Phase 1 testing - camelCase naming matches Firestore schema

struct DummyMusicState {
    char trackTitle[64];
    char artistName[64];
    bool isPlaying;
    uint32_t progressMs;    // Current playback position in milliseconds
    uint32_t durationMs;    // Track duration in milliseconds
    bool hypeActive;        // true when Hype mode timer is running
    bool restActive;        // true when Rest mode timer is running
    uint16_t hypeTimerSeconds;   // Hype timer remaining (for display)
    uint16_t restTimerSeconds;   // Rest timer remaining (for display)
};

struct DummyWorkoutState {
    char workoutName[64];
    char currentExercise[64];
    char muscleGroup[32];
    char targetReps[16];     // e.g. "4x10-12"

    // Set data (warmup + 4 working sets)
    struct SetData {
        char label[32];      // e.g. "Warmup: 15 Kg X 8"
        bool isWarmup;
        bool isCurrent;      // true for the set currently being performed
    } sets[5];

    uint8_t currentSetIndex; // 0-4, which set is highlighted
};

struct DummySettingsState {
    uint8_t brightness;      // 0-100
    bool wifiConnected;
    uint8_t batteryPct;      // 0-100
};

// Global dummy state instances
extern DummyMusicState g_dummy_music;
extern DummyWorkoutState g_dummy_workout;
extern DummySettingsState g_dummy_settings;

// Initialization function
void dummy_state_init();
