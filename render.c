#include "render.h"
#include "animation.h"
#include "faces.h"

// ============================================================
//  Layout constants
// ============================================================
//
// Frame geometry (inside a titled overlay box):
//   Outer rframe: (2, 2) size 124x60 → line at y=2 and y=61
//   Inner rframe: (4, 4) size 120x56 → line at y=4 and y=59
//   Title (FontPrimary): y=8 AlignTop → occupies y=8..17
//   Separator line:      y=19
//   Content area:        y=20..58 (39px of usable vertical space)
//
// Font metrics (approximate, drawn at baseline y):
//   FontPrimary:   ascent ~8px, descent ~2px (total ~10px)
//   FontSecondary: ascent ~6px, descent ~2px (total ~8px)
//                  — most labels have no descenders, effective ~7px
// ============================================================

// ============================================================
//  Shared helpers
// ============================================================

static void draw_titled_frame(Canvas* c, const char* title) {
    canvas_set_color(c, ColorWhite);
    canvas_draw_box(c, 2, 2, 124, 60);
    canvas_set_color(c, ColorBlack);
    canvas_draw_rframe(c, 2, 2, 124, 60, 4);
    canvas_draw_rframe(c, 4, 4, 120, 56, 3);
    canvas_set_font(c, FontPrimary);
    canvas_draw_str_aligned(c, 64, 8, AlignCenter, AlignTop, title);
    canvas_draw_line(c, 12, 19, 116, 19);
}

// Draw a menu row. Label uses baseline y; value is right-aligned at x=114
// using the same baseline (computed via canvas_string_width) so label and
// value are vertically aligned rather than offset by descent height.
static void draw_menu_item(Canvas* c, uint8_t x, uint8_t y, const char* label,
                           const char* value, bool selected) {
    if(selected) {
        canvas_draw_str(c, x - 6, y, ">");
    }
    canvas_draw_str(c, x, y, label);
    if(value) {
        uint16_t vw = canvas_string_width(c, value);
        if(vw > 100) vw = 100;
        canvas_draw_str(c, 114 - vw, y, value);
    }
}

// ============================================================
//  Main menu — 5 items at 8px spacing, no footer
// ============================================================
//
// Layout: items at y=27, 35, 43, 51, 59 fill the 39px content area
// exactly (item 5 bottom at y=59 = inner frame line). The version is
// shown on the About screen instead, so no footer is needed here.
// ============================================================

static const char* MENU_LABELS[MenuItemCount] = {
    "Flip",
    "Statistics",
    "Achievements",
    "Settings",
    "About",
};

void render_menu(Canvas* canvas, App* app) {
    draw_titled_frame(canvas, "FlippCoin");

    canvas_set_font(canvas, FontSecondary);
    for(uint8_t i = 0; i < MenuItemCount; i++) {
        uint8_t y = 27 + i * 8;
        draw_menu_item(canvas, 24, y, MENU_LABELS[i], NULL, i == app->menu_cursor);
    }
}

// ============================================================
//  Main (coin flip) screen
// ============================================================
//
// Layout:
//   y=1..10:  Title "FlippCoin" (FontPrimary)         — idle only
//   y=2..9:   Counter "#N" (FontSecondary, right)     — always
//   y=12:     Separator line                            — idle only
//   y=14..42: Coin at COIN_CY=28, r=14 (idle position)
//   y=44..53: Result text (FontPrimary) OR toast
//   y=46..53: Initial prompt (FontSecondary) OR animating text
//   y=55..63: Bottom bar (FontSecondary)
//
// Title/separator hide during animation because the coin at apex
// (cy=13) has artwork extending to y=2 and would overlap the title
// text. Counter stays because it's far right (x≥110) while coin
// artwork only spans x=53..75.
// ============================================================

