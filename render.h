/**
 * All screen rendering.
 *
 * Each function corresponds to an AppState. They read app state only —
 * they don't mutate it. The top-level render callback dispatches based on
 * app->state and calls the appropriate function.
 */
#pragma once
#include "app.h"

void render_menu(Canvas* canvas, App* app);
void render_main(Canvas* canvas, App* app);
void render_stats(Canvas* canvas, App* app);
void render_achievements(Canvas* canvas, App* app);
void render_settings(Canvas* canvas, App* app);
void render_about(Canvas* canvas, App* app);
void render_reset_confirm(Canvas* canvas, App* app);
