#include "render.h"
#include "animation.h"
#include "faces.h"

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

static void draw_menu_item(Canvas* c, uint8_t x, uint8_t y, const char* label,
                           const char* value, bool selected) {
    if(selected) {
        canvas_draw_str(c, x - 6, y, ">");
    }
    canvas_draw_str(c, x, y, label);
    if(value) {
        canvas_draw_str_aligned(c, 114, y, AlignRight, AlignBottom, value);
    }
}

// ============================================================
//  Main menu
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
    // 5 items at 7px spacing to fit + leave room for version footer
    for(uint8_t i = 0; i < MenuItemCount; i++) {
        uint8_t y = 27 + i * 7;
        draw_menu_item(canvas, 24, y, MENU_LABELS[i], NULL, i == app->menu_cursor);
    }

    // Footer (version)
    canvas_draw_str_aligned(canvas, 64, 62, AlignCenter, AlignBottom,
                            FLIPPCOIN_VERSION_STRING);
}

// ============================================================
//  Main (coin flip) screen
// ============================================================

void render_main(Canvas* canvas, App* app) {
    bool animating = (app->state == StateFlipping && app->frame < ANIM_TOTAL);

    // Title bar
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 1, AlignCenter, AlignTop, "FlippCoin");
    canvas_draw_line(canvas, 0, 12, SCREEN_W, 12);

    // Flip counter (top-right)
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

    // Impact sparkle lines
    if(app->sparkle > 0) {
        draw_sparkles(canvas, COIN_CX, COIN_CY, app->sparkle);
    }

    // Particles
    particles_draw(canvas, app);

    // Idle gleam — diagonal highlight that sweeps across the coin
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

    // Result text / prompt
    if(animating) {
        canvas_set_font(canvas, FontSecondary);
        char buf[20];
        if(app->auto_total > 0) {
            // Auto-flip progress: "auto 3/10"
            uint8_t done = app->auto_total - app->auto_remaining + 1;
            snprintf(buf, sizeof(buf), "auto %u/%u", done, app->auto_total);
        } else {
            uint8_t dots = (app->frame / 3) % 4;
            snprintf(buf, sizeof(buf), "flipping%.*s", dots, "...");
        }
        canvas_draw_str_aligned(canvas, 64, 47, AlignCenter, AlignTop, buf);
    } else if(app->toast_timer > 0 && app->toast_text) {
        // Toast banner — briefly flashes a milestone message (NEW BEST, achievement names)
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

    // Bottom bar
    canvas_set_font(canvas, FontSecondary);
    if(app->total > 0) {
        char stats[20];
        snprintf(stats, sizeof(stats), "H:%lu T:%lu", app->heads, app->tails);
        canvas_draw_str_aligned(canvas, 1, 63, AlignLeft, AlignBottom, stats);
    }
    canvas_draw_str_aligned(canvas, 127, 63, AlignRight, AlignBottom, "Up=Menu");
}

// ============================================================
//  Statistics overlay — numbers + ratio bar
// ============================================================

// Sparkline showing recent flip history — bars above the baseline = heads,
// bars below = tails. Newest flip on the right, oldest on the left.
// Communicates both ratio (density above/below) and temporal patterns
// (streaks visible as runs of same-side bars) simultaneously.
static void draw_sparkline(Canvas* c, App* app, uint8_t y_baseline) {
    if(app->history_count == 0) return;

    const uint8_t bar_w = 3;
    uint8_t count = app->history_count;
    uint16_t total_w = count * bar_w;
    uint8_t x_start = (SCREEN_W - total_w) / 2;

    // Baseline (slightly extends past the bars)
    canvas_draw_line(c, x_start - 2, y_baseline,
                     x_start + total_w + 1, y_baseline);

    // H / T labels at the ends
    canvas_draw_dot(c, x_start - 4, y_baseline - 2);
    canvas_draw_dot(c, x_start - 4, y_baseline + 2);

    // Bars: i=0 → leftmost (oldest), i=count-1 → rightmost (newest)
    uint32_t h = app->history;
    for(uint8_t i = 0; i < count; i++) {
        uint8_t bit_pos = count - 1 - i;
        bool is_heads = (h >> bit_pos) & 1u;
        uint8_t x = x_start + i * bar_w;
        if(is_heads) {
            canvas_draw_box(c, x, y_baseline - 3, bar_w - 1, 3);
        } else {
            canvas_draw_box(c, x, y_baseline + 1, bar_w - 1, 3);
        }
    }
}