void render_main(Canvas* canvas, App* app) {
    bool animating = (app->state == StateFlipping && app->frame < ANIM_TOTAL);

    // Title + separator — suppressed during flight so coin can fly high
    if(!animating) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 1, AlignCenter, AlignTop, "FlippCoin");
        canvas_draw_line(canvas, 0, 12, SCREEN_W, 12);
    }

    // Flip counter (top-right, stays visible — doesn't intersect coin)
    if(app->total > 0) {
        char cnt[12];
        snprintf(cnt, sizeof(cnt), "#%lu", app->total);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 126, 2, AlignRight, AlignTop, cnt);
    }

    // Compute coin state for this frame
    uint8_t cy = COIN_CY;
    uint8_t hw = COIN_R;
    uint8_t face = app->result;

    if(animating) {
        uint8_t f = app->frame;
        cy = COIN_CY - ANIM_Y[f];
        hw = ANIM_HW[f];
        face = ANIM_FACE[f] ? app->pending_result
                            : ((app->pending_result == COIN_HEADS) ? COIN_TAILS : COIN_HEADS);
    }

    // Shadow (only during flight)
    if(animating) {
        draw_shadow(canvas, ANIM_Y[app->frame]);
    }

    // Coin body
    if(hw >= COIN_R - 2) {
        draw_coin_full(canvas, COIN_CX, cy, face);
    } else {
        draw_coin_narrow(canvas, COIN_CX, cy, hw);
    }

    // Impact sparkle lines (on landing)
    if(app->sparkle > 0) {
        draw_sparkles(canvas, COIN_CX, COIN_CY, app->sparkle);
    }

    // Particles
    particles_draw(canvas, app);

    // Idle gleam — diagonal highlight sweeping across the coin
    if(app->state == StateIdle && app->result != COIN_NONE) {
        uint8_t phase = app->tick % 90;
        if(phase < 8) {
            int16_t gx = COIN_CX - COIN_R + (phase * 4);
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_line(canvas, gx, COIN_CY - 6, gx + 4, COIN_CY + 2);
            canvas_draw_line(canvas, gx + 1, COIN_CY - 6, gx + 5, COIN_CY + 2);
            canvas_set_color(canvas, ColorBlack);
        }
    }

    // Result text / toast / prompt (mutually exclusive)
    if(animating) {
        canvas_set_font(canvas, FontSecondary);
        char buf[20];
        if(app->auto_total > 0) {
            uint8_t done = app->auto_total - app->auto_remaining + 1;
            snprintf(buf, sizeof(buf), "auto %u/%u", done, app->auto_total);
        } else {
            uint8_t dots = (app->frame / 3) % 4;
            snprintf(buf, sizeof(buf), "flipping%.*s", dots, "...");
        }
        canvas_draw_str_aligned(canvas, 64, 47, AlignCenter, AlignTop, buf);
    } else if(app->toast_timer > 0 && app->toast_text) {
        // Inverted toast banner — box y=43..54, text centered at y=44 AlignTop
        canvas_draw_box(canvas, 10, 43, 108, 12);
        canvas_set_color(canvas, ColorWhite);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignTop, app->toast_text);
        canvas_set_color(canvas, ColorBlack);
    } else {
        canvas_set_font(canvas, FontPrimary);
        if(app->result == COIN_HEADS) {
            canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignTop, "> HEADS! <");
        } else if(app->result == COIN_TAILS) {
            canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignTop, "> TAILS! <");
        } else {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, 46, AlignCenter, AlignTop, "OK=flip   Right=auto");
        }
    }

    // Bottom bar (FontSecondary, AlignBottom at y=63 → occupies y=55..63)
    canvas_set_font(canvas, FontSecondary);
    if(app->total > 0) {
        char stats[20];
        snprintf(stats, sizeof(stats), "H:%lu T:%lu", app->heads, app->tails);
        canvas_draw_str_aligned(canvas, 1, 63, AlignLeft, AlignBottom, stats);
    }
    canvas_draw_str_aligned(canvas, 127, 63, AlignRight, AlignBottom, "Up=Menu");
}

// ============================================================
//  Sparkline — recent flip history as up/down bars
// ============================================================
//
// Bars above the baseline = heads, below = tails, newest on right.
// Uses 2px-tall bars (instead of 3px) for tighter stats-screen fit
// without colliding with the Tails text above or the Streak text
// below.
// ============================================================

