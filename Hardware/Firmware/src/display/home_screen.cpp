#include "home_screen.h"
#include "screens.h"
#include "../app/dummy_state.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstdio>

extern TFT_eSPI tft;

// Forward declare constants from Lopaka exports (defined elsewhere)
extern const unsigned char PROGMEM image_hype_icon_bits[];
extern const unsigned char PROGMEM image_rest_icon_bits[];
extern const unsigned char PROGMEM image_topbar_battery_case_bits[];
extern const uint16_t PROGMEM image_topbar_homescreen_margin_pixels[];
extern const unsigned char PROGMEM image_topbar_network_connected_bits[];
extern const unsigned char PROGMEM image_music_icon_bits[];
extern const unsigned char PROGMEM image_artist_icon_bits[];

// Color constants
#define COLOR_DARK_BG       0x0801  // Near-black background
#define COLOR_ACCENT        0xE33F  // Magenta/purple accent
#define COLOR_SECONDARY_GR  0x6870  // Secondary gray
#define COLOR_ACTIVE_HYPE   0xD15E  // Active Hype/Rest logo color (matches #D15EE)
#define COLOR_WHITE         0xFFFF  // White text
#define COLOR_INACTIVE_LOGO 0x6870  // Inactive logo color (gray)

// Helper function: Draw checkerboard pattern for album art
static void draw_checkerboard(int x, int y, int w, int h, int square_size) {
    for (int i = 0; i < w; i += square_size) {
        for (int j = 0; j < h; j += square_size) {
            int col = (i / square_size + j / square_size) % 2;
            if (col == 0) {
                tft.fillRect(x + i, y + j, square_size, square_size, COLOR_ACCENT);
            }
            // Leave unfilled squares as background
        }
    }
}

// Helper function: Draw time formatted as MM:SS
static void draw_time(int x, int y, uint32_t ms) {
    uint32_t total_secs = ms / 1000;
    uint32_t mins = total_secs / 60;
    uint32_t secs = total_secs % 60;
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%u:%02u", (unsigned)mins, (unsigned)secs);
    tft.drawString(buffer, x, y);
}

