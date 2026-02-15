#include <nds.h>
#include <stdio.h>
#include <time.h>

#include "app.h"
#include "top_display.h"
#include "ui.h"

static void applyTheme(const Theme *theme) {
    uiApplyTheme(theme);
    topDisplayApplyTheme(theme);
}

static void updateThemeState(AppState *state) {
    state->theme.light = (state->themeIndex & 1) != 0;
}

static void toggleStopwatch(AppState *state) {
    state->stopwatchRunning = !state->stopwatchRunning;
}

static void resetStopwatch(AppState *state) {
    state->stopwatchFrames = 0;
    state->stopwatchRunning = false;
}

static void handleUiAction(AppState *state, UiAction action) {
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

static void handleKeys(AppState *state, int down) {
    if (down & KEY_START) {
        state->themeIndex = (state->themeIndex + 1) & 1;
        updateThemeState(state);
        applyTheme(&state->theme);
    }

    if (down & KEY_SELECT) {
        state->mode = (state->mode == MODE_CLOCK) ? MODE_STOPWATCH : MODE_CLOCK;
    }

    if (state->mode != MODE_STOPWATCH) {
        return;
    }

    if (down & KEY_A) {
        toggleStopwatch(state);
    }
    if (down & KEY_B) {
        resetStopwatch(state);
    }
}

static void handleTouch(AppState *state, int down) {
    touchPosition touch;
    UiAction action;

    if ((down & KEY_TOUCH) == 0) {
        return;
    }

    touchRead(&touch);
    action = uiActionFromTouch(state->mode, touch.px, touch.py);
    handleUiAction(state, action);
}

static void buildTimeText(const AppState *state, char *out, size_t outSize) {
    if (state->mode == MODE_CLOCK) {
        time_t unixTime = time(NULL);
        struct tm *t = localtime(&unixTime);
        snprintf(out, outSize, "%02d:%02d", t->tm_hour, t->tm_min);
        return;
    }

    const u32 totalSeconds = state->stopwatchFrames / 60;
    const u32 h = totalSeconds / 3600;
    const u32 m = (totalSeconds / 60) % 60;
    const u32 s = totalSeconds % 60;
    snprintf(out, outSize, "%02lu:%02lu:%02lu",
             (unsigned long)h,
             (unsigned long)m,
             (unsigned long)s);
}

int appRun(void) {
    PrintConsole bottomScreen;
    AppState state = {0};
    char timeText[16];

    state.mode = MODE_CLOCK;
    state.themeIndex = 0;
    updateThemeState(&state);

    lcdMainOnTop();
    videoSetMode(MODE_0_2D | DISPLAY_SPR_ACTIVE);
    videoSetModeSub(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_SPRITE);
    vramSetBankC(VRAM_C_SUB_BG);

    topDisplayInit();
    uiInit(&bottomScreen);
    applyTheme(&state.theme);

    while (pmMainLoop()) {
        swiWaitForVBlank();
        scanKeys();

        const int down = keysDown();
        handleKeys(&state, down);
        handleTouch(&state, down);

        if (state.stopwatchRunning) {
            state.stopwatchFrames++;
        }

        uiDraw(&state);
        buildTimeText(&state, timeText, sizeof(timeText));
        topDisplayRenderTime(timeText);
        topDisplayUpdate();
    }

    return 0;
}
