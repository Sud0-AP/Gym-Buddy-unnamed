#include "app_task.h"
#include "../input/input_events.h"
#include "../input/input_task.h"
#include "../display/display_events.h"
#include "../display/display_task.h"
#include "../display/screens.h"
#include <Arduino.h>

// Global Hype&Rest state
HypeRestState g_hype_rest_state = {
    .hype_exercise_based_time = false,
    .hype_minutes = 1,
    .hype_seconds = 30,
    .hype_playlist_index = 0,
    .rest_exercise_based_time = true,
    .rest_minutes = 2,
    .rest_seconds = 30,
    .rest_playlist_index = 1,
    .selected_item = 0,
    .edit_mode = false
};

NavStack g_nav_stack;

static void notify_display_refresh() {
    if (g_display_queue != nullptr) {
        DisplayEvent evt;
        evt.type = DisplayEventType::DISPLAY_REFRESH_NEEDED;
        evt.payload = 0;
        xQueueSend(g_display_queue, &evt, 0);
    }
}

static void log_nav_state(const char* prefix) {
    const NavigationState& current = g_nav_stack.current();
    Serial.printf("[APP] %s: nav_stack_depth=%d, screen_id=%s, selection_index=%u\n",
                  prefix,
                  g_nav_stack.get_depth(),
                  screen_id_to_string(current.screen_id),
                  current.selection_index);
}

