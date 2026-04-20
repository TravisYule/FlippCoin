/**
 * Animation system: precomputed keyframes + particle physics + effects.
 *
 * The coin flip is driven by four parallel 32-frame tables:
 *   ANIM_Y     — vertical offset (parabolic arc + bounce)
 *   ANIM_HW    — coin half-width (rotation via horizontal squash)
 *   ANIM_FACE  — which face is visible (result vs opposite)
 *   ANIM_VIB   — when to trigger haptic feedback
 *
 * Particles and sparkles are spawned on impact (frame 24) and live for a
 * handful of frames after.
 */
#pragma once
#include "app.h"

#define ANIM_TOTAL 32

extern const int8_t ANIM_Y[ANIM_TOTAL];
extern const uint8_t ANIM_HW[ANIM_TOTAL];
extern const bool ANIM_FACE[ANIM_TOTAL];
extern const uint8_t ANIM_VIB[ANIM_TOTAL];

// Particle system
void particles_spawn(App* app);
void particles_clear(App* app);
void particles_tick(App* app);
void particles_draw(Canvas* c, App* app);

// Ground shadow (drawn under coin during flight)
void draw_shadow(Canvas* c, uint8_t height_offset);

// Radiating impact lines (drawn on landing)
void draw_sparkles(Canvas* c, uint8_t cx, uint8_t cy, uint8_t intensity);
