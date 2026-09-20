#pragma once

#include <stdint.h>

// Home screen drawing functions for all 3 variants
// Combined variant = music + workout
// Music-only variant = music + "Up next" preview
// Workout-only variant = workout + no music

void draw_home_screen_combined();
void draw_home_screen_music_only();
void draw_home_screen_workout_only();
