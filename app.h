/**
 * FlippCoin — shared types and constants.
 *
 * This header is included by every translation unit in the app and defines:
 *   - Screen geometry and app-wide constants
 *   - The full App state struct (passed by pointer to every subsystem)
 *   - Result/state/menu enums
 *   - The Particle struct used by the animation subsystem
 *
 * Everything that multiple modules need lives here. Module-private types
 * stay in their individual .c files.
 */
#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ---------------------------------------------------------------
//  Version
// ---------------------------------------------------------------
#define FLIPPCOIN_VERSION_STRING "v3.2"

// ---------------------------------------------------------------
//  Screen
// ---------------------------------------------------------------
#define SCREEN_W 128
#define SCREEN_H 64

// ---------------------------------------------------------------
//  Coin geometry (idle position)
// ---------------------------------------------------------------
#define COIN_CX 64
#define COIN_CY 28
#define COIN_R  14

// ---------------------------------------------------------------
//  Result codes
// ---------------------------------------------------------------
#define COIN_NONE  0
#define COIN_HEADS 1
#define COIN_TAILS 2

// ---------------------------------------------------------------
//  Particles
// ---------------------------------------------------------------
#define NUM_PARTICLES 8

// ---------------------------------------------------------------
//  Event loop cadence (~30 FPS)
// ---------------------------------------------------------------
#define TICK_MS 33

// ---------------------------------------------------------------
//  Main menu items
// ---------------------------------------------------------------
typedef enum {
    MenuItemFlip = 0,
    MenuItemStats,
    MenuItemAchievements,
    MenuItemSettings,
    MenuItemAbout,
    MenuItemCount,
} MenuItem;

// ---------------------------------------------------------------
//  Achievements (bitmask — max 16 supported)
// ---------------------------------------------------------------
typedef enum {
    AchFirstFlip    = 1 << 0,  // First flip ever
    AchTenCount     = 1 << 1,  // 10 total flips
    AchCenturion    = 1 << 2,  // 100 total flips
    AchGrandMaster  = 1 << 3,  // 1000 total flips
    AchHotStreak    = 1 << 4,  // Streak of 5+
    AchImpossible   = 1 << 5,  // Streak of 10+
    AchAll          = AchFirstFlip | AchTenCount | AchCenturion
                    | AchGrandMaster | AchHotStreak | AchImpossible,
} Achievement;

#define ACHIEVEMENT_COUNT 6

// ---------------------------------------------------------------
//  Settings items
// ---------------------------------------------------------------
typedef enum {
    SettingHaptic = 0,
    SettingSound,
    SettingReset,
    SettingCount,
} SettingItem;

// ---------------------------------------------------------------
//  App states (screens)
// ---------------------------------------------------------------
typedef enum {
    StateMenu,
    StateIdle,
    StateFlipping,
    StateStats,
    StateAchievements,
    StateSettings,
    StateAbout,
    StateResetConfirm,
} AppState;

// ---------------------------------------------------------------
//  Particle (landing effect)
// ---------------------------------------------------------------
typedef struct {
    int16_t x, y;
    int8_t vx, vy;
    uint8_t life;
} Particle;

// ---------------------------------------------------------------
//  App state (owned by main, shared via mutex)
// ---------------------------------------------------------------
typedef struct {
    // Synchronization
    FuriMutex* mutex;

    // Current screen
    AppState state;
    AppState prev_state; // for Back navigation from confirm dialog

    // Navigation cursors
    uint8_t menu_cursor;
    uint8_t settings_cursor;

    // Coin
    uint8_t result;
    uint8_t pending_result;

    // Statistics (persisted)
    uint32_t total;
    uint32_t heads;
    uint32_t tails;
    uint16_t streak;
    uint16_t best_streak;
    uint8_t streak_side;
    uint8_t best_side;

    // Animation
    uint8_t frame;
    uint8_t sparkle; // countdown for impact sparkle lines

    // Particle effects
    Particle parts[NUM_PARTICLES];

    // Monotonic tick counter (for gleam and other idle animations)
    uint32_t tick;

    // Auto-flip mode: remaining flips in a rapid sequence (0 = normal)
    uint8_t auto_remaining;
    uint8_t auto_total;   // original count (for progress display)

    // Toast banner — briefly flashes a message (NEW BEST, achievements, etc.)
    uint8_t toast_timer;
    const char* toast_text;  // pointer to static literal (never freed)

    // Achievements (persisted, bitmask of Achievement values)
    uint16_t achievements;

    // Settings (persisted)
    bool haptic_enabled;
    bool sound_enabled;

    // Services
    NotificationApp* notif;
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
} App;

typedef struct {
    InputEvent input;
} AppEvent;
