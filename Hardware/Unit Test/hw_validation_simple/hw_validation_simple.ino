// ==========================================================================
// Minimal Hardware Validation Sketch
// ==========================================================================
// - Scans I2C bus on boot (PCF8574 @0x20, INA219 @0x40)
// - Encoder rotation -> adjusts backlight brightness (shown on screen)
// - Any expander button press -> shown on screen + Serial
// - Includes a raw red-screen flash right after tft.init() as a sanity
//   check, independent of backlight/text rendering — if you don't see red
//   flash briefly on boot, the display/SPI link itself isn't working.
// ==========================================================================

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// ------------------------- Pins -------------------------------------------

#define PIN_BACKLIGHT       3   // D2 / GPIO3
#define PIN_ENCODER_CLK     43  // D6 / GPIO43
#define PIN_ENCODER_DT      44  // D7 / GPIO44
#define PIN_I2C_SDA         5   // D4 / GPIO5
#define PIN_I2C_SCL         6   // D5 / GPIO6

#define PCF8574_I2C_ADDR    0x20
#define INA219_I2C_ADDR     0x40

#define EXPANDER_PIN_NEXT         0
#define EXPANDER_PIN_REST         1
#define EXPANDER_PIN_PREV         2
#define EXPANDER_PIN_HYPE         3
#define EXPANDER_PIN_PLAY_PAUSE   4
#define EXPANDER_PIN_ENCODER_SW   5

#define LEDC_CHANNEL_BACKLIGHT  0
#define LEDC_FREQ_HZ            5000
#define LEDC_RESOLUTION_BITS    8

// ------------------------- Globals ------------------------------------------

TFT_eSPI tft = TFT_eSPI();

static volatile long g_encoder_raw_pos = 0;
static volatile uint8_t g_last_clk = HIGH;
static long g_encoder_processed_pos = 0;

int g_brightness_pct = 50;
String g_last_button = "(none)";
float g_last_voltage = 0.0f;

uint8_t g_last_expander_sample = 0xFF;
uint8_t g_debounced_expander_state = 0xFF;

unsigned long g_last_ina219_read_ms = 0;
const unsigned long INA219_READ_INTERVAL_MS = 2000;

// ------------------------- Encoder ISR ---------------------------------------

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

// ------------------------- I2C scan -------------------------------------------

void run_i2c_scan() {
    Serial.println("[I2C] Scanning...");
    bool found_pcf8574 = false, found_ina219 = false;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[I2C]   Found 0x%02X\n", addr);
            if (addr == PCF8574_I2C_ADDR) found_pcf8574 = true;
            if (addr == INA219_I2C_ADDR) found_ina219 = true;
        }
    }
    Serial.printf("[I2C] PCF8574 (0x20): %s   INA219 (0x40): %s\n",
                  found_pcf8574 ? "OK" : "MISSING",
                  found_ina219 ? "OK" : "MISSING");
}

// ------------------------- PCF8574 -------------------------------------------

uint8_t read_pcf8574() {
    Wire.requestFrom((uint8_t)PCF8574_I2C_ADDR, (uint8_t)1);
    if (Wire.available()) return Wire.read();
    return 0xFF;
}

// ------------------------- INA219 (raw bus voltage) --------------------------

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

float ina219_read_bus_voltage() {
    uint16_t raw = ina219_read16(INA219_REG_BUSVOLTAGE);
    raw >>= 3;
    return raw * 0.004f;
}

// ------------------------- Display ---------------------------------------------

void update_display() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);

    tft.setCursor(10, 30);
    tft.printf("Brightness: %d%%", g_brightness_pct);

    tft.setCursor(10, 80);
    tft.printf("Last button:");
    tft.setCursor(10, 110);
    tft.print(g_last_button);

    tft.setCursor(10, 160);
    tft.printf("Bus: %.2fV", g_last_voltage);
}

