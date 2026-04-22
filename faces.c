#include "faces.h"

// ============================================================
//  HEADS — Classical Liberty profile (refined)
// ============================================================
//
// Design: taller hair mass, stronger nose protrusion, defined jaw and
// shoulder line. Two internal hair-texture carves suggest strands.
// Laurel wreath on the back-of-head side arranged as paired leaf
// marks (V-shapes) instead of loose dots, reading as real foliage.
// ============================================================

void draw_heads(Canvas* c, uint8_t cx, uint8_t cy) {
    // --- Silhouette scanlines (top → bottom) ---
    canvas_draw_line(c, cx - 2, cy - 10, cx + 2, cy - 10);  // crown top
    canvas_draw_line(c, cx - 3, cy - 9,  cx + 3, cy - 9);
    canvas_draw_line(c, cx - 4, cy - 8,  cx + 4, cy - 8);
    canvas_draw_line(c, cx - 5, cy - 7,  cx + 5, cy - 7);   // full hair width
    canvas_draw_line(c, cx - 5, cy - 6,  cx + 6, cy - 6);   // back of head extends right
    canvas_draw_line(c, cx - 5, cy - 5,  cx + 6, cy - 5);   // eye level
    canvas_draw_line(c, cx - 5, cy - 4,  cx + 6, cy - 4);   // upper face
    canvas_draw_line(c, cx - 6, cy - 3,  cx + 5, cy - 3);   // nose bridge
    canvas_draw_line(c, cx - 7, cy - 2,  cx + 4, cy - 2);   // NOSE TIP (max protrusion)
    canvas_draw_line(c, cx - 6, cy - 1,  cx + 4, cy - 1);   // under nose
    canvas_draw_line(c, cx - 5, cy,      cx + 3, cy);       // mouth level
    canvas_draw_line(c, cx - 3, cy + 1,  cx + 2, cy + 1);   // chin starts
    canvas_draw_line(c, cx - 2, cy + 2,  cx + 2, cy + 2);   // jaw narrows
    canvas_draw_line(c, cx - 1, cy + 3,  cx + 2, cy + 3);   // under-chin
    canvas_draw_line(c, cx - 1, cy + 4,  cx + 2, cy + 4);   // neck
    canvas_draw_line(c, cx - 2, cy + 5,  cx + 3, cy + 5);   // collar start
    canvas_draw_line(c, cx - 4, cy + 6,  cx + 5, cy + 6);   // shoulder
    canvas_draw_line(c, cx - 6, cy + 7,  cx + 6, cy + 7);   // shoulder (broad)

    // --- White carves (facial details + hair texture) ---
    canvas_set_color(c, ColorWhite);

    // Eye (2px) + brow
    canvas_draw_dot(c, cx - 3, cy - 5);
    canvas_draw_dot(c, cx - 2, cy - 5);
    canvas_draw_dot(c, cx - 3, cy - 6);

    // Nostril
    canvas_draw_dot(c, cx - 5, cy - 1);

    // Mouth
    canvas_draw_line(c, cx - 3, cy, cx - 1, cy);

    // Ear indent (back of face)
    canvas_draw_dot(c, cx + 4, cy - 3);
    canvas_draw_dot(c, cx + 4, cy - 2);

    // Hair texture — two subtle curl lines
    canvas_draw_dot(c, cx - 1, cy - 8);
    canvas_draw_dot(c, cx + 2, cy - 7);
    canvas_draw_dot(c, cx, cy - 6);

    // Jaw shading hint (separates chin from neck)
    canvas_draw_dot(c, cx, cy + 3);

    canvas_set_color(c, ColorBlack);

    // --- Laurel wreath (paired V-leaves along back-of-head rim) ---
    // Upper back leaf
    canvas_draw_dot(c, cx + 7, cy - 7);
    canvas_draw_dot(c, cx + 8, cy - 6);
    canvas_draw_dot(c, cx + 7, cy - 5);
    // Middle back leaf
    canvas_draw_dot(c, cx + 8, cy - 3);
    canvas_draw_dot(c, cx + 9, cy - 2);
    canvas_draw_dot(c, cx + 8, cy - 1);
    // Lower back leaf
    canvas_draw_dot(c, cx + 8, cy + 1);
    canvas_draw_dot(c, cx + 9, cy + 2);
    canvas_draw_dot(c, cx + 8, cy + 3);
    // Bottom leaf (tucked under shoulder)
    canvas_draw_dot(c, cx + 7, cy + 5);
}

