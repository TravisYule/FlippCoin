/**
 * FlippCoin — main entry point.
 *
 * Responsibilities:
 *   - Boot: allocate state, load persisted data, wire up GUI + input + notification
 *   - Event loop: dispatch input + animation ticks based on AppState
 *   - Teardown: save state, free resources in correct order
 *
 * All rendering, animation data, audio, persistence, and coin artwork are
 * isolated in their own modules. This file is pure coordination.
 */
#include "app.h"
#include "animation.h"
#include "faces.h"
#include "render.h"
#include "persistence.h"
#include "sound.h"

#include <furi_hal_random.h>

// ============================================================
//  Render dispatcher
// ============================================================

static void render_callback(Canvas* canvas, void* ctx) {
    App* app = ctx;
    if(furi_mutex_acquire(app->mutex, 200) != FuriStatusOk) return;

    canvas_clear(canvas);

    switch(app->state) {
    case StateMenu:          render_menu(canvas, app); break;
    case StateIdle:          render_main(canvas, app); break;
    case StateFlipping:      render_main(canvas, app); break;
    case StateStats:         render_stats(canvas, app); break;
    case StateSettings:      render_settings(canvas, app); break;
    case StateAbout:         render_about(canvas, app); break;
    case StateResetConfirm:  render_reset_confirm(canvas, app); break;
    }

    furi_mutex_release(app->mutex);
}

// ============================================================
//  Input callback — pushes events into the main loop queue
// ============================================================

static void input_callback(InputEvent* event, void* ctx) {
    FuriMessageQueue* q = ctx;
    AppEvent ev = {.input = *event};
    furi_message_queue_put(q, &ev, FuriWaitForever);
}

// ============================================================
//  Stats helpers
// ============================================================

static void reset_all_stats(App* app) {
    app->total = 0;
    app->heads = 0;
    app->tails = 0;
    app->streak = 0;
    app->best_streak = 0;
    app->streak_side = COIN_HEADS;
    app->best_side = COIN_HEADS;
    app->result = COIN_NONE;
    particles_clear(app);
    app->sparkle = 0;
}

static void commit_flip_result(App* app) {
    app->result = app->pending_result;
    app->total++;

    if(app->result == COIN_HEADS) app->heads++;
    else app->tails++;

    // Streak tracking
    if(app->total == 1 || app->result != app->streak_side) {
        app->streak_side = app->result;
        app->streak = 1;
    } else {
        app->streak++;
    }
    // New best streak (require >= 3 to avoid celebrating trivial 2-in-a-rows)
    if(app->streak > app->best_streak) {
        app->best_streak = app->streak;
        app->best_side = app->streak_side;
        if(app->streak >= 3) {
            app->celebrate = 45; // ~1.5s banner at 30 FPS
            notification_message(app->notif, &sequence_success);
        }
    }
}

// ============================================================
//  State transitions
// ============================================================

static void go_to_state(App* app, AppState new_state) {
    app->prev_state = app->state;
    app->state = new_state;
}

static void start_flip(App* app) {
    uint8_t rb;
    furi_hal_random_fill_buf(&rb, 1);
    app->pending_result = (rb % 2 == 0) ? COIN_HEADS : COIN_TAILS;
    app->frame = 0;
    app->sparkle = 0;
    particles_clear(app);
    go_to_state(app, StateFlipping);
    sound_flip_launch(app);
}

// ============================================================
//  Input handlers per state
// ============================================================

static void handle_input_menu(App* app, InputKey key, bool* running) {
    switch(key) {
    case InputKeyUp:
        if(app->menu_cursor > 0) {
            app->menu_cursor--;
            sound_menu_click(app);
        }
        break;
    case InputKeyDown:
        if(app->menu_cursor < MenuItemCount - 1) {
            app->menu_cursor++;
            sound_menu_click(app);
        }
        break;
    case InputKeyOk:
        sound_menu_select(app);
        switch(app->menu_cursor) {
        case MenuItemFlip:     go_to_state(app, StateIdle); break;
        case MenuItemStats:    go_to_state(app, StateStats); break;
        case MenuItemSettings: go_to_state(app, StateSettings); break;
        case MenuItemAbout:    go_to_state(app, StateAbout); break;
        }
        break;
    case InputKeyBack:
        *running = false;
        break;
    default:
        break;
    }
}

#define AUTO_FLIP_COUNT 10

static void handle_input_idle(App* app, InputKey key) {
    switch(key) {
    case InputKeyOk:
        start_flip(app);
        break;
    case InputKeyRight:
        // Start auto-flip: N rapid flips in succession
        app->auto_remaining = AUTO_FLIP_COUNT;
        app->auto_total = AUTO_FLIP_COUNT;
        start_flip(app);
        break;
    case InputKeyUp:
        sound_menu_click(app);
        app->menu_cursor = MenuItemFlip;  // default back to Flip
        go_to_state(app, StateMenu);
        break;
    case InputKeyBack:
        go_to_state(app, StateMenu);
        break;
    default:
        break;
    }
}

static void handle_input_flipping(App* app, InputKey key) {
    if(key == InputKeyBack) {
        // Cancel flip — also cancels any remaining auto-flip chain
        app->frame = 0;
        app->sparkle = 0;
        app->auto_remaining = 0;
        app->auto_total = 0;
        particles_clear(app);
        go_to_state(app, StateIdle);
    }
}