// ------------------------- Setup ------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Minimal Validation Sketch ===");

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(100000);
    run_i2c_scan();

    Wire.beginTransmission(PCF8574_I2C_ADDR);
    Wire.write(0xFF);
    Wire.endTransmission();

    ina219_write16(INA219_REG_CONFIG, 0x399F);

    pinMode(PIN_ENCODER_CLK, INPUT_PULLUP);
    pinMode(PIN_ENCODER_DT, INPUT_PULLUP);
    g_last_clk = digitalRead(PIN_ENCODER_CLK);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_CLK), handle_encoder_isr, CHANGE);
    ledcAttach(PIN_BACKLIGHT, LEDC_FREQ_HZ, LEDC_RESOLUTION_BITS);

    // ledcSetup(LEDC_CHANNEL_BACKLIGHT, LEDC_FREQ_HZ, LEDC_RESOLUTION_BITS);
    // ledcAttachPin(PIN_BACKLIGHT, LEDC_CHANNEL_BACKLIGHT);
    ledcWrite(LEDC_CHANNEL_BACKLIGHT, map(g_brightness_pct, 0, 100, 0, 255));

    tft.init();
    tft.setRotation(1);

    // Raw sanity flash — proves SPI/display link works before anything else runs
    tft.fillScreen(TFT_RED);
    delay(500);

    update_display();
    Serial.println("=== Setup complete ===\n");
}

// ------------------------- Loop --------------------------------------------------

void loop() {
    // Encoder -> brightness
    long current_raw;
    noInterrupts();
    current_raw = g_encoder_raw_pos;
    interrupts();

    long diff = current_raw - g_encoder_processed_pos;
    bool changed = false;

    while (diff >= 2) {
        diff -= 2;
        g_encoder_processed_pos += 2;
        g_brightness_pct = constrain(g_brightness_pct - 5, 0, 100);
        changed = true;
    }
    while (diff <= -2) {
        diff += 2;
        g_encoder_processed_pos -= 2;
        g_brightness_pct = constrain(g_brightness_pct + 5, 0, 100);
        changed = true;
    }

    if (changed) {
        ledcWrite(LEDC_CHANNEL_BACKLIGHT, map(g_brightness_pct, 0, 100, 0, 255));
        Serial.printf("[BRIGHTNESS] %d%%\n", g_brightness_pct);
        update_display();
    }

    // Expander buttons -> last-pressed label
    uint8_t sample = read_pcf8574();
    if (sample == g_last_expander_sample && sample != g_debounced_expander_state) {
        for (int pin = 0; pin <= 5; pin++) {
            bool was_high = (g_debounced_expander_state & (1 << pin)) != 0;
            bool is_high  = (sample & (1 << pin)) != 0;
            if (was_high && !is_high) { // falling edge = press
                switch (pin) {
                    case EXPANDER_PIN_NEXT:       g_last_button = "NEXT"; break;
                    case EXPANDER_PIN_REST:       g_last_button = "REST"; break;
                    case EXPANDER_PIN_PREV:       g_last_button = "PREV"; break;
                    case EXPANDER_PIN_HYPE:       g_last_button = "HYPE"; break;
                    case EXPANDER_PIN_PLAY_PAUSE: g_last_button = "PLAY/PAUSE"; break;
                    case EXPANDER_PIN_ENCODER_SW: g_last_button = "ENCODER-SW"; break;
                }
                Serial.printf("[BTN] %s pressed\n", g_last_button.c_str());
                update_display();
            }
        }
        g_debounced_expander_state = sample;
    }
    g_last_expander_sample = sample;

    // INA219 periodic read
    unsigned long now = millis();
    if (now - g_last_ina219_read_ms >= INA219_READ_INTERVAL_MS) {
        g_last_ina219_read_ms = now;
        g_last_voltage = ina219_read_bus_voltage();
        Serial.printf("[BATT] %.3fV\n", g_last_voltage);
        update_display();
    }

    delay(20);
}
