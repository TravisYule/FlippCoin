#include <furi.h>
#include <furi_hal_random.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================
//  FlippCoin — Premium Coin Flip for Flipper Zero
// ============================================================

// Screen
#define SCREEN_W 128
#define SCREEN_H 64

// Coin geometry
#define COIN_CX 64
#define COIN_CY 28
#define COIN_R  14

// Result codes
#define COIN_NONE  0
#define COIN_HEADS 1
#define COIN_TAILS 2

// Animation: 32 frames at ~30 FPS ≈ 1.07 seconds
#define ANIM_TOTAL 32
#define TICK_MS    33

// Particles
#define NUM_PARTICLES 8

// History
#define HIST_MAX 20

// ============================================================
//  Animation Keyframes (precomputed)
// ============================================================

// Vertical offset: positive = upward from idle position
static const int8_t ANIM_Y[ANIM_TOTAL] = {
     0,  3,  5,  8, 10, 12, 13, 14,  // rise
    14, 15, 15, 15, 15, 15, 15, 14,  // apex hover
    14, 13, 12, 10,  8,  5,  3,  0,  // fall
     0,  0,  0,  0,  3,  2,  1,  0,  // land + bounce
};

// Coin half-width: 14=full circle, 2=edge-on
// 3 full rotations (8 frames each), then landing wobble
static const uint8_t ANIM_HW[ANIM_TOTAL] = {
    14, 10,  2, 10, 14, 10,  2, 10,  // rotation 1
    14, 10,  2, 10, 14, 10,  2, 10,  // rotation 2
    14, 10,  2, 10, 14, 10,  2, 10,  // rotation 3
    14, 13, 14, 14, 14, 14, 14, 14,  // settle
};

// Which face is visible: true=result face, false=opposite
// Toggles at each edge-on (hw=2). 6 toggles from true → back to true.
static const bool ANIM_FACE[ANIM_TOTAL] = {
    true,  true,  true,  false, false, false, false, true,
    true,  true,  true,  false, false, false, false, true,
    true,  true,  true,  false, false, false, false, true,
    true,  true,  true,  true,  true,  true,  true,  true,
};

// Haptic triggers: 0=none, 1=gentle tick on landing only
// Kept minimal to preserve motor longevity
static const uint8_t ANIM_VIB[ANIM_TOTAL] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0,  // single tick on landing
};

// ============================================================
//  Types
// ============================================================

typedef enum { StateIdle, StateFlipping, StateStats } AppState;

typedef struct {
    int16_t x, y;
    int8_t vx, vy;
    uint8_t life;
} Particle;

typedef struct {
    FuriMutex* mutex;
    AppState state;

    uint8_t result;          // current displayed result
    uint8_t pending_result;  // decided at flip start, revealed on land

    // Stats
    uint32_t total;
    uint32_t heads;
    uint32_t tails;
    uint16_t streak;
    uint16_t best_streak;
    uint8_t streak_side;
    uint8_t best_side;

    // History ring
    uint8_t hist[HIST_MAX];
    uint8_t hist_len;

    // Animation
    uint8_t frame;
    uint8_t sparkle;   // countdown for impact sparkle lines

    // Particles
    Particle parts[NUM_PARTICLES];

    // Idle tick counter (for gleam)
    uint32_t tick;
} FlipState;

typedef struct {
    InputEvent input;
} FlipEvent;

// ============================================================
//  Particles
// ============================================================

static void parts_spawn(FlipState* s) {
    for(uint8_t i = 0; i < NUM_PARTICLES; i++) {
        uint8_t r1, r2;
        furi_hal_random_fill_buf(&r1, 1);
        furi_hal_random_fill_buf(&r2, 1);
        s->parts[i].x = COIN_CX + (int8_t)(r1 % 20) - 10;
        s->parts[i].y = COIN_CY + COIN_R - 2;
        s->parts[i].vx = (int8_t)(r1 % 5) - 2;
        s->parts[i].vy = -((int8_t)(r2 % 3) + 2);
        s->parts[i].life = 5 + (r2 % 4);
    }
}

static void parts_tick(FlipState* s) {
    for(uint8_t i = 0; i < NUM_PARTICLES; i++) {
        if(s->parts[i].life == 0) continue;
        s->parts[i].x += s->parts[i].vx;
        s->parts[i].y += s->parts[i].vy;
        s->parts[i].vy += 1; // gravity
        s->parts[i].life--;
    }
}

