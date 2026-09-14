#pragma once

#include <stdint.h>

enum class InputEventType : uint8_t {
    NONE = 0,
    ENCODER_CW,
    ENCODER_CCW,
    BUTTON_ENCODER_PRESS,
    BUTTON_PREV_PRESS,
    BUTTON_PLAY_PAUSE_PRESS,
    BUTTON_NEXT_PRESS,
    BUTTON_HYPE_PRESS,
    BUTTON_REST_PRESS,
    SWITCH_LOCK,      // Deferred post-Phase-1
    SWITCH_WORKOUT,   // Deferred post-Phase-1
    SWITCH_MUSIC      // Deferred post-Phase-1
};

struct InputEvent {
    InputEventType type;
    uint32_t timestamp;
};

inline const char* input_event_to_string(InputEventType type) {
    switch (type) {
        case InputEventType::NONE: return "NONE";
        case InputEventType::ENCODER_CW: return "ENCODER_CW";
        case InputEventType::ENCODER_CCW: return "ENCODER_CCW";
        case InputEventType::BUTTON_ENCODER_PRESS: return "BUTTON_ENCODER_PRESS";
        case InputEventType::BUTTON_PREV_PRESS: return "BUTTON_PREV_PRESS";
        case InputEventType::BUTTON_PLAY_PAUSE_PRESS: return "BUTTON_PLAY_PAUSE_PRESS";
        case InputEventType::BUTTON_NEXT_PRESS: return "BUTTON_NEXT_PRESS";
        case InputEventType::BUTTON_HYPE_PRESS: return "BUTTON_HYPE_PRESS";
        case InputEventType::BUTTON_REST_PRESS: return "BUTTON_REST_PRESS";
        case InputEventType::SWITCH_LOCK: return "SWITCH_LOCK";
        case InputEventType::SWITCH_WORKOUT: return "SWITCH_WORKOUT";
        case InputEventType::SWITCH_MUSIC: return "SWITCH_MUSIC";
        default: return "UNKNOWN";
    }
}
