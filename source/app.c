#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "app.h"
#include "localization.h"
#include "save.h"
#include "top_display.h"
#include "ui.h"

#define TOP_FATAL_LINE_1 "Localization load failed"
#define TOP_FATAL_LINE_2 "No valid locales in NitroFS"
#define TOP_FATAL_LINE_3 "Rebuild and restart ROM"

static void applyTheme(const Theme *theme) {
    uiApplyTheme(theme);
    topDisplayApplyTheme(theme);
}

static void toggleStopwatch(AppState *state) {
    state->stopwatchRunning = !state->stopwatchRunning;
}

static void resetStopwatch(AppState *state) {
    state->stopwatchFrames = 0;
    state->stopwatchRunning = false;
}

static void enterSettingsMode(AppState *state) {
    state->returnMode = state->mode;
    state->mode = MODE_SETTINGS;
}

static void exitSettingsMode(AppState *state) {
    state->mode = (state->returnMode == MODE_SETTINGS) ? MODE_CLOCK : state->returnMode;
}

static void applyTouchAction(AppState *state, UiAction action) {
    switch (action) {
        case UI_ACTION_MODE_CLOCK:
            state->mode = MODE_CLOCK;
            break;
        case UI_ACTION_MODE_STOPWATCH:
            state->mode = MODE_STOPWATCH;
            break;
        case UI_ACTION_START_STOP:
            toggleStopwatch(state);
            break;
        case UI_ACTION_RESET:
            resetStopwatch(state);
            break;
        case UI_ACTION_NONE:
        default:
            break;
    }
}

static bool fillLanguageNames(const char **namesOut, int capacity, int *countOut) {
    const int languageCount = localizationCount();
    int i;

    if (namesOut == NULL || countOut == NULL || capacity <= 0 || languageCount <= 0) {
        return false;
    }

    for (i = 0; i < languageCount && i < capacity; i++) {
        const LocalizationLanguage *lang = localizationGet(i);
        if (lang == NULL) {
            return false;
        }
        namesOut[i] = lang->name;
    }

    *countOut = languageCount;
    return true;
}

static bool selectLanguageAndPersist(void) {
    const LocalizationLanguage *currentLanguage = localizationCurrent();
    const int initialSelected = localizationCurrentIndex();
    const char *languageNames[LOCALIZATION_MAX_LANGUAGES];
    int languageCount;
    int selected;

    if (currentLanguage == NULL) {
        return false;
    }
    if (!fillLanguageNames(languageNames, LOCALIZATION_MAX_LANGUAGES, &languageCount)) {
        return false;
    }

    selected = uiRunLanguageSelection(languageCount,
                                      languageNames,
                                      initialSelected,
                                      currentLanguage);
    if (selected < 0 || !localizationSetCurrentByIndex(selected)) {
        return false;
    }

    saveStoreLanguage(localizationCurrentId());
    return true;
}

static bool setupLanguage(void) {
    char savedLanguageId[LOCALIZATION_ID_SIZE];

    if (!localizationInit()) {
        return false;
    }

    if (saveLoadLanguage(savedLanguageId, sizeof(savedLanguageId))) {
        if (localizationSetCurrentById(savedLanguageId)) {
            return true;
        }
    }

    return selectLanguageAndPersist();
}

static void handleSettingsCombo(AppState *state, int held) {
    const bool comboHeld = ((held & KEY_L) != 0) && ((held & KEY_R) != 0);

    if (comboHeld && !state->settingsComboLatch) {
        if (state->mode == MODE_SETTINGS) {
            exitSettingsMode(state);
        } else {
            enterSettingsMode(state);
        }
    }

    state->settingsComboLatch = comboHeld;
}

static void handleSettingsInput(AppState *state, int down) {
    if (down & KEY_A) {
        selectLanguageAndPersist();
    }
    if (down & KEY_B) {
        exitSettingsMode(state);
    }
}