static void parts_draw(Canvas* c, FlipState* s) {
    for(uint8_t i = 0; i < NUM_PARTICLES; i++) {
        if(s->parts[i].life == 0) continue;
        canvas_draw_dot(c, s->parts[i].x, s->parts[i].y);
        canvas_draw_dot(c, s->parts[i].x + 1, s->parts[i].y);
    }
}

// ============================================================
//  Coin Face Artwork
// ============================================================

static void draw_heads(Canvas* c, uint8_t cx, uint8_t cy) {
    // Filled silhouette profile facing left — reads clean at any scale

    // === Head shape (scanline fill, top to chin) ===
    canvas_draw_line(c, cx - 1, cy - 9, cx + 2, cy - 9);   // crown
    canvas_draw_line(c, cx - 2, cy - 8, cx + 3, cy - 8);   // top skull
    canvas_draw_line(c, cx - 3, cy - 7, cx + 4, cy - 7);   // skull
    canvas_draw_line(c, cx - 3, cy - 6, cx + 4, cy - 6);   // forehead
    canvas_draw_line(c, cx - 4, cy - 5, cx + 4, cy - 5);   // brow / eye level
    canvas_draw_line(c, cx - 5, cy - 4, cx + 4, cy - 4);   // upper nose
    canvas_draw_line(c, cx - 6, cy - 3, cx + 3, cy - 3);   // nose bridge
    canvas_draw_line(c, cx - 6, cy - 2, cx + 3, cy - 2);   // nose tip
    canvas_draw_line(c, cx - 5, cy - 1, cx + 3, cy - 1);   // upper lip
    canvas_draw_line(c, cx - 4, cy,     cx + 2, cy);        // mouth level
    canvas_draw_line(c, cx - 3, cy + 1, cx + 2, cy + 1);   // lower lip
    canvas_draw_line(c, cx - 2, cy + 2, cx + 1, cy + 2);   // chin

    // === Neck ===
    canvas_draw_line(c, cx - 1, cy + 3, cx + 1, cy + 3);
    canvas_draw_line(c, cx - 1, cy + 4, cx + 1, cy + 4);
    canvas_draw_line(c, cx - 1, cy + 5, cx + 1, cy + 5);

    // === Shoulders ===
    canvas_draw_line(c, cx - 4, cy + 6, cx + 4, cy + 6);
    canvas_draw_line(c, cx - 6, cy + 7, cx + 6, cy + 7);

    // === Carve details with white ===
    canvas_set_color(c, ColorWhite);
    canvas_draw_dot(c, cx - 2, cy - 5);                     // eye
    canvas_draw_dot(c, cx - 5, cy - 1);                     // nostril
    canvas_draw_line(c, cx - 3, cy, cx - 1, cy);            // mouth
    canvas_draw_dot(c, cx + 3, cy - 4);                     // ear top
    canvas_draw_dot(c, cx + 3, cy - 3);                     // ear bottom
    canvas_set_color(c, ColorBlack);

    // === Laurel leaf marks along rim ===
    canvas_draw_dot(c, cx - 9, cy - 3);
    canvas_draw_dot(c, cx - 8, cy - 4);
    canvas_draw_dot(c, cx - 9, cy + 3);
    canvas_draw_dot(c, cx - 8, cy + 4);
    canvas_draw_dot(c, cx + 8, cy - 4);
    canvas_draw_dot(c, cx + 9, cy - 3);
    canvas_draw_dot(c, cx + 8, cy + 4);
    canvas_draw_dot(c, cx + 9, cy + 3);
}

