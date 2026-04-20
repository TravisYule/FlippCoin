/**
 * Coin face artwork and coin-body rendering.
 *
 * draw_heads / draw_tails render the interior pixel art (profile portrait
 * and spread-wing eagle respectively) centered on (cx, cy).
 *
 * draw_coin_full / draw_coin_narrow pick the appropriate body style based
 * on the coin's current half-width — full circle with face when facing
 * the viewer, rounded rectangle when at an angle, edge-on rectangle with
 * apparent depth when rotating through perpendicular.
 */
#pragma once
#include "app.h"

void draw_heads(Canvas* c, uint8_t cx, uint8_t cy);
void draw_tails(Canvas* c, uint8_t cx, uint8_t cy);

void draw_coin_full(Canvas* c, uint8_t cx, uint8_t cy, uint8_t face);
void draw_coin_narrow(Canvas* c, uint8_t cx, uint8_t cy, uint8_t hw);
