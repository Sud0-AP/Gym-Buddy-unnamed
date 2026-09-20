// ==========================================================================
// Gym Companion Device — Hardware Validation Sketch
// ==========================================================================
// PURPOSE: Bring-up / soldering-check only. Simple polling loop, no FreeRTOS.
// Not final firmware architecture — see input_task.cpp for the real
// interrupt+queue based input handling this will eventually be replaced by.
//
// Board: Seeed Studio XIAO ESP32S3, Arduino-ESP32 core.
//
// PIN MAP (as resolved for this bring-up):
//   Display (ST7789 via TFT_eSPI, pins live in TFT_eSPI's User_Setup.h):
//     CS=D0/GPIO1, DC=D1/GPIO2, RST=D3/GPIO4, SCK=D8/GPIO7, MOSI=D10/GPIO9
//     MISO: NOT wired to the display (write-only) — physical pin repurposed
//     below as native Lock-sense input.
//   Backlight:      D2 / GPIO3  — PWM via LEDC (freed up by moving the
//                                 encoder push-button fully onto the
//                                 expander, see below)
//   Lock-sense:     D9 / GPIO8  — native digital input (repurposed unused
//                                 TFT MISO trace). HIGH = Lock, LOW = On.
//   Encoder A/B:    D6/GPIO43, D7/GPIO44 — rotation, CHANGE-interrupt decode
//   I2C bus:        SDA=D4/GPIO5, SCL=D5/GPIO6
//     PCF8574T @ 0x20: P0=Next, P1=Rest, P2=Prev, P3=Hype, P4=Play/Pause,
//                       P5=Encoder-select, P6=Battery-active indicator,
//                       P7=CHRG (charging indicator)   (all assumed active LOW)
//     INA219    @ 0x40: bus voltage / current sensing on battery discharge path
//
// FLAGGED ASSUMPTIONS (confirm against your actual wiring before trusting
// the output):
//   1. Lock-sense (GPIO8) confirmed: switch drives 3.3V (HIGH) when active/
//      Lock, read as a plain INPUT (no internal pull). If the other switch
//      position floats instead of driving LOW, change pinMode() below to
//      INPUT_PULLDOWN.
//   2. P6 "battery active indicator" polarity/meaning is not fully confirmed
//      (not standard TP4056 terminology) — the sketch prints P6/P7's raw
//      HIGH/LOW state plus a best-guess interpretation so you can watch it
//      change live (plug/unplug USB) and confirm what it actually means.
//   3. TFT_eSPI's User_Setup.h must NOT define TFT_MISO on GPIO8 anymore —
//      that pin is now Lock-sense. Not touched by this sketch, just check it.
// ==========================================================================

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// ------------------------- Pin definitions -------------------------------

// Backlight (freed-up D2/GPIO3, now PWM via LEDC)
#define PIN_BACKLIGHT       3   // D2 / GPIO3

// Lock-sense (repurposed unused TFT MISO trace)
#define PIN_LOCK_SENSE      8   // D9 / GPIO8

// Rotary encoder (unchanged from existing input_task)
#define PIN_ENCODER_CLK     43  // D6 / GPIO43
#define PIN_ENCODER_DT      44  // D7 / GPIO44

// I2C bus
#define PIN_I2C_SDA         5   // D4 / GPIO5
#define PIN_I2C_SCL         6   // D5 / GPIO6

// I2C addresses
#define PCF8574_I2C_ADDR    0x20
#define INA219_I2C_ADDR     0x40

// PCF8574 expander pin assignments (active LOW)
#define EXPANDER_PIN_NEXT         0
#define EXPANDER_PIN_REST         1
#define EXPANDER_PIN_PREV         2
#define EXPANDER_PIN_HYPE         3
#define EXPANDER_PIN_PLAY_PAUSE   4
#define EXPANDER_PIN_ENCODER_SW   5
#define EXPANDER_PIN_BATT_ACTIVE  6   // "battery active" indicator — meaning/polarity unconfirmed, see notes at top
#define EXPANDER_PIN_CHRG         7   // charging indicator