static void draw_tails(Canvas* c, uint8_t cx, uint8_t cy) {
    // Spread-wing eagle

    // --- Head & beak ---
    canvas_draw_dot(c, cx, cy - 9);
    canvas_draw_dot(c, cx - 1, cy - 8);
    canvas_draw_dot(c, cx, cy - 8);
    canvas_draw_dot(c, cx + 1, cy - 8);
    canvas_draw_dot(c, cx, cy - 7);

    // --- Body ---
    canvas_draw_line(c, cx, cy - 6, cx, cy + 2);

    // --- Shield on chest ---
    canvas_draw_frame(c, cx - 2, cy - 5, 5, 7);
    canvas_draw_line(c, cx, cy - 5, cx, cy + 1);

    // --- Left wing ---
    canvas_draw_line(c, cx - 3, cy - 4, cx - 7, cy - 8);
    canvas_draw_line(c, cx - 3, cy - 3, cx - 9, cy - 6);
    canvas_draw_line(c, cx - 3, cy - 2, cx - 9, cy - 3);
    canvas_draw_line(c, cx - 3, cy - 1, cx - 8, cy - 1);
    // feather tips
    canvas_draw_dot(c, cx - 8, cy - 9);
    canvas_draw_dot(c, cx - 10, cy - 7);
    canvas_draw_dot(c, cx - 10, cy - 4);

    // --- Right wing (mirror) ---
    canvas_draw_line(c, cx + 3, cy - 4, cx + 7, cy - 8);
    canvas_draw_line(c, cx + 3, cy - 3, cx + 9, cy - 6);
    canvas_draw_line(c, cx + 3, cy - 2, cx + 9, cy - 3);
    canvas_draw_line(c, cx + 3, cy - 1, cx + 8, cy - 1);
    canvas_draw_dot(c, cx + 8, cy - 9);
    canvas_draw_dot(c, cx + 10, cy - 7);
    canvas_draw_dot(c, cx + 10, cy - 4);

    // --- Tail feathers ---
    canvas_draw_line(c, cx, cy + 3, cx - 3, cy + 7);
    canvas_draw_line(c, cx, cy + 3, cx, cy + 8);
    canvas_draw_line(c, cx, cy + 3, cx + 3, cy + 7);
    canvas_draw_dot(c, cx - 1, cy + 8);
    canvas_draw_dot(c, cx + 1, cy + 8);

    // --- Talons ---
    canvas_draw_line(c, cx - 2, cy + 2, cx - 4, cy + 5);
    canvas_draw_line(c, cx + 2, cy + 2, cx + 4, cy + 5);
    canvas_draw_dot(c, cx - 5, cy + 5);
    canvas_draw_dot(c, cx + 5, cy + 5);

    // --- Stars above wings ---
    canvas_draw_dot(c, cx - 5, cy - 10);
    canvas_draw_dot(c, cx - 2, cy - 11);
    canvas_draw_dot(c, cx + 2, cy - 11);
    canvas_draw_dot(c, cx + 5, cy - 10);
}

// ============================================================
//  Coin Rendering
// ============================================================

static void draw_coin_full(Canvas* c, uint8_t cx, uint8_t cy, uint8_t face) {
    // Outer rim (2px thick)
    canvas_draw_circle(c, cx, cy, COIN_R);
    canvas_draw_circle(c, cx, cy, COIN_R - 1);
    // Inner decorative ring
    canvas_draw_circle(c, cx, cy, COIN_R - 3);

    if(face == COIN_HEADS) {
        draw_heads(c, cx, cy);
    } else if(face == COIN_TAILS) {
        draw_tails(c, cx, cy);
    } else {
        canvas_set_font(c, FontPrimary);
        canvas_draw_str_aligned(c, cx, cy, AlignCenter, AlignCenter, "?");
    }
}

static void draw_coin_narrow(Canvas* c, uint8_t cx, uint8_t cy, uint8_t hw) {
    // Coin during rotation: shows depth/thickness
    if(hw < 3) {
        // True edge-on: render with apparent thickness (3D illusion)
        uint8_t top = cy - COIN_R + 1;
        uint8_t bot = cy + COIN_R - 1;
        // Two parallel edges = front + back of coin's rim
        canvas_draw_line(c, cx - 1, top, cx - 1, bot);
        canvas_draw_line(c, cx + 1, top, cx + 1, bot);
        // Rounded caps (top and bottom of edge)
        canvas_draw_dot(c, cx, top - 1);
        canvas_draw_dot(c, cx, bot + 1);
        // Subtle highlight on the "near" edge
        canvas_set_color(c, ColorWhite);
        canvas_draw_dot(c, cx, cy - 3);
        canvas_draw_dot(c, cx, cy + 3);
        canvas_set_color(c, ColorBlack);
    } else {
        // Rotating at angle: rounded rectangle (ellipse approximation)
        uint8_t r = (hw < COIN_R) ? hw : COIN_R;
        uint8_t w = hw * 2 + 1;
        uint8_t h = COIN_R * 2 + 1;
        canvas_draw_rframe(c, cx - hw, cy - COIN_R, w, h, r);
        if(hw > 4) {
            canvas_draw_rframe(c, cx - hw + 1, cy - COIN_R + 1,
                               w - 2, h - 2, (r > 1) ? r - 1 : 1);
        }
        // Inner ring hint (shows some face hint when partially rotated)
        if(hw >= 7) {
            uint8_t ih = hw - 4;
            if(ih >= 1) {
                canvas_draw_rframe(c, cx - ih, cy - COIN_R + 3,
                                   ih * 2 + 1, (COIN_R - 3) * 2 + 1, (ih < 4) ? ih : 4);
            }
        }
    }
}