void draw_home_screen_combined() {
    // Background
    tft.fillScreen(COLOR_DARK_BG);

    // === MUSIC BLOCK (LEFT SIDE) ===
    // Album art placeholder - checkerboard pattern
    tft.drawRect(15, 30, 130, 130, COLOR_SECONDARY_GR);
    draw_checkerboard(17, 32, 126, 126, 14);

    // Music icon
    tft.setTextColor(COLOR_SECONDARY_GR);
    tft.setTextSize(1);
    tft.drawBitmap(26, 188, image_music_icon_bits, 17, 16, COLOR_SECONDARY_GR);

    // Track times
    tft.drawString("0:00", 18, 168);
    tft.drawString("4:20", 119, 168);

    // Progress bar outline and fill
    tft.drawRect(44, 167, 72, 8, COLOR_SECONDARY_GR);
    // Calculate progress bar fill width based on dummy state
    int progress_fill = (g_dummy_music.progressMs * 72) / g_dummy_music.durationMs;
    tft.fillRect(46, 169, progress_fill, 4, COLOR_ACCENT);

    // Track title
    tft.setTextColor(COLOR_WHITE);
    tft.drawString(g_dummy_music.trackTitle, 54, 193);

    // Artist icon and name
    tft.setTextColor(COLOR_SECONDARY_GR);
    tft.drawBitmap(28, 211, image_artist_icon_bits, 13, 16, COLOR_SECONDARY_GR);
    tft.drawString(g_dummy_music.artistName, 54, 216);

    // === WORKOUT BLOCK (RIGHT SIDE) ===
    tft.setTextColor(COLOR_ACCENT);
    tft.drawString("Current", 211, 38);
    tft.setTextSize(1);
    tft.drawString("exercise:", 225, 57);

    // Exercise info box
    tft.drawRect(149, 75, 71, 60, COLOR_SECONDARY_GR);
    tft.setTextColor(COLOR_SECONDARY_GR);
    tft.drawString(g_dummy_workout.currentExercise, 175, 79);
    tft.drawString(g_dummy_workout.muscleGroup, 188, 94);
    tft.drawString(g_dummy_workout.targetReps, 242, 94);

    // Sets list
    tft.setTextColor(COLOR_WHITE);
    for (int i = 0; i < 5; i++) {
        int y_pos = 115 + (i * 13);
        if (i == g_dummy_workout.currentSetIndex) {
            // Highlight current set
            tft.setTextColor(COLOR_ACCENT);
            tft.drawRect(170, y_pos - 2, 124, 13, COLOR_SECONDARY_GR);
        } else {
            tft.setTextColor(COLOR_WHITE);
        }
        tft.drawString(g_dummy_workout.sets[i].label, 180, y_pos);
    }

    // === MODE INDICATOR (HYPE/REST) ===
    // Two boxes at bottom for Hype/Rest state
    tft.drawRect(149, 195, 71, 28, COLOR_SECONDARY_GR);
    tft.drawRect(230, 195, 71, 28, COLOR_SECONDARY_GR);

    // Hype box
    tft.setTextSize(2);
    tft.setTextColor(g_dummy_music.hypeActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);
    tft.drawString("HYPE", 168, 202);
    tft.drawBitmap(151, 201, image_hype_icon_bits, 16, 16,
                   g_dummy_music.hypeActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);

    // Rest box
    tft.setTextSize(2);
    tft.setTextColor(g_dummy_music.restActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);
    tft.drawString("REST", 250, 202);
    tft.setTextSize(1);
    char timer_str[8];
    snprintf(timer_str, sizeof(timer_str), "%u:%02u",
             g_dummy_music.hypeTimerSeconds / 60,
             g_dummy_music.hypeTimerSeconds % 60);
    tft.drawString(timer_str, 250, 202);

    // === TOP BAR ===
    tft.pushImage(0, 0, 321, 21, image_topbar_homescreen_margin_pixels);

    // Battery indicator (5-step)
    uint8_t battery_step = (g_dummy_settings.batteryPct / 20); // 0-5
    if (battery_step > 0) {
        tft.fillRect(296, 6, 12, 9, COLOR_ACCENT);
    }
    tft.drawBitmap(286, 3, image_topbar_battery_case_bits, 24, 16, COLOR_SECONDARY_GR);

    // Time, date, workout
    tft.setTextColor(COLOR_ACCENT);
    tft.setTextSize(1);
    tft.drawString("12:23 PM", 15, 6);
    tft.drawString("Monday: 14th May - Push A", 85, 6);

    // WiFi icon
    tft.drawBitmap(257, 3, image_topbar_network_connected_bits, 15, 14, COLOR_ACCENT);
}

void draw_home_screen_music_only() {
    // Background
    tft.fillScreen(COLOR_DARK_BG);

    // === LARGE MUSIC BLOCK ===
    tft.drawRect(30, 35, 130, 130, COLOR_SECONDARY_GR);
    draw_checkerboard(32, 37, 126, 126, 14);

    tft.setTextColor(COLOR_SECONDARY_GR);
    tft.setTextSize(1);
    tft.drawBitmap(26, 188, image_music_icon_bits, 17, 16, COLOR_SECONDARY_GR);
    tft.drawString("0:00", 40, 181);
    tft.drawString("4:20", 257, 181);

    tft.drawRect(70, 180, 180, 8, COLOR_SECONDARY_GR);
    int progress_fill = (g_dummy_music.progressMs * 180) / g_dummy_music.durationMs;
    tft.fillRect(72, 182, progress_fill, 4, COLOR_ACCENT);

    tft.setTextColor(COLOR_WHITE);
    tft.drawString(g_dummy_music.trackTitle, 211, 45);

    tft.setTextColor(COLOR_SECONDARY_GR);
    tft.drawBitmap(184, 66, image_artist_icon_bits, 13, 16, COLOR_SECONDARY_GR);
    tft.setTextColor(COLOR_WHITE);
    tft.drawString(g_dummy_music.artistName, 211, 71);

    // Mode boxes
    tft.drawRect(85, 200, 71, 28, COLOR_SECONDARY_GR);
    tft.drawRect(179, 200, 71, 28, COLOR_SECONDARY_GR);

    tft.setTextSize(2);
    tft.setTextColor(g_dummy_music.hypeActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);
    tft.drawString("HYPE", 106, 207);
    tft.drawBitmap(88, 207, image_hype_icon_bits, 16, 16,
                   g_dummy_music.hypeActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);

    tft.setTextColor(g_dummy_music.restActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);
    tft.drawString("REST", 199, 207);
    tft.drawBitmap(183, 206, image_rest_icon_bits, 15, 15,
                   g_dummy_music.restActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);

    // "Up next" preview (2 tracks)
    tft.setTextSize(1);
    tft.setTextColor(COLOR_SECONDARY_GR);
    tft.drawString("Up next:", 211, 96);
    tft.drawString("Before I Forget", 211, 115);
    tft.drawString("Slipknot", 210, 128);
    tft.drawString("101", 211, 145);
    tft.drawString("Seedhe Maut", 210, 158);

    // === TOP BAR ===
    tft.pushImage(0, 1, 321, 21, image_topbar_homescreen_margin_pixels);
    uint8_t battery_step = (g_dummy_settings.batteryPct / 20);
    if (battery_step > 0) {
        tft.fillRect(296, 7, 12, 9, COLOR_ACCENT);
    }
    tft.drawBitmap(286, 4, image_topbar_battery_case_bits, 24, 16, COLOR_SECONDARY_GR);
    tft.setTextColor(COLOR_ACCENT);
    tft.drawString("12:23 PM", 15, 7);
    tft.drawString("Monday: 14th May", 115, 7);
    tft.drawBitmap(257, 4, image_topbar_network_connected_bits, 15, 14, COLOR_ACCENT);
}