static void handleMainInput(AppState *state, int down) {
    if (down & KEY_SELECT) {
        state->mode = (state->mode == MODE_CLOCK) ? MODE_STOPWATCH : MODE_CLOCK;
    }

    if (state->mode == MODE_STOPWATCH) {
        if (down & KEY_A) {
            toggleStopwatch(state);
        }
        if (down & KEY_B) {
            resetStopwatch(state);
        }
    }
}

static void handleKeys(AppState *state, int down, int held) {
    handleSettingsCombo(state, held);

    if (down & KEY_START) {
        state->theme.light = !state->theme.light;
        applyTheme(&state->theme);
    }

    if (state->mode == MODE_SETTINGS) {
        handleSettingsInput(state, down);
    } else {
        handleMainInput(state, down);
    }
}

static void handleTouch(AppState *state, int down) {
    touchPosition touch;

    if ((down & KEY_TOUCH) == 0) {
        return;
    }

    touchRead(&touch);

    if (state->mode == MODE_SETTINGS) {
        if (uiSettingsLanguageHit(touch.px, touch.py)) {
            selectLanguageAndPersist();
        }
        return;
    }

    applyTouchAction(state, uiActionFromTouch(state->mode, touch.px, touch.py));
}

static void buildTimeText(const AppState *state, char *out, size_t outSize) {
    if (state->mode != MODE_STOPWATCH) {
        const time_t unixTime = time(NULL);
        const struct tm *t = localtime(&unixTime);

        snprintf(out, outSize, "%02d:%02d", t->tm_hour, t->tm_min);
        return;
    }

    {
        const u32 totalSeconds = state->stopwatchFrames / 60;
        const u32 h = totalSeconds / 3600;
        const u32 m = (totalSeconds / 60) % 60;
        const u32 s = totalSeconds % 60;

        snprintf(out,
                 outSize,
                 "%02lu:%02lu:%02lu",
                 (unsigned long)h,
                 (unsigned long)m,
                 (unsigned long)s);
    }
}

static void drawFatalLocalizationError(PrintConsole *screen) {
    consoleSelect(screen);
    iprintf("\x1b[2J");
    iprintf("\x1b[5;2H%s", TOP_FATAL_LINE_1);
    iprintf("\x1b[7;2H%s", TOP_FATAL_LINE_2);
    iprintf("\x1b[9;2H%s", TOP_FATAL_LINE_3);
}

static void initVideo(void) {
    lcdMainOnTop();
    videoSetMode(MODE_0_2D | DISPLAY_SPR_ACTIVE);
    videoSetModeSub(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_SPRITE);
    vramSetBankC(VRAM_C_SUB_BG);
}

static void initAppState(AppState *state) {
    memset(state, 0, sizeof(*state));
    state->mode = MODE_CLOCK;
    state->returnMode = MODE_CLOCK;
    state->theme.light = false;
}

int appRun(void) {
    PrintConsole bottomScreen;
    AppState state;
    char timeText[16];

    initAppState(&state);
    initVideo();

    topDisplayInit();
    uiInit(&bottomScreen);
    applyTheme(&state.theme);

    if (!setupLanguage()) {
        drawFatalLocalizationError(&bottomScreen);
        while (pmMainLoop()) {
            swiWaitForVBlank();
        }
        return 1;
    }

    while (pmMainLoop()) {
        const LocalizationLanguage *currentLanguage;
        int down;
        int held;

        swiWaitForVBlank();
        scanKeys();

        down = keysDown();
        held = keysHeld();
        handleKeys(&state, down, held);
        handleTouch(&state, down);

        if (state.mode == MODE_STOPWATCH && state.stopwatchRunning) {
            state.stopwatchFrames++;
        }

        currentLanguage = localizationCurrent();
        if (currentLanguage == NULL) {
            drawFatalLocalizationError(&bottomScreen);
            continue;
        }

        uiDraw(&state, currentLanguage);
        buildTimeText(&state, timeText, sizeof(timeText));
        topDisplayRenderTime(timeText);
        topDisplayUpdate();
    }

    return 0;
}