// LEDC (backlight PWM) config
#define LEDC_CHANNEL_BACKLIGHT  0
#define LEDC_FREQ_HZ            5000
#define LEDC_RESOLUTION_BITS    8      // 0-255 duty range

// ------------------------- Globals ----------------------------------------

TFT_eSPI tft = TFT_eSPI();

// Encoder ISR state (single-CLK CHANGE-interrupt method, matches input_task.cpp)
static volatile long g_encoder_raw_pos = 0;
static volatile uint8_t g_last_clk = HIGH;
static long g_encoder_processed_pos = 0;

int g_brightness_pct = 50;   // start at 50%
int g_battery_pct = -1;      // -1 = not yet read

// Expander debounce state
uint8_t g_last_expander_sample = 0xFF;
uint8_t g_debounced_expander_state = 0xFF;

// Lock-sense debounce state
int g_last_lock_sample = LOW;
int g_debounced_lock_state = LOW;

unsigned long g_last_ina219_read_ms = 0;
const unsigned long INA219_READ_INTERVAL_MS = 2000;

// ------------------------- Encoder ISR -------------------------------------

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

// ------------------------- I2C scanner --------------------------------------

void run_i2c_scan() {
    Serial.println("[I2C] Scanning bus...");
    int found = 0;
    bool found_pcf8574 = false;
    bool found_ina219 = false;

    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t err = Wire.endTransmission();
        if (err == 0) {
            Serial.printf("[I2C]   Found device at 0x%02X\n", addr);
            found++;
            if (addr == PCF8574_I2C_ADDR) found_pcf8574 = true;
            if (addr == INA219_I2C_ADDR) found_ina219 = true;
        }
    }

    Serial.printf("[I2C] Scan complete. %d device(s) found.\n", found);
    Serial.printf("[I2C]   PCF8574 (0x20): %s\n", found_pcf8574 ? "OK" : "MISSING");
    Serial.printf("[I2C]   INA219  (0x40): %s\n", found_ina219 ? "OK" : "MISSING");

    if (!found_pcf8574 || !found_ina219) {
        Serial.println("[I2C]   *** One or more expected devices not found — check wiring before continuing. ***");
    }
}

// ------------------------- PCF8574 helpers ----------------------------------

uint8_t read_pcf8574() {
    Wire.requestFrom((uint8_t)PCF8574_I2C_ADDR, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0xFF; // all high = no buttons pressed / read failure
}

void print_charge_state(bool chrg_low, bool batt_active_low) {
    // Raw states always printed first — P6's exact meaning/polarity isn't
    // confirmed yet, so watch these raw values while plugging/unplugging USB
    // to work out what "battery active" actually indicates on this board.
    Serial.printf("[BATT] CHRG=%s  BATT_ACTIVE=%s",
                  chrg_low ? "LOW(asserted)" : "HIGH",
                  batt_active_low ? "LOW(asserted)" : "HIGH");

    if (chrg_low) {
        Serial.println("  -> best guess: Charging...");
    } else if (batt_active_low) {
        Serial.println("  -> best guess: battery active (not charging)");
    } else {
        Serial.println("  -> best guess: idle / no USB");
    }
}

// ------------------------- INA219 raw register access -----------------------

#define INA219_REG_CONFIG      0x00
#define INA219_REG_BUSVOLTAGE  0x02

void ina219_write16(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(INA219_I2C_ADDR);
    Wire.write(reg);
    Wire.write((uint8_t)(value >> 8));
    Wire.write((uint8_t)(value & 0xFF));
    Wire.endTransmission();
}

uint16_t ina219_read16(uint8_t reg) {
    Wire.beginTransmission(INA219_I2C_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)INA219_I2C_ADDR, (uint8_t)2);
    uint16_t value = 0;
    if (Wire.available() >= 2) {
        value = (Wire.read() << 8);
        value |= Wire.read();
    }
    return value;
}

