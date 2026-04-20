#include "faces.h"

// ============================================================
//  HEADS — Liberty-style profile portrait facing left
// ============================================================
//
// Design notes: silhouette built scanline-by-scanline for solid reads,
// then white "carves" for eye, brow, nostril, mouth, and ear. The nose
// protrudes 2px past the main face line (cx-7 at the tip) for a strong,
// recognizable Roman/Liberty profile. Decorative laurel dots sit on the
// opposite (back-of-head) side of the rim.
// ============================================================

void draw_heads(Canvas* c, uint8_t cx, uint8_t cy) {
    // --- Silhouette fill (top to bottom) ---
    canvas_draw_line(c, cx - 1, cy - 9, cx + 1, cy - 9);   // hair tuft (top)
    canvas_draw_line(c, cx - 2, cy - 8, cx + 2, cy - 8);   // crown
    canvas_draw_line(c, cx - 3, cy - 7, cx + 3, cy - 7);   // skull widening
    canvas_draw_line(c, cx - 4, cy - 6, cx + 4, cy - 6);   // hair fullness
    canvas_draw_line(c, cx - 5, cy - 5, cx + 5, cy - 5);   // eye level (max width)
    canvas_draw_line(c, cx - 5, cy - 4, cx + 5, cy - 4);   // upper face
    canvas_draw_line(c, cx - 6, cy - 3, cx + 5, cy - 3);   // nose bridge begins
    canvas_draw_line(c, cx - 7, cy - 2, cx + 5, cy - 2);   // NOSE TIP (max protrusion)
    canvas_draw_line(c, cx - 6, cy - 1, cx + 4, cy - 1);   // under nose / upper lip
    canvas_draw_line(c, cx - 5, cy,     cx + 3, cy);       // mouth line
    canvas_draw_line(c, cx - 3, cy + 1, cx + 2, cy + 1);   // lower lip / chin start
    canvas_draw_line(c, cx - 2, cy + 2, cx + 1, cy + 2);   // chin
    canvas_draw_line(c, cx - 1, cy + 3, cx + 1, cy + 3);   // neck top
    canvas_draw_line(c, cx - 1, cy + 4, cx + 1, cy + 4);   // neck
    canvas_draw_line(c, cx - 1, cy + 5, cx + 2, cy + 5);   // neck/collar
    canvas_draw_line(c, cx - 3, cy + 6, cx + 3, cy + 6);   // collar
    canvas_draw_line(c, cx - 5, cy + 7, cx + 5, cy + 7);   // shoulder
    canvas_draw_line(c, cx - 6, cy + 8, cx + 6, cy + 8);   // shoulder (broad)

    // --- White-carved details ---
    canvas_set_color(c, ColorWhite);

    // Eye (2px wide - clearly visible)
    canvas_draw_dot(c, cx - 3, cy - 5);
    canvas_draw_dot(c, cx - 2, cy - 5);
    // Eye top lid (suggests brow)
    canvas_draw_dot(c, cx - 3, cy - 6);

    // Nostril
    canvas_draw_dot(c, cx - 5, cy - 1);

    // Mouth
    canvas_draw_line(c, cx - 3, cy, cx - 1, cy);

    // Ear indent (vertical slot)
    canvas_draw_dot(c, cx + 4, cy - 2);
    canvas_draw_dot(c, cx + 4, cy - 1);

    // Hairline hint (separates hair from forehead)
    canvas_draw_dot(c, cx + 3, cy - 6);
    canvas_draw_dot(c, cx + 4, cy - 5);

    canvas_set_color(c, ColorBlack);

    // --- Laurel wreath marks along rim (back-of-head side) ---
    canvas_draw_dot(c, cx + 7, cy - 5);
    canvas_draw_dot(c, cx + 8, cy - 3);
    canvas_draw_dot(c, cx + 8, cy);
    canvas_draw_dot(c, cx + 8, cy + 3);
    canvas_draw_dot(c, cx + 7, cy + 5);
    // Tiny leaf V-marks
    canvas_draw_dot(c, cx + 9, cy - 2);
    canvas_draw_dot(c, cx + 9, cy + 2);
}

// ============================================================
//  TAILS — Spread-wing eagle with heraldic shield
// ============================================================
//
// Design notes: fully symmetric around cx. Five-star arc above, pointed
// beak centered, heraldic shield on chest with horizontal band and four
// vertical stripes, wings spread edge-to-edge with distinct feather
// strokes, fanned tail below, olive branch (left talon) and arrows
// (right talon) completing the classic national-seal composition.
// ============================================================