static void app_handle_input_event(const InputEvent& evt) {
    const NavigationState& current = g_nav_stack.current();
    uint8_t screen_id = current.screen_id;

    if (screen_id == SCREEN_HOME) {
        switch (evt.type) {
            case InputEventType::ENCODER_CW:
            case InputEventType::ENCODER_CCW: {
                NavigationState menu_state = { SCREEN_MAIN_MENU, 0, 0 };
                if (g_nav_stack.push(menu_state)) {
                    log_nav_state("Nav transition");
                    notify_display_refresh();
                }
                break;
            }
            case InputEventType::BUTTON_HYPE_PRESS:
                Serial.println("[APP] Home Hype action (Spotify Hype trigger - Phase 1 dummy)");
                break;
            case InputEventType::BUTTON_REST_PRESS:
                Serial.println("[APP] Home Rest action (Spotify Rest trigger - Phase 1 dummy)");
                break;
            case InputEventType::BUTTON_ENCODER_PRESS:
                Serial.println("[APP] Home Encoder press (context shortcut - Phase 1 no-op)");
                break;
            case InputEventType::BUTTON_PREV_PRESS:
            case InputEventType::BUTTON_PLAY_PAUSE_PRESS:
            case InputEventType::BUTTON_NEXT_PRESS:
                Serial.printf("[APP] Global music button pressed on Home: %s\n", input_event_to_string(evt.type));
                break;
            default:
                break;
        }
    } else {
        // Non-Home screens
        switch (evt.type) {
            case InputEventType::BUTTON_HYPE_PRESS: {
                // Hype acts as universal Back on non-Home screens
                if (g_nav_stack.pop()) {
                    log_nav_state("Nav transition");
                    notify_display_refresh();
                }
                break;
            }
            case InputEventType::ENCODER_CW: {
                if (screen_id == SCREEN_MAIN_MENU || screen_id == SCREEN_SETTINGS_MAIN || screen_id == SCREEN_SETTINGS_DISPLAY) {
                    uint8_t next_sel = (current.selection_index + 1) % 3;
                    g_nav_stack.set_current_selection(next_sel);
                    log_nav_state("Nav selection");
                    notify_display_refresh();
                } else if (screen_id == SCREEN_SETTINGS_BRIGHTNESS) {
                    // Decrement brightness (clamped at 0, no wrap)
                    if (current.selection_index > 0) {
                        uint8_t new_brightness = (current.selection_index >= 5) ? (current.selection_index - 5) : 0;
                        g_nav_stack.set_current_selection(new_brightness);
                        log_nav_state("Brightness adjust");
                        notify_display_refresh();
                    }
                } else if (screen_id == SCREEN_SETTINGS_HYPE_REST && !g_hype_rest_state.edit_mode) {
                    // CW = scroll DOWN (increment item index)
                    g_hype_rest_state.selected_item = (g_hype_rest_state.selected_item + 1) % 8;
                    Serial.printf("[APP] Hype&Rest selection: %u\n", g_hype_rest_state.selected_item);
                    notify_display_refresh();
                } else if (screen_id == SCREEN_SETTINGS_HYPE_REST && g_hype_rest_state.edit_mode) {
                    // CW = decrement value (reversed from navigation mode)
                    uint8_t item = g_hype_rest_state.selected_item;
                    if (item == 1 && g_hype_rest_state.hype_minutes > 0) g_hype_rest_state.hype_minutes--;
                    else if (item == 2 && g_hype_rest_state.hype_seconds >= 5) g_hype_rest_state.hype_seconds -= 5;
                    else if (item == 2 && g_hype_rest_state.hype_seconds > 0) g_hype_rest_state.hype_seconds = 0;
                    else if (item == 3) g_hype_rest_state.hype_playlist_index = (g_hype_rest_state.hype_playlist_index + 4) % 5;
                    else if (item == 5 && g_hype_rest_state.rest_minutes > 0) g_hype_rest_state.rest_minutes--;
                    else if (item == 6 && g_hype_rest_state.rest_seconds >= 5) g_hype_rest_state.rest_seconds -= 5;
                    else if (item == 6 && g_hype_rest_state.rest_seconds > 0) g_hype_rest_state.rest_seconds = 0;
                    else if (item == 7) g_hype_rest_state.rest_playlist_index = (g_hype_rest_state.rest_playlist_index + 4) % 5;
                    Serial.printf("[APP] Hype&Rest edit: adjusted value\n");
                    notify_display_refresh();
                }
                break;
            }
            case InputEventType::ENCODER_CCW: {
                if (screen_id == SCREEN_MAIN_MENU || screen_id == SCREEN_SETTINGS_MAIN || screen_id == SCREEN_SETTINGS_DISPLAY) {
                    uint8_t prev_sel = (current.selection_index + 2) % 3;
                    g_nav_stack.set_current_selection(prev_sel);
                    log_nav_state("Nav selection");
                    notify_display_refresh();
                } else if (screen_id == SCREEN_SETTINGS_BRIGHTNESS) {
                    // Increment brightness (clamped at 100, no wrap)
                    if (current.selection_index < 100) {
                        uint8_t new_brightness = (current.selection_index <= 95) ? (current.selection_index + 5) : 100;
                        g_nav_stack.set_current_selection(new_brightness);
                        log_nav_state("Brightness adjust");
                        notify_display_refresh();
                    }
                } else if (screen_id == SCREEN_SETTINGS_HYPE_REST && !g_hype_rest_state.edit_mode) {
                    // CCW = scroll UP (decrement item index)
                    g_hype_rest_state.selected_item = (g_hype_rest_state.selected_item + 7) % 8;
                    Serial.printf("[APP] Hype&Rest selection: %u\n", g_hype_rest_state.selected_item);
                    notify_display_refresh();
                } else if (screen_id == SCREEN_SETTINGS_HYPE_REST && g_hype_rest_state.edit_mode) {
                    // CCW = increment value (reversed from navigation mode)
                    uint8_t item = g_hype_rest_state.selected_item;
                    if (item == 1 && g_hype_rest_state.hype_minutes < 10) g_hype_rest_state.hype_minutes++;
                    else if (item == 2 && g_hype_rest_state.hype_seconds <= 54) g_hype_rest_state.hype_seconds += 5;
                    else if (item == 2 && g_hype_rest_state.hype_seconds < 59) g_hype_rest_state.hype_seconds = 59;
                    else if (item == 3) g_hype_rest_state.hype_playlist_index = (g_hype_rest_state.hype_playlist_index + 1) % 5;
                    else if (item == 5 && g_hype_rest_state.rest_minutes < 10) g_hype_rest_state.rest_minutes++;
                    else if (item == 6 && g_hype_rest_state.rest_seconds <= 54) g_hype_rest_state.rest_seconds += 5;
                    else if (item == 6 && g_hype_rest_state.rest_seconds < 59) g_hype_rest_state.rest_seconds = 59;
                    else if (item == 7) g_hype_rest_state.rest_playlist_index = (g_hype_rest_state.rest_playlist_index + 1) % 5;
                    Serial.printf("[APP] Hype&Rest edit: adjusted value\n");
                    notify_display_refresh();
                }
                break;
            }
            case InputEventType::BUTTON_ENCODER_PRESS: {
                if (screen_id == SCREEN_MAIN_MENU && current.selection_index == 2) {
                    // Main Menu → Settings selected: push Settings—Main
                    NavigationState settings_state = { SCREEN_SETTINGS_MAIN, 0, 0 };
                    if (g_nav_stack.push(settings_state)) {
                        log_nav_state("Nav transition");
                        notify_display_refresh();
                    }
                } else if (screen_id == SCREEN_SETTINGS_MAIN && current.selection_index == 0) {
                    // Settings—Main → Display selected: push Settings—Display
                    NavigationState settings_display_state = { SCREEN_SETTINGS_DISPLAY, 0, 0 };
                    if (g_nav_stack.push(settings_display_state)) {
                        log_nav_state("Nav transition");
                        notify_display_refresh();
                    }
                } else if (screen_id == SCREEN_SETTINGS_MAIN && current.selection_index == 1) {
                    // Settings—Main → Hype & Rest selected: push Settings—Hype&Rest
                    NavigationState settings_hype_rest_state = { SCREEN_SETTINGS_HYPE_REST, 0, 0 };
                    if (g_nav_stack.push(settings_hype_rest_state)) {
                        log_nav_state("Nav transition");
                        notify_display_refresh();
                    }
                } else if (screen_id == SCREEN_SETTINGS_DISPLAY && current.selection_index == 0) {
                    // Settings—Display → Brightness selected: push Settings—Brightness (starting at 50%)
                    NavigationState settings_brightness_state = { SCREEN_SETTINGS_BRIGHTNESS, 50, 0 };
                    if (g_nav_stack.push(settings_brightness_state)) {
                        log_nav_state("Nav transition");
                        notify_display_refresh();
                    }
                } else if (screen_id == SCREEN_SETTINGS_HYPE_REST && !g_hype_rest_state.edit_mode) {
                    // Settings—Hype&Rest BROWSE MODE: encoder press enters edit or toggles checkbox
                    uint8_t item = g_hype_rest_state.selected_item;
                    if (item == 0 || item == 4) {
                        // Checkbox: toggle immediately, no edit mode
                        if (item == 0) g_hype_rest_state.hype_exercise_based_time = !g_hype_rest_state.hype_exercise_based_time;
                        else g_hype_rest_state.rest_exercise_based_time = !g_hype_rest_state.rest_exercise_based_time;
                        Serial.printf("[APP] Toggled checkbox %u\n", item);
                    } else {
                        // Time field or playlist: enter edit mode
                        g_hype_rest_state.edit_mode = true;
                        Serial.printf("[APP] Entered edit mode for item %u\n", item);
                    }
                    notify_display_refresh();
                } else if (screen_id == SCREEN_SETTINGS_HYPE_REST && g_hype_rest_state.edit_mode) {
                    // Settings—Hype&Rest EDIT MODE: encoder press saves & exits edit mode
                    g_hype_rest_state.edit_mode = false;
                    Serial.printf("[APP] Exited edit mode, value saved\n");
                    notify_display_refresh();
                } else {
                    Serial.printf("[APP] Encoder press on screen_id=%s, selection_index=%u (no action)\n",
                                  screen_id_to_string(screen_id),
                                  current.selection_index);
                }
                break;
            }
            case InputEventType::BUTTON_REST_PRESS: {
                Serial.println("[APP] Rest button on non-Home (Info action placeholder - Phase 1 no-op)");
                break;
            }
            case InputEventType::BUTTON_PREV_PRESS:
            case InputEventType::BUTTON_PLAY_PAUSE_PRESS:
            case InputEventType::BUTTON_NEXT_PRESS:
                Serial.printf("[APP] Global music button pressed: %s\n", input_event_to_string(evt.type));
                break;
            default:
                break;
        }
    }
}

static void app_task_loop(void* pvParameters) {
    (void)pvParameters;
    Serial.println("[APP] App-logic task started on Core " + String(xPortGetCoreID()));

    log_nav_state("Boot nav stack state");

    InputEvent evt;
    for (;;) {
        if (g_input_queue != nullptr && xQueueReceive(g_input_queue, &evt, portMAX_DELAY) == pdTRUE) {
            app_handle_input_event(evt);
        }
    }
}

void app_task_init() {
    g_nav_stack.init();
    Serial.println("[APP] App-logic initialized with Home screen as default.");
}

void app_task_start(UBaseType_t priority, BaseType_t core) {
    xTaskCreatePinnedToCore(
        app_task_loop,
        "app_task",
        4096,
        nullptr,
        priority,
        nullptr,
        core
    );
}

const NavigationState& app_get_current_nav_state() {
    return g_nav_stack.current();
}
