/**
 * Sound effects via the Flipper Zero speaker.
 *
 * All functions no-op when sound is disabled in settings or if the speaker
 * cannot be acquired (another app is using it). Keep tones short — each
 * call blocks for the duration of the effect, so budget <30ms per call.
 */
#pragma once
#include "app.h"

void sound_menu_click(App* app);   // UI navigation
void sound_menu_select(App* app);  // UI confirmation
void sound_flip_launch(App* app);  // coin lifts off
void sound_flip_land(App* app);    // coin hits surface
void sound_reveal(App* app, uint8_t result);  // result shown
