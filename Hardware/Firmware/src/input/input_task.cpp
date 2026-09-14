#include "input_task.h"
#include "../config/pins.h"
#include <Arduino.h>
#include <Wire.h>

QueueHandle_t g_input_queue = nullptr;

// Encoder ISR variables (single-CLK CHANGE interrupt method)
static volatile long g_encoder_raw_pos = 0;
static volatile uint8_t g_last_clk = HIGH;

static void IRAM_ATTR handle_encoder_isr() {
    uint8_t clk_state = digitalRead(PIN_ENCODER_CLK);
    if (clk_state != g_last_clk) {
        if (digitalRead(PIN_ENCODER_DT) != clk_state) {
            g_encoder_raw_pos++;
        } else {
            g_encoder_raw_pos--;
        }
        g_last_clk = clk_state;
    }
}

static void post_input_event(InputEventType type) {
    if (type == InputEventType::NONE || g_input_queue == nullptr) {
        return;
    }

    InputEvent evt;
    evt.type = type;
    evt.timestamp = millis();

    Serial.printf("[INPUT] %s\n", input_event_to_string(type));

    if (xQueueSend(g_input_queue, &evt, 0) != pdTRUE) {
        Serial.printf("[INPUT] Queue full, dropped event: %s\n", input_event_to_string(type));
    }
}

static uint8_t read_pcf8574() {
    Wire.requestFrom((uint8_t)PCF8574_I2C_ADDR, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0xFF; // All high = no buttons pressed
}

static void input_task_loop(void* pvParameters) {
    Serial.println("[INPUT] Input task started on Core " + String(xPortGetCoreID()));

    // Detent accumulator state
    long processed_raw_pos = 0;

    // Expander button state & debounce
    uint8_t last_expander_sample = 0xFF;
    uint8_t debounced_expander_state = 0xFF;

    // Encoder push button state & debounce
    int last_encoder_sw_sample = HIGH;
    int debounced_encoder_sw = HIGH;

    const TickType_t poll_period = pdMS_TO_TICKS(20); // 50 Hz polling

    for (;;) {
        // ----------------------------------------------------
        // 1. Process Encoder Rotation — 1 physical detent = 1 event
        //    Direction is SWAPPED (physical CCW = semantic CW) per hardware.
        // ----------------------------------------------------
        long current_raw;
        noInterrupts();
        current_raw = g_encoder_raw_pos;
        interrupts();

        long diff = current_raw - processed_raw_pos;

        // 2 raw ISR transitions = 1 physical detent
        while (diff >= 2) {
            diff -= 2;
            processed_raw_pos += 2;
            // Hardware direction swapped: raw+ → semantic CCW
            post_input_event(InputEventType::ENCODER_CCW);
        }

        while (diff <= -2) {
            diff += 2;
            processed_raw_pos -= 2;
            // Hardware direction swapped: raw- → semantic CW
            post_input_event(InputEventType::ENCODER_CW);
        }

        // ----------------------------------------------------
        // 2. Poll & Debounce PCF8574 Expander (5 Buttons + SW)
        // ----------------------------------------------------
        uint8_t current_expander_sample = read_pcf8574();
        if (current_expander_sample == last_expander_sample) {
            // Two consecutive identical reads confirm state change
            if (current_expander_sample != debounced_expander_state) {
                for (int pin = 0; pin <= 5; pin++) {
                    bool was_high = (debounced_expander_state & (1 << pin)) != 0;
                    bool is_high = (current_expander_sample & (1 << pin)) != 0;

                    // Falling edge = button press (active LOW)
                    if (was_high && !is_high) {
                        switch (pin) {
                            case EXPANDER_PIN_NEXT:
                                post_input_event(InputEventType::BUTTON_NEXT_PRESS);
                                break;
                            case EXPANDER_PIN_REST:
                                post_input_event(InputEventType::BUTTON_REST_PRESS);
                                break;
                            case EXPANDER_PIN_PREV:
                                post_input_event(InputEventType::BUTTON_PREV_PRESS);
                                break;
                            case EXPANDER_PIN_HYPE:
                                post_input_event(InputEventType::BUTTON_HYPE_PRESS);
                                break;
                            case EXPANDER_PIN_PLAY_PAUSE:
                                post_input_event(InputEventType::BUTTON_PLAY_PAUSE_PRESS);
                                break;
                            case EXPANDER_PIN_ENCODER_SW:
                                post_input_event(InputEventType::BUTTON_ENCODER_PRESS);
                                break;
                            default:
                                break;
                        }
                    }
                }
                debounced_expander_state = current_expander_sample;
            }
        }
        last_expander_sample = current_expander_sample;

        // ----------------------------------------------------
        // 3. Poll & Debounce Encoder Push Button (SW)
        // ----------------------------------------------------
        int current_sw_sample = digitalRead(PIN_ENCODER_SW);
        if (current_sw_sample == last_encoder_sw_sample) {
            if (current_sw_sample != debounced_encoder_sw) {
                debounced_encoder_sw = current_sw_sample;
                if (debounced_encoder_sw == LOW) {
                    // Falling edge = button press
                    post_input_event(InputEventType::BUTTON_ENCODER_PRESS);
                }
            }
        }
        last_encoder_sw_sample = current_sw_sample;

        vTaskDelay(poll_period);
    }
}

void input_task_init() {
    // Create 8-entry input queue
    if (g_input_queue == nullptr) {
        g_input_queue = xQueueCreate(8, sizeof(InputEvent));
    }

    // Configure Encoder pins
    pinMode(PIN_ENCODER_CLK, INPUT_PULLUP);
    pinMode(PIN_ENCODER_DT, INPUT_PULLUP);
    pinMode(PIN_ENCODER_SW, INPUT_PULLUP);

    g_last_clk = digitalRead(PIN_ENCODER_CLK);

    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_CLK), handle_encoder_isr, CHANGE);

    // Configure I2C for PCF8574
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(100000);

    // Set expander pins HIGH (quasi-bidirectional pull-ups)
    Wire.beginTransmission((uint8_t)PCF8574_I2C_ADDR);
    Wire.write(0xFF);
    Wire.endTransmission();

    Serial.println("[INPUT] Input hardware initialized.");
}

void input_task_start(UBaseType_t priority, BaseType_t core) {
    xTaskCreatePinnedToCore(
        input_task_loop,
        "input_task",
        4096,
        nullptr,
        priority,
        nullptr,
        core
    );
}
