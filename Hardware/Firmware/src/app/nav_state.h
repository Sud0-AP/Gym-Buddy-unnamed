#pragma once

#include <stdint.h>

enum ScreenId : uint8_t {
    SCREEN_HOME = 0,
    SCREEN_MAIN_MENU,
    SCREEN_MUSIC_QUEUE,
    SCREEN_SETTINGS_MAIN,
    SCREEN_SETTINGS_DISPLAY,
    SCREEN_SETTINGS_BRIGHTNESS,
    SCREEN_SETTINGS_HYPE_REST,
    SCREEN_LOG_WORKOUT_OVERVIEW,
    SCREEN_LOG_WORKOUT_EXERCISE_LIST,
    SCREEN_LOG_WORKOUT_IN_PROGRESS,
    SCREEN_LOG_WORKOUT_SET_ENTRY,
    SCREEN_LOG_WORKOUT_COMPLETE,
    SCREEN_COUNT
};

struct NavigationState {
    uint8_t screen_id;
    uint8_t selection_index;
    uint16_t scroll_offset;
};

inline const char* screen_id_to_string(uint8_t id) {
    switch (id) {
        case SCREEN_HOME: return "HOME_SCREEN";
        case SCREEN_MAIN_MENU: return "MAIN_MENU";
        case SCREEN_MUSIC_QUEUE: return "MUSIC_QUEUE";
        case SCREEN_SETTINGS_MAIN: return "SETTINGS_MAIN";
        case SCREEN_SETTINGS_DISPLAY: return "SETTINGS_DISPLAY";
        case SCREEN_SETTINGS_BRIGHTNESS: return "SETTINGS_BRIGHTNESS";
        case SCREEN_SETTINGS_HYPE_REST: return "SETTINGS_HYPE_REST";
        case SCREEN_LOG_WORKOUT_OVERVIEW: return "LOG_WORKOUT_OVERVIEW";
        case SCREEN_LOG_WORKOUT_EXERCISE_LIST: return "LOG_WORKOUT_EXERCISE_LIST";
        case SCREEN_LOG_WORKOUT_IN_PROGRESS: return "LOG_WORKOUT_IN_PROGRESS";
        case SCREEN_LOG_WORKOUT_SET_ENTRY: return "LOG_WORKOUT_SET_ENTRY";
        case SCREEN_LOG_WORKOUT_COMPLETE: return "LOG_WORKOUT_COMPLETE";
        default: return "UNKNOWN";
    }
}