// ============================================================
//  Drop Shadow (ground-level cast shadow during flight)
// ============================================================

static void draw_shadow(Canvas* c, uint8_t height_offset) {
    // Dashed horizontal ellipse at ground. Width scales with coin height:
    // wider/more-spread at apex (diffuse), tighter when coin is low (sharper).
    uint8_t sw = 3 + (height_offset * 2) / 3;  // 3px at rest → ~13px at apex
    uint8_t sy = COIN_CY + COIN_R + 1;          // just below idle coin bottom

    // Dashed line — every-other pixel creates a soft "grey" impression on 1-bit display
    for(int16_t x = -sw; x <= sw; x += 2) {
        canvas_draw_dot(c, COIN_CX + x, sy);
    }
    // Slightly denser core (3 pixels under coin center) for ground contact feel
    canvas_draw_dot(c, COIN_CX - 1, sy);
    canvas_draw_dot(c, COIN_CX + 1, sy);
}

// ============================================================
//  Impact Sparkle Lines
// ============================================================

static void draw_sparkles(Canvas* c, uint8_t cx, uint8_t cy, uint8_t intensity) {
    // Radiating lines from coin center, 8 directions
    static const int8_t dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    static const int8_t dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    uint8_t inner = COIN_R + 2 + (3 - intensity);
    uint8_t outer = inner + 3;
    for(uint8_t i = 0; i < 8; i++) {
        canvas_draw_line(c,
            cx + dx[i] * inner, cy + dy[i] * inner,
            cx + dx[i] * outer, cy + dy[i] * outer);
    }
}

// ============================================================
//  Ratio Bar (heads-vs-tails visual proportion)
// ============================================================

static void draw_ratio_bar(Canvas* c, FlipState* s, uint8_t y) {
    if(s->total == 0) return;
    uint8_t bx = 10;
    uint8_t bw = 108;
    uint8_t bh = 4;

    // Outer frame
    canvas_draw_frame(c, bx, y, bw, bh);

    // "H" label before, "T" after (tiny tick marks for clarity)
    canvas_draw_dot(c, bx - 2, y + 1);
    canvas_draw_dot(c, bx - 2, y + 2);
    canvas_draw_dot(c, bx + bw + 1, y + 1);
    canvas_draw_dot(c, bx + bw + 1, y + 2);

    // Fill heads portion
    uint32_t heads_fill = (uint32_t)s->heads * (bw - 2) / s->total;
    if(heads_fill > 0) {
        canvas_draw_box(c, bx + 1, y + 1, heads_fill, bh - 2);
    }

    // Midpoint tick (shows 50% reference line)
    uint8_t mid = bx + bw / 2;
    canvas_draw_dot(c, mid, y - 1);
    canvas_draw_dot(c, mid, y + bh);
}

// ============================================================
//  Stats Overlay
// ============================================================

static void draw_stats(Canvas* c, FlipState* s) {
    // White background box
    canvas_set_color(c, ColorWhite);
    canvas_draw_box(c, 2, 2, 124, 60);
    canvas_set_color(c, ColorBlack);

    // Double border
    canvas_draw_rframe(c, 2, 2, 124, 60, 4);
    canvas_draw_rframe(c, 4, 4, 120, 56, 3);

    // Title
    canvas_set_font(c, FontPrimary);
    canvas_draw_str_aligned(c, 64, 8, AlignCenter, AlignTop, "~ STATISTICS ~");
    canvas_draw_line(c, 12, 19, 116, 19);

    canvas_set_font(c, FontSecondary);
    char buf[30];

    snprintf(buf, sizeof(buf), "Total Flips:  %lu", s->total);
    canvas_draw_str(c, 14, 27, buf);

    if(s->total > 0) {
        uint32_t hp = s->heads * 100 / s->total;
        uint32_t tp = 100 - hp;
        snprintf(buf, sizeof(buf), "Heads: %lu (%lu%%)", s->heads, hp);
        canvas_draw_str(c, 14, 35, buf);
        snprintf(buf, sizeof(buf), "Tails: %lu (%lu%%)", s->tails, tp);
        canvas_draw_str(c, 14, 43, buf);

        // Ratio bar — visual heads/tails proportion
        draw_ratio_bar(c, s, 47);

        snprintf(buf, sizeof(buf), "Streak:%u%c  Best:%u%c",
                 s->streak,
                 (s->streak_side == COIN_HEADS) ? 'H' : 'T',
                 s->best_streak,
                 (s->best_side == COIN_HEADS) ? 'H' : 'T');
        canvas_draw_str(c, 14, 59, buf);
    } else {
        canvas_draw_str(c, 14, 37, "No flips yet!");
    }
}