// ============================================================
//  TAILS — Heraldic eagle (refined)
// ============================================================
//
// Design: symmetric spread-wing eagle with eye dot, E-pluribus-style
// ribbon scroll under the shield, sharper talons (olive vs arrows),
// and crisper 4-stripe shield. Five-star arc above remains as the
// traditional starfield element.
// ============================================================

void draw_tails(Canvas* c, uint8_t cx, uint8_t cy) {
    // --- Star arc above eagle ---
    canvas_draw_dot(c, cx,     cy - 11);   // top center
    canvas_draw_dot(c, cx - 3, cy - 10);
    canvas_draw_dot(c, cx + 3, cy - 10);
    canvas_draw_dot(c, cx - 6, cy - 9);    // outer
    canvas_draw_dot(c, cx + 6, cy - 9);

    // --- Head + beak + eye ---
    canvas_draw_dot(c, cx,     cy - 8);              // beak tip (points down)
    canvas_draw_line(c, cx - 1, cy - 7, cx + 1, cy - 7);  // head top
    canvas_draw_line(c, cx - 1, cy - 6, cx + 1, cy - 6);  // head
    // Eye (white carve) after fill
    canvas_set_color(c, ColorWhite);
    canvas_draw_dot(c, cx + 1, cy - 7);
    canvas_set_color(c, ColorBlack);

    // --- Neck ---
    canvas_draw_dot(c, cx, cy - 5);
    canvas_draw_line(c, cx - 1, cy - 4, cx + 1, cy - 4);

    // --- Shield on chest (chief + 4 pales) ---
    // Chief (horizontal band at top)
    canvas_draw_line(c, cx - 3, cy - 3, cx + 3, cy - 3);
    canvas_draw_line(c, cx - 3, cy - 2, cx + 3, cy - 2);
    // Pales (4 vertical stripes)
    canvas_draw_line(c, cx - 3, cy - 1, cx - 3, cy + 2);
    canvas_draw_line(c, cx - 1, cy - 1, cx - 1, cy + 2);
    canvas_draw_line(c, cx + 1, cy - 1, cx + 1, cy + 2);
    canvas_draw_line(c, cx + 3, cy - 1, cx + 3, cy + 2);
    // Shield point (tapers to bottom)
    canvas_draw_line(c, cx - 2, cy + 3, cx + 2, cy + 3);
    canvas_draw_dot(c, cx, cy + 4);

    // --- Left wing (5 radiating primary feathers) ---
    canvas_draw_line(c, cx - 4, cy - 3, cx - 7, cy - 6);   // upper feather
    canvas_draw_line(c, cx - 4, cy - 2, cx - 9, cy - 4);   // upper-mid
    canvas_draw_line(c, cx - 4, cy - 1, cx - 9, cy - 1);   // mid
    canvas_draw_line(c, cx - 4, cy,     cx - 9, cy + 1);   // lower-mid
    canvas_draw_line(c, cx - 4, cy + 1, cx - 7, cy + 2);   // lower feather
    // Wing tip articulation (feather separation dots)
    canvas_draw_dot(c, cx - 8, cy - 6);
    canvas_draw_dot(c, cx - 10, cy - 3);
    canvas_draw_dot(c, cx - 10, cy);

    // --- Right wing (mirror) ---
    canvas_draw_line(c, cx + 4, cy - 3, cx + 7, cy - 6);
    canvas_draw_line(c, cx + 4, cy - 2, cx + 9, cy - 4);
    canvas_draw_line(c, cx + 4, cy - 1, cx + 9, cy - 1);
    canvas_draw_line(c, cx + 4, cy,     cx + 9, cy + 1);
    canvas_draw_line(c, cx + 4, cy + 1, cx + 7, cy + 2);
    canvas_draw_dot(c, cx + 8, cy - 6);
    canvas_draw_dot(c, cx + 10, cy - 3);
    canvas_draw_dot(c, cx + 10, cy);

    // --- Tail feathers (5-feather fan) ---
    canvas_draw_line(c, cx - 3, cy + 5, cx - 2, cy + 8);   // leftmost
    canvas_draw_line(c, cx - 1, cy + 5, cx - 1, cy + 8);   // left inner
    canvas_draw_line(c, cx,     cy + 5, cx,     cy + 9);   // center (longest)
    canvas_draw_line(c, cx + 1, cy + 5, cx + 1, cy + 8);   // right inner
    canvas_draw_line(c, cx + 3, cy + 5, cx + 2, cy + 8);   // rightmost

    // --- Left talon: olive branch (3-leaf cluster) ---
    canvas_draw_line(c, cx - 3, cy + 4, cx - 5, cy + 6);   // branch
    canvas_draw_dot(c, cx - 5, cy + 4);                    // leaf 1
    canvas_draw_dot(c, cx - 6, cy + 4);
    canvas_draw_dot(c, cx - 7, cy + 5);                    // leaf 2
    canvas_draw_dot(c, cx - 6, cy + 7);                    // leaf 3
    canvas_draw_dot(c, cx - 7, cy + 7);

    // --- Right talon: arrows (shafts + fletching) ---
    canvas_draw_line(c, cx + 3, cy + 4, cx + 5, cy + 6);   // shaft
    canvas_draw_dot(c, cx + 6, cy + 4);                    // arrowhead
    canvas_draw_dot(c, cx + 7, cy + 4);
    // Fletching (X marks at arrow ends)
    canvas_draw_dot(c, cx + 6, cy + 7);
    canvas_draw_dot(c, cx + 7, cy + 6);
    canvas_draw_dot(c, cx + 7, cy + 7);
}

