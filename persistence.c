#include "persistence.h"

#define SAVE_DIR  "/ext/apps_data/flipp_coin"
#define SAVE_PATH SAVE_DIR "/state.bin"

#define SAVE_MAGIC   0xF11CC01Du
#define SAVE_VERSION 3u  // v1: base · v2: +achievements · v3: +history

// On-disk format — fixed 32 bytes so version compat is trivial.
//   v1 → v2: `achievements` claims what was `_reserved[0..1]`.
//   v2 → v3: `history_count` claims `_pad`; `history` claims `_reserved[4]`.
// Old saves (bytes 25-31 zeroed in v1 and v2) naturally load as
// history_count=0, history=0 under the v3 layout. No migration needed.
typedef struct {
    uint32_t magic;          // 0
    uint32_t version;        // 4
    uint32_t total;          // 8
    uint32_t heads;          // 12
    uint32_t tails;          // 16
    uint16_t best_streak;    // 20
    uint8_t  best_side;      // 22
    uint8_t  haptic_enabled; // 23
    uint8_t  sound_enabled;  // 24
    uint8_t  history_count;  // 25 (was _pad in v2)
    uint16_t achievements;   // 26
    uint32_t history;        // 28 (was _reserved[4] in v2)
} SavedState;                 // 32 bytes total

void persistence_load(App* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if(storage_file_open(file, SAVE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        SavedState s;
        uint16_t read = storage_file_read(file, &s, sizeof(s));
        // Accept v1, v2, or v3 — old saves naturally zero the new fields
        if(read == sizeof(s) && s.magic == SAVE_MAGIC &&
           (s.version >= 1u && s.version <= SAVE_VERSION)) {
            app->total = s.total;
            app->heads = s.heads;
            app->tails = s.tails;
            app->best_streak = s.best_streak;
            app->best_side = s.best_side ? s.best_side : COIN_HEADS;
            app->haptic_enabled = s.haptic_enabled != 0;
            app->sound_enabled = s.sound_enabled != 0;
            app->achievements = (s.version >= 2u) ? s.achievements : 0;
            if(s.version >= 3u) {
                app->history = s.history;
                app->history_count = (s.history_count <= 32) ? s.history_count : 0;
            }
        }
        storage_file_close(file);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

void persistence_save(App* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, SAVE_DIR);

    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, SAVE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        SavedState s = {
            .magic = SAVE_MAGIC,
            .version = SAVE_VERSION,
            .total = app->total,
            .heads = app->heads,
            .tails = app->tails,
            .best_streak = app->best_streak,
            .best_side = app->best_side,
            .haptic_enabled = app->haptic_enabled ? 1 : 0,
            .sound_enabled = app->sound_enabled ? 1 : 0,
            .history_count = app->history_count,
            .achievements = app->achievements,
            .history = app->history,
        };
        storage_file_write(file, &s, sizeof(s));
        storage_file_close(file);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}