void ina219_init() {
    // 32V bus range, 320mV shunt gain, 12-bit ADC, continuous shunt+bus sampling.
    // Standard INA219 default-ish config, sufficient for a bus-voltage-only test.
    ina219_write16(INA219_REG_CONFIG, 0x399F);
}

float ina219_read_bus_voltage() {
    uint16_t raw = ina219_read16(INA219_REG_BUSVOLTAGE);
    raw >>= 3;              // drop CNVR/OVF flag bits (low 3 bits)
    return raw * 0.004f;    // 4 mV per LSB
}

int bucket_battery_pct(float voltage) {
    if (voltage >= 4.10f) return 100;
    if (voltage >= 3.85f) return 75;
    if (voltage >= 3.70f) return 50;
    if (voltage >= 3.50f) return 25;
    return 0;
}

// ------------------------- Backlight ----------------------------------------

void set_backlight_pct(int pct) {
    pct = constrain(pct, 0, 100);
    int duty = map(pct, 0, 100, 0, 255);
    ledcWrite(LEDC_CHANNEL_BACKLIGHT, duty);
}

// ------------------------- Display -------------------------------------------

void update_display() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);

    tft.setCursor(10, 40);
    tft.printf("Brightness: %d%%", g_brightness_pct);

    tft.setCursor(10, 90);
    if (g_battery_pct < 0) {
        tft.print("Battery: ...");
    } else {
        tft.printf("Battery: %d%%", g_battery_pct);
    }
}

// ------------------------- Setup ----------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Hardware Validation Sketch ===");

    // I2C
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(100000);
    run_i2c_scan();

    // PCF8574: set all pins HIGH (quasi-bidirectional pull-ups, inputs)
    Wire.beginTransmission(PCF8574_I2C_ADDR);
    Wire.write(0xFF);
    Wire.endTransmission();

    // INA219
    ina219_init();

    // Lock-sense (native GPIO, plain input — see flagged assumption at top of file)
    pinMode(PIN_LOCK_SENSE, INPUT);
    g_last_lock_sample = digitalRead(PIN_LOCK_SENSE);
    g_debounced_lock_state = g_last_lock_sample;

    // Encoder rotation
    pinMode(PIN_ENCODER_CLK, INPUT_PULLUP);
    pinMode(PIN_ENCODER_DT, INPUT_PULLUP);
    g_last_clk = digitalRead(PIN_ENCODER_CLK);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_CLK), handle_encoder_isr, CHANGE);

    // Backlight PWM (classic LEDC API — if you're on Arduino-ESP32 core v3.x,
    // replace these two lines with: 
    ledcAttach(PIN_BACKLIGHT, LEDC_FREQ_HZ, LEDC_RESOLUTION_BITS);
    // ledcSetup(LEDC_CHANNEL_BACKLIGHT, LEDC_FREQ_HZ, LEDC_RESOLUTION_BITS);
    // ledcAttachPin(PIN_BACKLIGHT, LEDC_CHANNEL_BACKLIGHT);
    set_backlight_pct(g_brightness_pct);

    // Display
    tft.init();
    tft.setRotation(1);
    update_display();

    Serial.println("=== Setup complete. Watching inputs... ===\n");
}

// ------------------------- Loop -----------------------------------------------