void draw_home_screen_workout_only() {
    // Background
    tft.fillScreen(COLOR_DARK_BG);

    // === WORKOUT BLOCK ===
    tft.setTextColor(COLOR_SECONDARY_GR);
    tft.setTextSize(2);
    tft.drawString("Incline dumbbell", 115, 50);
    tft.drawString("Press", 177, 72);

    tft.setTextColor(COLOR_WHITE);
    tft.setTextSize(1);
    tft.drawString("Target Muscles:", 30, 220);
    tft.drawString("chest, front delt, triceps", 129, 220);

    // Sets
    for (int i = 0; i < 5; i++) {
        int y_pos = 135 + (i * 13);
        if (i == g_dummy_workout.currentSetIndex) {
            tft.setTextColor(COLOR_ACCENT);
            tft.drawRect(151, y_pos - 2, 124, 13, COLOR_SECONDARY_GR);
        } else {
            tft.setTextColor(COLOR_WHITE);
        }
        tft.drawString(g_dummy_workout.sets[i].label, 161, y_pos);
    }

    // Mode boxes
    tft.drawRect(27, 125, 71, 28, COLOR_SECONDARY_GR);
    tft.drawRect(27, 165, 71, 28, COLOR_SECONDARY_GR);

    tft.setTextSize(2);
    tft.setTextColor(g_dummy_music.hypeActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);
    tft.drawString("HYPE", 46, 132);
    tft.drawBitmap(29, 131, image_hype_icon_bits, 16, 16,
                   g_dummy_music.hypeActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);

    tft.setTextColor(g_dummy_music.restActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);
    tft.drawString(g_dummy_music.restActive ? "REST" : "3:00", 47, 172);
    tft.drawBitmap(31, 171, image_rest_icon_bits, 15, 15,
                   g_dummy_music.restActive ? COLOR_ACTIVE_HYPE : COLOR_INACTIVE_LOGO);

    // === TOP BAR ===
    tft.pushImage(1, 1, 321, 21, image_topbar_homescreen_margin_pixels);
    uint8_t battery_step = (g_dummy_settings.batteryPct / 20);
    if (battery_step > 0) {
        tft.fillRect(297, 7, 12, 9, COLOR_ACCENT);
    }
    tft.drawBitmap(287, 4, image_topbar_battery_case_bits, 24, 16, COLOR_SECONDARY_GR);
    tft.setTextColor(COLOR_ACCENT);
    tft.setTextSize(1);
    tft.drawString("12:23 PM", 16, 7);
    tft.drawString("Monday: 14th May - Push A", 86, 7);
    tft.drawBitmap(258, 4, image_topbar_network_connected_bits, 15, 14, COLOR_ACCENT);
}