static void handle_input_stats(App* app, InputKey key) {
    if(key == InputKeyBack || key == InputKeyOk) {
        sound_menu_click(app);
        go_to_state(app, StateMenu);
    }
}

static void handle_input_about(App* app, InputKey key) {
    if(key == InputKeyBack || key == InputKeyOk) {
        sound_menu_click(app);
        go_to_state(app, StateMenu);
    }
}

static void handle_input_settings(App* app, InputKey key) {
    switch(key) {
    case InputKeyUp:
        if(app->settings_cursor > 0) {
            app->settings_cursor--;
            sound_menu_click(app);
        }
        break;
    case InputKeyDown:
        if(app->settings_cursor < SettingCount - 1) {
            app->settings_cursor++;
            sound_menu_click(app);
        }
        break;
    case InputKeyOk:
        switch(app->settings_cursor) {
        case SettingHaptic:
            app->haptic_enabled = !app->haptic_enabled;
            persistence_save(app);
            sound_menu_select(app);
            break;
        case SettingSound:
            app->sound_enabled = !app->sound_enabled;
            persistence_save(app);
            sound_menu_select(app);
            break;
        case SettingReset:
            go_to_state(app, StateResetConfirm);
            break;
        }
        break;
    case InputKeyBack:
        sound_menu_click(app);
        go_to_state(app, StateMenu);
        break;
    default:
        break;
    }
}

static void handle_input_reset_confirm(App* app, InputKey key) {
    switch(key) {
    case InputKeyOk:
        reset_all_stats(app);
        persistence_save(app);
        sound_menu_select(app);
        go_to_state(app, StateSettings);
        break;
    case InputKeyBack:
        sound_menu_click(app);
        go_to_state(app, StateSettings);
        break;
    default:
        break;
    }
}

// ============================================================
//  Animation tick — advances flip animation frame
// ============================================================

static void tick_animation(App* app) {
    if(app->state != StateFlipping || app->frame >= ANIM_TOTAL) return;

    uint8_t f = app->frame;

    // Haptic feedback
    if(app->haptic_enabled && ANIM_VIB[f]) {
        notification_message(app->notif, &sequence_single_vibro);
    }

    // Impact effects on landing
    if(f == 24) {
        particles_spawn(app);
        app->sparkle = 3;
        sound_flip_land(app);
    }

    app->frame++;

    // Animation complete → commit result + save
    if(app->frame >= ANIM_TOTAL) {
        commit_flip_result(app);
        persistence_save(app);
        sound_reveal(app, app->result);

        // Auto-flip: chain into the next flip if any remain
        if(app->auto_remaining > 0) {
            app->auto_remaining--;
            if(app->auto_remaining > 0) {
                start_flip(app);
                return;
            } else {
                app->auto_total = 0;  // sequence complete
            }
        }

        go_to_state(app, StateIdle);
    }
}

// ============================================================
//  Main entry
// ============================================================

int32_t coin_flip_app(void* p) {
    UNUSED(p);

    // Allocate and initialize state
    App* app = malloc(sizeof(App));
    if(!app) return -1;
    memset(app, 0, sizeof(App));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->state = StateMenu;
    app->menu_cursor = MenuItemFlip;
    app->streak_side = COIN_HEADS;
    app->best_side = COIN_HEADS;
    app->haptic_enabled = true;
    app->sound_enabled = true;

    // Load persisted state (silent fail if missing/corrupt → defaults used)
    persistence_load(app);

    // Services
    app->event_queue = furi_message_queue_alloc(8, sizeof(AppEvent));
    app->notif = furi_record_open(RECORD_NOTIFICATION);

    // GUI
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, render_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app->event_queue);
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    // Main loop
    AppEvent event;
    bool running = true;

    while(running) {
        FuriStatus status = furi_message_queue_get(app->event_queue, &event, TICK_MS);

        // --- Input ---
        if(status == FuriStatusOk &&
           (event.input.type == InputTypeShort || event.input.type == InputTypePress)) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);

            switch(app->state) {
            case StateMenu:         handle_input_menu(app, event.input.key, &running); break;
            case StateIdle:         handle_input_idle(app, event.input.key); break;
            case StateFlipping:     handle_input_flipping(app, event.input.key); break;
            case StateStats:        handle_input_stats(app, event.input.key); break;
            case StateSettings:     handle_input_settings(app, event.input.key); break;
            case StateAbout:        handle_input_about(app, event.input.key); break;
            case StateResetConfirm: handle_input_reset_confirm(app, event.input.key); break;
            }

            furi_mutex_release(app->mutex);
        }

        // --- Tick updates ---
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        tick_animation(app);
        if(app->sparkle > 0) app->sparkle--;
        if(app->celebrate > 0) app->celebrate--;
        particles_tick(app);
        app->tick++;
        furi_mutex_release(app->mutex);

        view_port_update(app->view_port);
    }

    // Save on exit (in case we missed anything)
    persistence_save(app);

    // Teardown (reverse order of setup)
    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    view_port_free(app->view_port);
    furi_message_queue_free(app->event_queue);
    furi_mutex_free(app->mutex);
    free(app);

    return 0;
}
