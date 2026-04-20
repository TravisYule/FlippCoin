#include "animation.h"
#include <furi_hal_random.h>

// ============================================================
//  Precomputed animation keyframes
// ============================================================

// Vertical offset: positive = upward from idle position.
// Parabolic arc over frames 0-23, then land + bounce frames 24-31.
const int8_t ANIM_Y[ANIM_TOTAL] = {
     0,  3,  5,  8, 10, 12, 13, 14,  // rise
    14, 15, 15, 15, 15, 15, 15, 14,  // apex hover
    14, 13, 12, 10,  8,  5,  3,  0,  // fall
     0,  0,  0,  0,  3,  2,  1,  0,  // land + bounce
};

// Coin half-width: 14 = full circle, 2 = edge-on.
// Three full rotations during the airborne phase, then settle on landing.
const uint8_t ANIM_HW[ANIM_TOTAL] = {
    14, 10,  2, 10, 14, 10,  2, 10,  // rotation 1
    14, 10,  2, 10, 14, 10,  2, 10,  // rotation 2
    14, 10,  2, 10, 14, 10,  2, 10,  // rotation 3
    14, 13, 14, 14, 14, 14, 14, 14,  // settle wobble
};

// Which face is visible: true = result face, false = opposite.
// Toggles at each edge-on frame; 6 toggles total (even) → ends on result.
const bool ANIM_FACE[ANIM_TOTAL] = {
    true,  true,  true,  false, false, false, false, true,
    true,  true,  true,  false, false, false, false, true,
    true,  true,  true,  false, false, false, false, true,
    true,  true,  true,  true,  true,  true,  true,  true,
};

// Haptic triggers: 0 = none, 1 = gentle tick.
// Minimal to preserve motor longevity — just the landing.
const uint8_t ANIM_VIB[ANIM_TOTAL] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0,
};

// ============================================================
//  Particles — spawn on landing, fly outward, affected by gravity
// ============================================================

void particles_spawn(App* app) {
    for(uint8_t i = 0; i < NUM_PARTICLES; i++) {
        uint8_t r1, r2;
        furi_hal_random_fill_buf(&r1, 1);
        furi_hal_random_fill_buf(&r2, 1);
        app->parts[i].x = COIN_CX + (int8_t)(r1 % 20) - 10;
        app->parts[i].y = COIN_CY + COIN_R - 2;
        app->parts[i].vx = (int8_t)(r1 % 5) - 2;
        app->parts[i].vy = -((int8_t)(r2 % 3) + 2);
        app->parts[i].life = 5 + (r2 % 4);
    }
}

void particles_clear(App* app) {
    memset(app->parts, 0, sizeof(app->parts));
}

void particles_tick(App* app) {
    for(uint8_t i = 0; i < NUM_PARTICLES; i++) {
        if(app->parts[i].life == 0) continue;
        app->parts[i].x += app->parts[i].vx;
        app->parts[i].y += app->parts[i].vy;
        app->parts[i].vy += 1; // gravity
        app->parts[i].life--;
    }
}

void particles_draw(Canvas* c, App* app) {
    for(uint8_t i = 0; i < NUM_PARTICLES; i++) {
        if(app->parts[i].life == 0) continue;
        canvas_draw_dot(c, app->parts[i].x, app->parts[i].y);
        canvas_draw_dot(c, app->parts[i].x + 1, app->parts[i].y);
    }
}

// ============================================================
//  Ground shadow — softer at altitude, tighter near ground
// ============================================================

void draw_shadow(Canvas* c, uint8_t height_offset) {
    uint8_t sw = 3 + (height_offset * 2) / 3;
    uint8_t sy = COIN_CY + COIN_R + 1;

    // Dashed ellipse (every-other pixel reads as 50% grey on 1-bit LCD)
    for(int16_t x = -sw; x <= sw; x += 2) {
        canvas_draw_dot(c, COIN_CX + x, sy);
    }
    // Denser core right under coin for ground-contact feel
    canvas_draw_dot(c, COIN_CX - 1, sy);
    canvas_draw_dot(c, COIN_CX + 1, sy);
}

// ============================================================
//  Impact sparkle lines — 8 radiating strokes on landing
// ============================================================

void draw_sparkles(Canvas* c, uint8_t cx, uint8_t cy, uint8_t intensity) {
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