// ============================================================
//  Render Callback
// ============================================================

static void render_cb(Canvas* canvas, void* ctx) {
    FlipState* s = ctx;
    if(furi_mutex_acquire(s->mutex, 200) != FuriStatusOk) return;

    canvas_clear(canvas);

    // --- Stats screen ---
    if(s->state == StateStats) {
        draw_stats(canvas, s);
        furi_mutex_release(s->mutex);
        return;
    }

    // --- Title bar ---
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 1, AlignCenter, AlignTop, "FlippCoin");
    canvas_draw_line(canvas, 0, 12, SCREEN_W, 12);

    // --- Flip counter (top right) ---
    if(s->total > 0) {
        char cnt[12];
        snprintf(cnt, sizeof(cnt), "#%lu", s->total);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 126, 2, AlignRight, AlignTop, cnt);
    }

    // --- Compute coin state for this frame ---
    uint8_t cy = COIN_CY;
    uint8_t hw = COIN_R;
    uint8_t face = s->result;
    bool animating = (s->state == StateFlipping && s->frame < ANIM_TOTAL);

    if(animating) {
        uint8_t f = s->frame;
        cy = COIN_CY - ANIM_Y[f];
        hw = ANIM_HW[f];
        if(ANIM_FACE[f]) {
            face = s->pending_result;
        } else {
            face = (s->pending_result == COIN_HEADS) ? COIN_TAILS : COIN_HEADS;
        }
    }

    // --- Ground shadow (shows coin airborne, drawn BEFORE coin so coin overlaps it at rest) ---
    if(animating) {
        draw_shadow(canvas, ANIM_Y[s->frame]);
    }

    // --- Draw coin ---
    if(hw >= COIN_R - 2) {
        draw_coin_full(canvas, COIN_CX, cy, face);
    } else {
        draw_coin_narrow(canvas, COIN_CX, cy, hw);
    }

    // --- Impact sparkle lines ---
    if(s->sparkle > 0) {
        draw_sparkles(canvas, COIN_CX, COIN_CY, s->sparkle);
    }

    // --- Particles ---
    parts_draw(canvas, s);

    // --- Idle gleam (sweeping highlight across coin) ---
    if(s->state == StateIdle && s->result != COIN_NONE) {
        uint8_t phase = s->tick % 90;
        if(phase < 8) {
            int16_t gx = COIN_CX - COIN_R + (phase * 4);
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_line(canvas, gx, COIN_CY - 6, gx + 4, COIN_CY + 2);
            canvas_draw_line(canvas, gx + 1, COIN_CY - 6, gx + 5, COIN_CY + 2);
            canvas_set_color(canvas, ColorBlack);
        }
    }

    // --- Result text ---
    if(animating) {
        // Animated "flipping..." with cycling dots
        canvas_set_font(canvas, FontSecondary);
        uint8_t dots = (s->frame / 3) % 4;  // 0, 1, 2, 3 dots cycling
        char buf[16];
        snprintf(buf, sizeof(buf), "flipping%.*s", dots, "...");
        canvas_draw_str_aligned(canvas, 64, 47, AlignCenter, AlignTop, buf);
    } else {
        canvas_set_font(canvas, FontPrimary);
        if(s->result == COIN_HEADS) {
            canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignTop, "> HEADS! <");
        } else if(s->result == COIN_TAILS) {
            canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignTop, "> TAILS! <");
        } else {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, 47, AlignCenter, AlignTop, "Press OK to flip");
        }
    }

    // --- Bottom bar ---
    canvas_set_font(canvas, FontSecondary);
    if(s->total > 0) {
        char stats[20];
        snprintf(stats, sizeof(stats), "H:%lu T:%lu", s->heads, s->tails);
        canvas_draw_str_aligned(canvas, 1, 63, AlignLeft, AlignBottom, stats);
    }
    canvas_draw_str_aligned(canvas, 127, 63, AlignRight, AlignBottom,
                            s->total > 0 ? "Up:Stats" : "");

    furi_mutex_release(s->mutex);
}

// ============================================================
//  Input Callback
// ============================================================

