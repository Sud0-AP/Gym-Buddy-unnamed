#include "dummy_state.h"
#include <string.h>

DummyMusicState g_dummy_music;
DummyWorkoutState g_dummy_workout;
DummySettingsState g_dummy_settings;

void dummy_state_init() {
    // Initialize music state with partial track progress
    strcpy(g_dummy_music.trackTitle, "Spaceship");
    strcpy(g_dummy_music.artistName, "Kanye West");
    g_dummy_music.isPlaying = true;
    g_dummy_music.progressMs = 67000;   // 1:07
    g_dummy_music.durationMs = 260000;  // 4:20
    g_dummy_music.hypeActive = false;
    g_dummy_music.restActive = false;
    g_dummy_music.hypeTimerSeconds = 150;  // 2:30 (last used, not active)
    g_dummy_music.restTimerSeconds = 180;  // 3:00

    // Initialize workout state
    strcpy(g_dummy_workout.workoutName, "Push A");
    strcpy(g_dummy_workout.currentExercise, "Incline dumbbell p..");
    strcpy(g_dummy_workout.muscleGroup, "chest");
    strcpy(g_dummy_workout.targetReps, "4x10-12");

    strcpy(g_dummy_workout.sets[0].label, "Warmup: 15 Kg X 8");
    g_dummy_workout.sets[0].isWarmup = true;
    g_dummy_workout.sets[0].isCurrent = false;

    strcpy(g_dummy_workout.sets[1].label, "Set 1 : 20 Kg X 9");
    g_dummy_workout.sets[1].isWarmup = false;
    g_dummy_workout.sets[1].isCurrent = false;

    strcpy(g_dummy_workout.sets[2].label, "Set 2 : 20 Kg X 8-10");
    g_dummy_workout.sets[2].isWarmup = false;
    g_dummy_workout.sets[2].isCurrent = true;   // Current set

    strcpy(g_dummy_workout.sets[3].label, "Set 3 : 20 Kg X 8-10");
    g_dummy_workout.sets[3].isWarmup = false;
    g_dummy_workout.sets[3].isCurrent = false;

    strcpy(g_dummy_workout.sets[4].label, "Set 4 : 20 Kg X 8-10");
    g_dummy_workout.sets[4].isWarmup = false;
    g_dummy_workout.sets[4].isCurrent = false;

    g_dummy_workout.currentSetIndex = 2;  // Set 2 is current

    // Initialize settings state
    g_dummy_settings.brightness = 75;
    g_dummy_settings.wifiConnected = true;
    g_dummy_settings.batteryPct = 85;
}
