#ifndef ORGANIZER_LOCALIZATION_H
#define ORGANIZER_LOCALIZATION_H

#include <stdbool.h>

#define LOCALIZATION_MAX_LANGUAGES 32
#define LOCALIZATION_ID_SIZE 24

/*
 * Single source of truth for all localization keys and text field sizes.
 * To add a new localized string, append one line here.
 */
#define LOCALIZATION_FIELD_MAP(X) \
    X("name", name, 32) \
    X("mode_title", modeTitle, 32) \
    X("mode_clock", modeClock, 32) \
    X("mode_stopwatch", modeStopwatch, 32) \
    X("control_title", controlTitle, 32) \
    X("action_start", actionStart, 32) \
    X("action_stop", actionStop, 32) \
    X("action_reset", actionReset, 32) \
    X("status_run", statusRun, 32) \
    X("status_pause", statusPause, 32) \
    X("settings_title", settingsTitle, 32) \
    X("settings_language", settingsLanguage, 24) \
    X("settings_hint_a", settingsHintA, 64) \
    X("settings_hint_b", settingsHintB, 64) \
    X("settings_hint_lr", settingsHintLR, 64) \
    X("language_select_title", languageSelectTitle, 32) \
    X("language_select_hint", languageSelectHint, 64) \
    X("hint_start", hintStart, 64) \
    X("hint_select", hintSelect, 64) \
    X("hint_a", hintA, 64) \
    X("hint_b", hintB, 64) \
    X("hint_lr", hintLR, 64)

typedef struct {
    char id[LOCALIZATION_ID_SIZE];
#define LOCALIZATION_DECLARE_FIELD(key, member, size) char member[size];
    LOCALIZATION_FIELD_MAP(LOCALIZATION_DECLARE_FIELD)
#undef LOCALIZATION_DECLARE_FIELD
} LocalizationLanguage;

bool localizationInit(void);
int localizationCount(void);
const LocalizationLanguage *localizationGet(int index);
const LocalizationLanguage *localizationCurrent(void);
int localizationCurrentIndex(void);
const char *localizationCurrentId(void);
bool localizationSetCurrentByIndex(int index);
bool localizationSetCurrentById(const char *languageId);

#endif