static void draw_sparkline(Canvas* c, App* app, uint8_t y_baseline) {
    if(app->history_count == 0) return;

    const uint8_t bar_w = 3;
    uint8_t count = app->history_count;
    uint16_t total_w = count * bar_w;
    uint8_t x_start = (SCREEN_W - total_w) / 2;

    // Baseline (extends slightly past the bars)
    canvas_draw_line(c, x_start - 2, y_baseline,
                     x_start + total_w + 1, y_baseline);

    // H/T dots on the left end for visual context
    canvas_draw_dot(c, x_start - 4, y_baseline - 1);
    canvas_draw_dot(c, x_start - 4, y_baseline + 1);

    // Bars: i=0 → leftmost (oldest), i=count-1 → rightmost (newest)
    uint32_t h = app->history;
    for(uint8_t i = 0; i < count; i++) {
        uint8_t bit_pos = count - 1 - i;
        bool is_heads = (h >> bit_pos) & 1u;
        uint8_t x = x_start + i * bar_w;
        if(is_heads) {
            canvas_draw_box(c, x, y_baseline - 2, bar_w - 1, 2);
        } else {
            canvas_draw_box(c, x, y_baseline + 1, bar_w - 1, 2);
        }
    }
}

// ============================================================
//  Statistics screen
// ============================================================
//
// Layout (all within frame content area y=20..58):
//   y=27: Total Flips: N           (baseline, FontSecondary)
//   y=35: Heads: N (P%)
//   y=43: Tails: N (P%)
//   y=48: Sparkline baseline (2px bars up/down, spans y=46..50)
//   y=57: Streak:NX  Best:NX
// ============================================================

void render_stats(Canvas* canvas, App* app) {
    draw_titled_frame(canvas, "Statistics");
    canvas_set_font(canvas, FontSecondary);
    char buf[30];

    snprintf(buf, sizeof(buf), "Total Flips: %lu", app->total);
    canvas_draw_str(canvas, 14, 27, buf);

    if(app->total > 0) {
        uint32_t hp = app->heads * 100 / app->total;
        uint32_t tp = 100 - hp;
        snprintf(buf, sizeof(buf), "Heads: %lu (%lu%%)", app->heads, hp);
        canvas_draw_str(canvas, 14, 35, buf);
        snprintf(buf, sizeof(buf), "Tails: %lu (%lu%%)", app->tails, tp);
        canvas_draw_str(canvas, 14, 43, buf);

        // Sparkline baseline y=48, 2px bars → total spans y=46..50
        draw_sparkline(canvas, app, 48);

        snprintf(buf, sizeof(buf), "Streak:%u%c  Best:%u%c",
                 app->streak, (app->streak_side == COIN_HEADS) ? 'H' : 'T',
                 app->best_streak, (app->best_side == COIN_HEADS) ? 'H' : 'T');
        canvas_draw_str(canvas, 14, 57, buf);
    } else {
        canvas_draw_str_aligned(canvas, 64, 38, AlignCenter, AlignTop,
                                "No flips yet!");
        canvas_draw_str_aligned(canvas, 64, 48, AlignCenter, AlignTop,
                                "Flip a coin to start.");
    }
}

// ============================================================
//  Achievements screen
// ============================================================
//
// Layout: count embedded in title ("Achievements N/M"), six rows
// at 6px spacing fill the content area. No footer needed.
// ============================================================

typedef struct {
    uint16_t flag;
    const char* name;
    const char* detail;
} AchievementRow;

// Name on left, goal right-aligned to x=114 (keeps text within inner
// frame regardless of how long the name is).
static const AchievementRow ACHIEVEMENTS[ACHIEVEMENT_COUNT] = {
    { AchFirstFlip,   "First Flip",   "1"      },
    { AchTenCount,    "Ten Count",    "10"     },
    { AchCenturion,   "Centurion",    "100"    },
    { AchGrandMaster, "Grand Master", "1000"   },
    { AchHotStreak,   "Hot Streak",   "5 row"  },
    { AchImpossible,  "Impossible",   "10 row" },
};

