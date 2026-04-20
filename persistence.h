/**
 * Persistent storage — stats and settings survive app restarts.
 *
 * Writes a single binary blob to /ext/apps_data/flipp_coin/state.bin.
 * Uses a magic number + version byte for forward-compatibility; if the
 * file is missing, corrupted, or from an incompatible version, the load
 * silently fails and the app starts with defaults.
 */
#pragma once
#include "app.h"

// Load persisted state from SD card. Safe to call at startup — missing
// or corrupt files are handled silently (app starts with defaults).
void persistence_load(App* app);

// Save state to SD card. Called after each flip and when settings change.
void persistence_save(App* app);