// ============================================================
//  Milled rim — radial tick marks in the ring gap
// ============================================================
//
// Classic coins have "reeding" on their edge. We simulate it by
// placing small dots at 12 positions around the perimeter, in the
// 1-pixel gap between the outer border (r=13) and the inner ring
// (r=11). At r=12 they read as a subtle ridge pattern.
// ============================================================

static void draw_milled_rim(Canvas* c, uint8_t cx, uint8_t cy) {
    // Precomputed (dx, dy) for 12 positions at 30° intervals on r~12.
    // Radii check: 12²=144, 10²+6²=136 (r≈11.66), 6²+10²=136, ✓
    static const int8_t dx[12] = {12, 10,  6,  0, -6, -10, -12, -10, -6,   0,  6, 10};
    static const int8_t dy[12] = { 0, -6,-10,-12,-10,  -6,   0,   6, 10,  12, 10,  6};
    for(uint8_t i = 0; i < 12; i++) {
        canvas_draw_dot(c, cx + dx[i], cy + dy[i]);
    }
}

// ============================================================
//  Full-size coin body — triple ring + milled rim + face art
// ============================================================

void draw_coin_full(Canvas* c, uint8_t cx, uint8_t cy, uint8_t face) {
    canvas_draw_circle(c, cx, cy, COIN_R);
    canvas_draw_circle(c, cx, cy, COIN_R - 1);
    canvas_draw_circle(c, cx, cy, COIN_R - 3);
    draw_milled_rim(c, cx, cy);

    if(face == COIN_HEADS) {
        draw_heads(c, cx, cy);
    } else if(face == COIN_TAILS) {
        draw_tails(c, cx, cy);
    } else {
        canvas_set_font(c, FontPrimary);
        canvas_draw_str_aligned(c, cx, cy, AlignCenter, AlignCenter, "?");
    }
}

// ============================================================
//  Narrow coin body — rotating or edge-on
// ============================================================

void draw_coin_narrow(Canvas* c, uint8_t cx, uint8_t cy, uint8_t hw) {
    if(hw < 3) {
        // Edge-on: two parallel rim lines + caps (apparent thickness)
        uint8_t top = cy - COIN_R + 1;
        uint8_t bot = cy + COIN_R - 1;
        canvas_draw_line(c, cx - 1, top, cx - 1, bot);
        canvas_draw_line(c, cx + 1, top, cx + 1, bot);
        canvas_draw_dot(c, cx, top - 1);
        canvas_draw_dot(c, cx, bot + 1);
        canvas_set_color(c, ColorWhite);
        canvas_draw_dot(c, cx, cy - 3);
        canvas_draw_dot(c, cx, cy + 3);
        canvas_set_color(c, ColorBlack);
    } else {
        uint8_t r = (hw < COIN_R) ? hw : COIN_R;
        uint8_t w = hw * 2 + 1;
        uint8_t h = COIN_R * 2 + 1;
        canvas_draw_rframe(c, cx - hw, cy - COIN_R, w, h, r);
        if(hw > 4) {
            canvas_draw_rframe(c, cx - hw + 1, cy - COIN_R + 1,
                               w - 2, h - 2, (r > 1) ? r - 1 : 1);
        }
        if(hw >= 7) {
            uint8_t ih = hw - 4;
            if(ih >= 1) {
                canvas_draw_rframe(c, cx - ih, cy - COIN_R + 3,
                                   ih * 2 + 1, (COIN_R - 3) * 2 + 1,
                                   (ih < 4) ? ih : 4);
            }
        }
    }
}