void render_stats(Canvas* canvas, App* app) {
    draw_titled_frame(canvas, "~ STATISTICS ~");
    canvas_set_font(canvas, FontSecondary);
    char buf[30];

    snprintf(buf, sizeof(buf), "Total Flips:  %lu", app->total);
    canvas_draw_str(canvas, 14, 27, buf);

    if(app->total > 0) {
        uint32_t hp = app->heads * 100 / app->total;
        uint32_t tp = 100 - hp;
        snprintf(buf, sizeof(buf), "Heads: %lu (%lu%%)", app->heads, hp);
        canvas_draw_str(canvas, 14, 35, buf);
        snprintf(buf, sizeof(buf), "Tails: %lu (%lu%%)", app->tails, tp);
        canvas_draw_str(canvas, 14, 43, buf);

        draw_sparkline(canvas, app, 48);

        snprintf(buf, sizeof(buf), "Streak:%u%c  Best:%u%c",
                 app->streak, (app->streak_side == COIN_HEADS) ? 'H' : 'T',
                 app->best_streak, (app->best_side == COIN_HEADS) ? 'H' : 'T');
        canvas_draw_str(canvas, 14, 59, buf);
    } else {
        canvas_draw_str(canvas, 14, 37, "No flips yet!");
    }
}

// ============================================================
//  Achievements — 6 milestones with unlock indicators
// ============================================================

typedef struct {
    uint16_t flag;
    const char* name;
    const char* detail;
} AchievementRow;

static const AchievementRow ACHIEVEMENTS[ACHIEVEMENT_COUNT] = {
    { AchFirstFlip,   "First Flip",   "any flip"      },
    { AchTenCount,    "Ten Count",    "10 flips"      },
    { AchCenturion,   "Centurion",    "100 flips"     },
    { AchGrandMaster, "Grand Master", "1000 flips"    },
    { AchHotStreak,   "Hot Streak",   "5 in a row"    },
    { AchImpossible,  "Impossible",   "10 in a row"   },
};

void render_achievements(Canvas* c, App* app) {
    draw_titled_frame(c, "ACHIEVEMENTS");

    canvas_set_font(c, FontSecondary);

    uint8_t unlocked = 0;
    for(uint8_t i = 0; i < ACHIEVEMENT_COUNT; i++) {
        bool got = (app->achievements & ACHIEVEMENTS[i].flag) != 0;
        if(got) unlocked++;

        uint8_t y = 26 + i * 6;

        // Checkbox
        canvas_draw_frame(c, 10, y - 4, 5, 5);
        if(got) {
            canvas_draw_box(c, 11, y - 3, 3, 3);
        }

        // Name + detail (dimmed if locked)
        char line[28];
        snprintf(line, sizeof(line), "%s (%s)",
                 ACHIEVEMENTS[i].name, ACHIEVEMENTS[i].detail);
        canvas_draw_str(c, 18, y, line);
    }

    // Progress footer
    char progress[24];
    snprintf(progress, sizeof(progress), "%u / %u unlocked",
             unlocked, ACHIEVEMENT_COUNT);
    canvas_draw_str_aligned(c, 64, 62, AlignCenter, AlignBottom, progress);
}

// ============================================================
//  Settings
// ============================================================

void render_settings(Canvas* canvas, App* app) {
    draw_titled_frame(canvas, "Settings");

    canvas_set_font(canvas, FontSecondary);
    draw_menu_item(canvas, 18, 30, "Haptic",
                   app->haptic_enabled ? "[ON]" : "[OFF]",
                   app->settings_cursor == SettingHaptic);
    draw_menu_item(canvas, 18, 40, "Sound",
                   app->sound_enabled ? "[ON]" : "[OFF]",
                   app->settings_cursor == SettingSound);
    draw_menu_item(canvas, 18, 50, "Reset Stats", NULL,
                   app->settings_cursor == SettingReset);

    canvas_draw_str_aligned(canvas, 64, 60, AlignCenter, AlignBottom,
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
    canvas_draw_str_aligned(canvas, 64, 52, AlignCenter, AlignTop,
                            "OK=Yes  Back=No");
}

// ============================================================
//  About
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
    canvas_draw_str_aligned(canvas, 64, 54, AlignCenter, AlignTop,
                            "github.com/TravisYule");
}
