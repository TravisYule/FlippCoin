#include "persistence.h"

#define SAVE_DIR  "/ext/apps_data/flipp_coin"
#define SAVE_PATH SAVE_DIR "/state.bin"

#define SAVE_MAGIC   0xF11CC01Du
#define SAVE_VERSION 1u

// On-disk format — fixed size so version compat is trivial.
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t total;
    uint32_t heads;
    uint32_t tails;
    uint16_t best_streak;
    uint8_t  best_side;
    uint8_t  haptic_enabled;
    uint8_t  sound_enabled;
    uint8_t  _reserved[7]; // pad to 32 bytes for future-proofing
} SavedState;

void persistence_load(App* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if(storage_file_open(file, SAVE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        SavedState s;
        uint16_t read = storage_file_read(file, &s, sizeof(s));
        if(read == sizeof(s) && s.magic == SAVE_MAGIC && s.version == SAVE_VERSION) {
            app->total = s.total;
            app->heads = s.heads;
            app->tails = s.tails;
            app->best_streak = s.best_streak;
            app->best_side = s.best_side ? s.best_side : COIN_HEADS;
            app->haptic_enabled = s.haptic_enabled != 0;
            app->sound_enabled = s.sound_enabled != 0;
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
            ._reserved = {0},
        };
        storage_file_write(file, &s, sizeof(s));
        storage_file_close(file);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}