static void input_cb(InputEvent* ev, void* ctx) {
    FuriMessageQueue* q = ctx;
    FlipEvent event = {.input = *ev};
    furi_message_queue_put(q, &event, FuriWaitForever);
}

// ============================================================
//  Main
// ============================================================

int32_t coin_flip_app(void* p) {
    UNUSED(p);

    FlipState* s = malloc(sizeof(FlipState));
    memset(s, 0, sizeof(FlipState));
    s->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    s->streak_side = COIN_HEADS;
    s->best_side = COIN_HEADS;

    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(FlipEvent));
    NotificationApp* notif = furi_record_open(RECORD_NOTIFICATION);

    ViewPort* vp = view_port_alloc();
    view_port_draw_callback_set(vp, render_cb, s);
    view_port_input_callback_set(vp, input_cb, queue);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, vp, GuiLayerFullscreen);

    FlipEvent event;
    bool running = true;

    while(running) {
        FuriStatus status = furi_message_queue_get(queue, &event, TICK_MS);

        // --- Input handling ---
        if(status == FuriStatusOk &&
           (event.input.type == InputTypeShort || event.input.type == InputTypePress)) {
            furi_mutex_acquire(s->mutex, FuriWaitForever);

            switch(event.input.key) {
            case InputKeyOk:
                if(s->state == StateIdle) {
                    uint8_t rb;
                    furi_hal_random_fill_buf(&rb, 1);
                    s->pending_result = (rb % 2 == 0) ? COIN_HEADS : COIN_TAILS;
                    s->frame = 0;
                    s->sparkle = 0;
                    s->state = StateFlipping;
                    memset(s->parts, 0, sizeof(s->parts));
                } else if(s->state == StateStats) {
                    s->state = StateIdle;
                }
                break;

            case InputKeyUp:
                if(s->state == StateIdle && s->total > 0) {
                    s->state = StateStats;
                }
                break;

            case InputKeyLeft:
                if(s->state == StateIdle) {
                    s->total = 0;
                    s->heads = 0;
                    s->tails = 0;
                    s->streak = 0;
                    s->best_streak = 0;
                    s->result = COIN_NONE;
                    s->hist_len = 0;
                    memset(s->parts, 0, sizeof(s->parts));
                    s->sparkle = 0;
                }
                break;

            case InputKeyBack:
                if(s->state == StateStats) {
                    s->state = StateIdle;
                } else if(s->state == StateFlipping) {
                    s->state = StateIdle;
                    s->frame = 0;
                    memset(s->parts, 0, sizeof(s->parts));
                    s->sparkle = 0;
                } else {
                    running = false;
                }
                break;

            default:
                break;
            }

            furi_mutex_release(s->mutex);
        }

        // --- Animation tick ---
        furi_mutex_acquire(s->mutex, FuriWaitForever);

        if(s->state == StateFlipping && s->frame < ANIM_TOTAL) {
            uint8_t f = s->frame;

            // Gentle haptic — single short tick only
            if(ANIM_VIB[f]) {
                notification_message(notif, &sequence_single_vibro);
            }

            // Spawn particles & sparkle on landing
            if(f == 24) {
                parts_spawn(s);
                s->sparkle = 3;
            }

            s->frame++;

            // Animation complete → commit result
            if(s->frame >= ANIM_TOTAL) {
                s->result = s->pending_result;
                s->total++;

                if(s->result == COIN_HEADS) s->heads++;
                else s->tails++;

                // Streak
                if(s->total == 1 || s->result != s->streak_side) {
                    s->streak_side = s->result;
                    s->streak = 1;
                } else {
                    s->streak++;
                }
                if(s->streak > s->best_streak) {
                    s->best_streak = s->streak;
                    s->best_side = s->streak_side;
                }

                // History (shift left if full)
                if(s->hist_len < HIST_MAX) {
                    s->hist[s->hist_len++] = s->result;
                } else {
                    for(uint8_t i = 0; i < HIST_MAX - 1; i++) {
                        s->hist[i] = s->hist[i + 1];
                    }
                    s->hist[HIST_MAX - 1] = s->result;
                }

                s->state = StateIdle;
            }
        }

        // Sparkle countdown
        if(s->sparkle > 0) s->sparkle--;

        parts_tick(s);
        s->tick++;

        furi_mutex_release(s->mutex);
        view_port_update(vp);
    }

    // Cleanup
    gui_remove_view_port(gui, vp);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    view_port_free(vp);
    furi_message_queue_free(queue);
    furi_mutex_free(s->mutex);
    free(s);

    return 0;
}