void loop() {
    // ---- 1. Encoder rotation -> brightness ----
    long current_raw;
    noInterrupts();
    current_raw = g_encoder_raw_pos;
    interrupts();

    long diff = current_raw - g_encoder_processed_pos;
    bool brightness_changed = false;

    while (diff >= 2) {
        diff -= 2;
        g_encoder_processed_pos += 2;
        Serial.println("[ENCODER] CCW");
        g_brightness_pct = constrain(g_brightness_pct - 5, 0, 100);
        brightness_changed = true;
    }
    while (diff <= -2) {
        diff += 2;
        g_encoder_processed_pos -= 2;
        Serial.println("[ENCODER] CW");
        g_brightness_pct = constrain(g_brightness_pct + 5, 0, 100);
        brightness_changed = true;
    }

    if (brightness_changed) {
        set_backlight_pct(g_brightness_pct);
        Serial.printf("[BACKLIGHT] %d%%\n", g_brightness_pct);
        update_display();
    }

    // ---- 2. PCF8574 poll + debounce (buttons, encoder-select, CHRG/STDBY) ----
    uint8_t current_expander_sample = read_pcf8574();
    if (current_expander_sample == g_last_expander_sample) {
        if (current_expander_sample != g_debounced_expander_state) {
            for (int pin = 0; pin <= 7; pin++) {
                bool was_high = (g_debounced_expander_state & (1 << pin)) != 0;
                bool is_high  = (current_expander_sample & (1 << pin)) != 0;

                if (was_high == is_high) continue; // no change on this bit

                bool pressed = (!is_high); // active LOW

                switch (pin) {
                    case EXPANDER_PIN_NEXT:
                        Serial.println(pressed ? "[BTN] NEXT pressed" : "[BTN] NEXT released");
                        break;
                    case EXPANDER_PIN_REST:
                        Serial.println(pressed ? "[BTN] REST pressed" : "[BTN] REST released");
                        break;
                    case EXPANDER_PIN_PREV:
                        Serial.println(pressed ? "[BTN] PREV pressed" : "[BTN] PREV released");
                        break;
                    case EXPANDER_PIN_HYPE:
                        Serial.println(pressed ? "[BTN] HYPE pressed" : "[BTN] HYPE released");
                        break;
                    case EXPANDER_PIN_PLAY_PAUSE:
                        Serial.println(pressed ? "[BTN] PLAY/PAUSE pressed" : "[BTN] PLAY/PAUSE released");
                        break;
                    case EXPANDER_PIN_ENCODER_SW:
                        Serial.println(pressed ? "[BTN] ENCODER-SELECT pressed" : "[BTN] ENCODER-SELECT released");
                        break;
                    case EXPANDER_PIN_CHRG:
                    case EXPANDER_PIN_BATT_ACTIVE: {
                        bool chrg_low        = (current_expander_sample & (1 << EXPANDER_PIN_CHRG)) == 0;
                        bool batt_active_low = (current_expander_sample & (1 << EXPANDER_PIN_BATT_ACTIVE)) == 0;
                        print_charge_state(chrg_low, batt_active_low);
                        break;
                    }
                    default:
                        break;
                }
            }
            g_debounced_expander_state = current_expander_sample;
        }
    }
    g_last_expander_sample = current_expander_sample;

    // ---- 3. Lock-sense poll + debounce ----
    int current_lock_sample = digitalRead(PIN_LOCK_SENSE);
    if (current_lock_sample == g_last_lock_sample) {
        if (current_lock_sample != g_debounced_lock_state) {
            g_debounced_lock_state = current_lock_sample;
            Serial.println(g_debounced_lock_state == HIGH ? "[SWITCH] Switch: LOCK" : "[SWITCH] Switch: ON");
        }
    }
    g_last_lock_sample = current_lock_sample;

    // ---- 4. INA219 periodic read ----
    unsigned long now = millis();
    if (now - g_last_ina219_read_ms >= INA219_READ_INTERVAL_MS) {
        g_last_ina219_read_ms = now;
        float voltage = ina219_read_bus_voltage();
        int new_battery_pct = bucket_battery_pct(voltage);
        Serial.printf("[BATT] Bus voltage: %.3f V -> %d%%\n", voltage, new_battery_pct);
        if (new_battery_pct != g_battery_pct) {
            g_battery_pct = new_battery_pct;
            update_display();
        }
    }

    delay(20); // ~50 Hz poll, matches existing input_task style
}