void render_achievements(Canvas* c, App* app) {
    // Count unlocked first so we can put it in the title
    uint8_t unlocked = 0;
    for(uint8_t i = 0; i < ACHIEVEMENT_COUNT; i++) {
        if(app->achievements & ACHIEVEMENTS[i].flag) unlocked++;
    }

    char title[24];
    snprintf(title, sizeof(title), "Achievements %u/%u",
             unlocked, ACHIEVEMENT_COUNT);
    draw_titled_frame(c, title);

    canvas_set_font(c, FontSecondary);
    for(uint8_t i = 0; i < ACHIEVEMENT_COUNT; i++) {
        bool got = (app->achievements & ACHIEVEMENTS[i].flag) != 0;
        // Baselines at y=27, 33, 39, 45, 51, 57 (6px spacing)
        uint8_t y = 27 + i * 6;

        // Checkbox (5x5 at y-4..y)
        canvas_draw_frame(c, 10, y - 4, 5, 5);
        if(got) {
            canvas_draw_box(c, 11, y - 3, 3, 3);
        }

        // Name on left
        canvas_draw_str(c, 18, y, ACHIEVEMENTS[i].name);

        // Goal right-aligned at x=114
        uint16_t gw = canvas_string_width(c, ACHIEVEMENTS[i].detail);
        canvas_draw_str(c, 114 - gw, y, ACHIEVEMENTS[i].detail);
    }
}

// ============================================================
//  Settings screen
// ============================================================
//
// Layout: 3 items at 10px spacing in content area, footer at bottom.
//   y=28: Haptic   [ON/OFF]
//   y=38: Sound    [ON/OFF]
//   y=48: Reset Stats
//   y=58: footer "OK=toggle  Back=return" (AlignBottom)
// ============================================================

void render_settings(Canvas* canvas, App* app) {
    draw_titled_frame(canvas, "Settings");

    canvas_set_font(canvas, FontSecondary);
    draw_menu_item(canvas, 18, 28, "Haptic",
                   app->haptic_enabled ? "[ON]" : "[OFF]",
                   app->settings_cursor == SettingHaptic);
    draw_menu_item(canvas, 18, 38, "Sound",
                   app->sound_enabled ? "[ON]" : "[OFF]",
                   app->settings_cursor == SettingSound);
    draw_menu_item(canvas, 18, 48, "Reset Stats", NULL,
                   app->settings_cursor == SettingReset);

    // Footer — FontSecondary AlignBottom at y=58 → spans y=50..58
    // Reset item baseline y=48 → bottom y=50. Touch (1px), OK.
    canvas_draw_str_aligned(canvas, 64, 58, AlignCenter, AlignBottom,
                            "OK=toggle  Back=return");
}

// ============================================================
//  Reset confirmation dialog
// ============================================================

void render_reset_confirm(Canvas* canvas, App* app) {
    (void)app;
    draw_titled_frame(canvas, "Reset Stats?");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignTop,
                            "This will erase all");
    canvas_draw_str_aligned(canvas, 64, 37, AlignCenter, AlignTop,
                            "flip history & streaks.");

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 48, AlignCenter, AlignTop,
                            "OK=Yes  Back=No");
}

// ============================================================
//  About screen
// ============================================================
//
// Layout:
//   y=24..33: "FlippCoin"             (FontPrimary)
//   y=35..42: version                 (FontSecondary)
//   y=44..51: "by Travis Yule"        (FontSecondary)
//   y=52..59: "github.com/TravisYule" (FontSecondary)
//
// Every baseline chosen so text stays within the inner frame (y=59).
// ============================================================

void render_about(Canvas* canvas, App* app) {
    (void)app;
    draw_titled_frame(canvas, "About");

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 24, AlignCenter, AlignTop, "FlippCoin");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 35, AlignCenter, AlignTop,
                            FLIPPCOIN_VERSION_STRING);
    canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignTop,
                            "by Travis Yule");
    canvas_draw_str_aligned(canvas, 64, 52, AlignCenter, AlignTop,
                            "github.com/TravisYule");
}
