#include "sound.h"
#include <furi_hal_speaker.h>

// Brief blocking tone. Returns without playing if speaker is unavailable
// or sound is disabled — callers don't need to check.
static void play_tone(App* app, float frequency, float volume, uint32_t duration_ms) {
    if(!app->sound_enabled) return;
    if(!furi_hal_speaker_acquire(30)) return;
    furi_hal_speaker_start(frequency, volume);
    furi_delay_ms(duration_ms);
    furi_hal_speaker_stop();
    furi_hal_speaker_release();
}

// Two-tone sweep — useful for chirpy UI feedback.
static void play_sweep(App* app, float f1, float f2, float volume, uint32_t ms_each) {
    if(!app->sound_enabled) return;
    if(!furi_hal_speaker_acquire(30)) return;
    furi_hal_speaker_start(f1, volume);
    furi_delay_ms(ms_each);
    furi_hal_speaker_start(f2, volume);
    furi_delay_ms(ms_each);
    furi_hal_speaker_stop();
    furi_hal_speaker_release();
}

void sound_menu_click(App* app) {
    play_tone(app, 1200.0f, 0.2f, 12);
}

void sound_menu_select(App* app) {
    play_sweep(app, 800.0f, 1400.0f, 0.25f, 18);
}

void sound_flip_launch(App* app) {
    play_sweep(app, 400.0f, 900.0f, 0.3f, 15);
}

void sound_flip_land(App* app) {
    play_tone(app, 220.0f, 0.4f, 25);
}

void sound_reveal(App* app, uint8_t result) {
    // Heads = bright rising, Tails = warm falling — gives each result a distinct feel
    if(result == COIN_HEADS) {
        play_sweep(app, 1000.0f, 1600.0f, 0.3f, 20);
    } else {
        play_sweep(app, 1000.0f, 700.0f, 0.3f, 20);
    }
}