void draw_tails(Canvas* c, uint8_t cx, uint8_t cy) {
    // --- Stars above eagle (arced arrangement) ---
    canvas_draw_dot(c, cx,     cy - 10);  // top center
    canvas_draw_dot(c, cx - 3, cy - 9);
    canvas_draw_dot(c, cx + 3, cy - 9);
    canvas_draw_dot(c, cx - 6, cy - 8);   // outer
    canvas_draw_dot(c, cx + 6, cy - 8);

    // --- Head + beak ---
    canvas_draw_dot(c, cx,     cy - 8);   // beak tip
    canvas_draw_line(c, cx - 1, cy - 7, cx + 1, cy - 7);  // head top
    canvas_draw_line(c, cx - 1, cy - 6, cx + 1, cy - 6);  // head mid

    // --- Neck ---
    canvas_draw_dot(c, cx, cy - 5);
    canvas_draw_line(c, cx - 1, cy - 4, cx + 1, cy - 4);

    // --- Shield (horizontal band + vertical stripes) ---
    // Top horizontal band (chief)
    canvas_draw_line(c, cx - 3, cy - 3, cx + 3, cy - 3);
    canvas_draw_line(c, cx - 3, cy - 2, cx + 3, cy - 2);
    // Vertical stripes (paly)
    canvas_draw_line(c, cx - 3, cy - 1, cx - 3, cy + 2);
    canvas_draw_line(c, cx - 1, cy - 1, cx - 1, cy + 2);
    canvas_draw_line(c, cx + 1, cy - 1, cx + 1, cy + 2);
    canvas_draw_line(c, cx + 3, cy - 1, cx + 3, cy + 2);
    // Shield point (bottom)
    canvas_draw_line(c, cx - 2, cy + 3, cx + 2, cy + 3);
    canvas_draw_dot(c, cx, cy + 4);

    // --- Left wing (primary feathers, radiating out) ---
    canvas_draw_line(c, cx - 4, cy - 3, cx - 7, cy - 6);  // upper primary
    canvas_draw_line(c, cx - 4, cy - 2, cx - 9, cy - 4);  // upper-mid
    canvas_draw_line(c, cx - 4, cy - 1, cx - 9, cy - 1);  // mid
    canvas_draw_line(c, cx - 4, cy,     cx - 9, cy + 1);  // lower-mid
    canvas_draw_line(c, cx - 4, cy + 1, cx - 7, cy + 2);  // lower primary
    // Wing tip articulation
    canvas_draw_dot(c, cx - 8, cy - 6);
    canvas_draw_dot(c, cx - 9, cy - 3);
    canvas_draw_dot(c, cx - 9, cy);

    // --- Right wing (mirror) ---
    canvas_draw_line(c, cx + 4, cy - 3, cx + 7, cy - 6);
    canvas_draw_line(c, cx + 4, cy - 2, cx + 9, cy - 4);
    canvas_draw_line(c, cx + 4, cy - 1, cx + 9, cy - 1);
    canvas_draw_line(c, cx + 4, cy,     cx + 9, cy + 1);
    canvas_draw_line(c, cx + 4, cy + 1, cx + 7, cy + 2);
    canvas_draw_dot(c, cx + 8, cy - 6);
    canvas_draw_dot(c, cx + 9, cy - 3);
    canvas_draw_dot(c, cx + 9, cy);

    // --- Tail feathers (5-feather fan below shield) ---
    canvas_draw_line(c, cx - 3, cy + 5, cx - 2, cy + 8);  // leftmost
    canvas_draw_line(c, cx - 1, cy + 5, cx - 1, cy + 8);  // left inner
    canvas_draw_line(c, cx,     cy + 5, cx,     cy + 9);  // center (longest)
    canvas_draw_line(c, cx + 1, cy + 5, cx + 1, cy + 8);  // right inner
    canvas_draw_line(c, cx + 3, cy + 5, cx + 2, cy + 8);  // rightmost

    // --- Olive branch (left talon) ---
    canvas_draw_line(c, cx - 3, cy + 4, cx - 5, cy + 6);  // branch
    canvas_draw_dot(c, cx - 6, cy + 4);                    // leaf
    canvas_draw_dot(c, cx - 7, cy + 5);                    // leaf
    canvas_draw_dot(c, cx - 6, cy + 7);                    // leaf

    // --- Arrows (right talon) ---
    canvas_draw_line(c, cx + 3, cy + 4, cx + 5, cy + 6);  // shaft
    canvas_draw_dot(c, cx + 6, cy + 4);                    // arrowhead
    canvas_draw_dot(c, cx + 7, cy + 5);                    // arrowhead
    canvas_draw_dot(c, cx + 6, cy + 7);                    // fletching
}

// ============================================================
//  Full-size coin body — triple ring + face art
// ============================================================

void draw_coin_full(Canvas* c, uint8_t cx, uint8_t cy, uint8_t face) {
    canvas_draw_circle(c, cx, cy, COIN_R);
    canvas_draw_circle(c, cx, cy, COIN_R - 1);
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
        // Near-edge highlight
        canvas_set_color(c, ColorWhite);
        canvas_draw_dot(c, cx, cy - 3);
        canvas_draw_dot(c, cx, cy + 3);
        canvas_set_color(c, ColorBlack);
    } else {
        // Rotating capsule
        uint8_t r = (hw < COIN_R) ? hw : COIN_R;
        uint8_t w = hw * 2 + 1;
        uint8_t h = COIN_R * 2 + 1;
        canvas_draw_rframe(c, cx - hw, cy - COIN_R, w, h, r);
        if(hw > 4) {
            canvas_draw_rframe(c, cx - hw + 1, cy - COIN_R + 1,
                               w - 2, h - 2, (r > 1) ? r - 1 : 1);
        }
        // Inner ring hint (face suggestion during medium rotation)
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
