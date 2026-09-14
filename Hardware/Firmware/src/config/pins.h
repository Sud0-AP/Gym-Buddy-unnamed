#pragma once

#include <Arduino.h>

// ==========================================
// Seeed Studio XIAO ESP32S3 Pin Definitions
// ==========================================

// Display SPI (ST7789 240x320)
#define PIN_TFT_MOSI   9   // D10 / GPIO9
#define PIN_TFT_MISO   8   // D9  / GPIO8
#define PIN_TFT_SCLK   7   // D8  / GPIO7
#define PIN_TFT_CS     1   // D0  / GPIO1
#define PIN_TFT_DC     2   // D1  / GPIO2
#define PIN_TFT_RST    4   // D3  / GPIO4
// Backlight is wired to 3V3 (always on in Phase 1)

// I2C Bus (PCF8574T I/O Expander)
#define PIN_I2C_SDA    5   // D4  / GPIO5
#define PIN_I2C_SCL    6   // D5  / GPIO6
#define PCF8574_I2C_ADDR 0x20

// PCF8574 Expander Pin Assignments (Active LOW)
#define EXPANDER_PIN_NEXT       0 // P0
#define EXPANDER_PIN_REST       1 // P1
#define EXPANDER_PIN_PREV       2 // P2
#define EXPANDER_PIN_HYPE       3 // P3
#define EXPANDER_PIN_PLAY_PAUSE 4 // P4
#define EXPANDER_PIN_ENCODER_SW 5 // P5 (optional expander routing)

// Rotary Encoder (EC11 / M274)
#define PIN_ENCODER_CLK 43  // D6 / GPIO43 (Phase A, interrupt pin)
#define PIN_ENCODER_DT  44  // D7 / GPIO44 (Phase B, direction decode)
#define PIN_ENCODER_SW  3   // D2 / GPIO3 (Push button native GPIO, active LOW)
