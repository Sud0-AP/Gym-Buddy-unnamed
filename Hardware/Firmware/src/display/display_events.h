#pragma once

#include <stdint.h>

enum class DisplayEventType : uint8_t {
    NONE = 0,
    DISPLAY_REFRESH_NEEDED,
    DISPLAY_TEST_PATTERN
};

struct DisplayEvent {
    DisplayEventType type;
    uint32_t payload;
};
