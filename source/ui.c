#include <nds.h>
#include <stdio.h>
#include <string.h>

#include "font.h"
#include "ui.h"

static const Rect BTN_MODE_CLOCK = {4, 28, 115, 55};
static const Rect BTN_MODE_STOPWATCH = {140, 28, 251, 55};
static const Rect BTN_START_STOP = {10, 92, 82, 119};
static const Rect BTN_RESET = {174, 92, 246, 119};
static const Rect BTN_SETTINGS_LANGUAGE = {8, 52, 247, 79};
static const char SETTINGS_GITHUB_LABEL[] = "github:";
static const char SETTINGS_GITHUB_PATH[] = "yankimax/NDS-organizer";

static PrintConsole *gBottomScreen = NULL;

static bool pointInRect(int x, int y, const Rect *rect) {
    return x >= rect->x0 && x <= rect->x1 && y >= rect->y0 && y <= rect->y1;
}

static int centerCol(const char *text) {
    const int len = (int)strlen(text);
    return (len >= 32) ? 0 : ((32 - len) / 2);
}

static void drawButton(const Rect *rect, const char *label, bool active) {
    const int col = rect->x0 / 8;
    const int row = rect->y0 / 8;
    int widthChars = (rect->x1 - rect->x0 + 1) / 8;
    int labelLen = (int)strlen(label);
    char line[40];

    if (widthChars < 4) {
        widthChars = 4;
    }
    if (widthChars > 38) {
        widthChars = 38;
    }

    memset(line, ' ', (size_t)widthChars);
    line[widthChars] = '\0';
    line[0] = active ? '{' : '[';
    line[widthChars - 1] = active ? '}' : ']';

    if (labelLen > widthChars - 2) {
        labelLen = widthChars - 2;
    }
    memcpy(&line[1 + (widthChars - 2 - labelLen) / 2], label, (size_t)labelLen);
    iprintf("\x1b[%d;%dH%s", row, col, line);
}

static void drawSettingsFooter(const LocalizationLanguage *lang) {
    iprintf("\x1b[18;1H%-31.31s", lang->settingsHintA);
    iprintf("\x1b[19;1H%-31.31s", lang->settingsHintB);
    iprintf("\x1b[20;1H%-31.31s", lang->settingsHintLR);
    iprintf("\x1b[22;1H%-31.31s", SETTINGS_GITHUB_LABEL);
    iprintf("\x1b[23;1H%-31.31s", SETTINGS_GITHUB_PATH);
}

static void drawMainFooter(const LocalizationLanguage *lang) {
    iprintf("\x1b[18;1H%-31.31s", lang->hintStart);
    iprintf("\x1b[19;1H%-31.31s", lang->hintSelect);
    iprintf("\x1b[20;1H%-31.31s", lang->hintA);
    iprintf("\x1b[21;1H%-31.31s", lang->hintB);
    iprintf("\x1b[22;1H%-31.31s", lang->hintLR);
}

void uiInit(PrintConsole *bottomScreen) {
    ConsoleFont font;

    gBottomScreen = bottomScreen;
    consoleInit(gBottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

    font.gfx = (u16 *)fontTiles;
    font.pal = (u16 *)fontPal;
    font.numChars = 224;
    font.numColors = fontPalLen / 2;
    font.bpp = 4;
    font.asciiOffset = 32;
    font.convertSingleColor = false;
    consoleSetFont(gBottomScreen, &font);
}

void uiApplyTheme(const Theme *theme) {
    int i;

    for (i = 0; i < 16; i++) {
        int shade = (i * 31) / 15;
        if (theme->light) {
            shade = 31 - shade;
        }
        BG_PALETTE_SUB[i] = RGB15(shade, shade, shade);
    }

    BG_PALETTE_SUB[0] = theme->light ? RGB15(31, 31, 31) : RGB15(0, 0, 0);
}

bool uiSettingsLanguageHit(int px, int py) {
    return pointInRect(px, py, &BTN_SETTINGS_LANGUAGE);
}

int uiRunLanguageSelection(int languageCount,
                           const char *const *languageNames,
                           int initialSelected,
                           const LocalizationLanguage *lang) {
    int selected = initialSelected;
    int firstVisible = 0;
    const int visibleRows = 16;
    bool canConfirm = false;

    if (lang == NULL || languageNames == NULL || languageCount <= 0) {
        return -1;
    }
    if (selected < 0 || selected >= languageCount) {
        selected = 0;
    }

    consoleSelect(gBottomScreen);

    while (pmMainLoop()) {
        int i;

        swiWaitForVBlank();
        scanKeys();
        {
            const int down = keysDown();
            const int held = keysHeld();

            if ((held & (KEY_A | KEY_START)) == 0) {
                canConfirm = true;
            }
            if (down & KEY_UP) {
                selected = (selected + languageCount - 1) % languageCount;
            }
            if (down & KEY_DOWN) {
                selected = (selected + 1) % languageCount;
            }
            if (canConfirm && (down & (KEY_A | KEY_START))) {
                return selected;
            }
            if (down & KEY_TOUCH) {
                touchPosition touch;
                int row;
                int index;

                touchRead(&touch);
                row = touch.py / 8;
                index = firstVisible + (row - 4);
                if (row >= 4 && row < 4 + visibleRows && index >= 0 && index < languageCount) {
                    return index;
                }
            }
        }

        if (selected < firstVisible) {
            firstVisible = selected;
        } else if (selected >= firstVisible + visibleRows) {
            firstVisible = selected - visibleRows + 1;
        }

        iprintf("\x1b[2J");
        iprintf("\x1b[1;1H%-31.31s", lang->languageSelectTitle);
        iprintf("\x1b[2;1H%-31.31s", lang->languageSelectHint);

        for (i = 0; i < visibleRows; i++) {
            const int index = firstVisible + i;
            if (index >= languageCount) {
                break;
            }

            iprintf("\x1b[%d;1H%c %-29.29s",
                   4 + i,
                   (index == selected) ? '>' : ' ',
                   languageNames[index]);
        }
    }

    return selected;
}

void uiDraw(const AppState *state, const LocalizationLanguage *lang) {
    if (lang == NULL) {
        return;
    }

    consoleSelect(gBottomScreen);
    iprintf("\x1b[2J");

    if (state->mode == MODE_SETTINGS) {
        char languageLine[48];

        iprintf("\x1b[1;%dH%s", centerCol(lang->settingsTitle), lang->settingsTitle);
        snprintf(languageLine, sizeof(languageLine), "%.20s: %.24s", lang->settingsLanguage, lang->name);
        drawButton(&BTN_SETTINGS_LANGUAGE, languageLine, true);
        drawSettingsFooter(lang);
        return;
    }

    iprintf("\x1b[1;%dH%s", centerCol(lang->modeTitle), lang->modeTitle);
    drawButton(&BTN_MODE_CLOCK, lang->modeClock, state->mode == MODE_CLOCK);
    drawButton(&BTN_MODE_STOPWATCH, lang->modeStopwatch, state->mode == MODE_STOPWATCH);

    if (state->mode == MODE_STOPWATCH) {
        const char *status = state->stopwatchRunning ? lang->statusRun : lang->statusPause;

        iprintf("\x1b[9;%dH%s", centerCol(lang->controlTitle), lang->controlTitle);
        drawButton(&BTN_START_STOP,
                   state->stopwatchRunning ? lang->actionStop : lang->actionStart,
                   state->stopwatchRunning);
        drawButton(&BTN_RESET, lang->actionReset, false);
        iprintf("\x1b[13;%dH%s", centerCol(status), status);
    }

    drawMainFooter(lang);
}

UiAction uiActionFromTouch(AppMode mode, int px, int py) {
    if (pointInRect(px, py, &BTN_MODE_CLOCK)) {
        return UI_ACTION_MODE_CLOCK;
    }
    if (pointInRect(px, py, &BTN_MODE_STOPWATCH)) {
        return UI_ACTION_MODE_STOPWATCH;
    }
    if (mode != MODE_STOPWATCH) {
        return UI_ACTION_NONE;
    }
    if (pointInRect(px, py, &BTN_START_STOP)) {
        return UI_ACTION_START_STOP;
    }
    if (pointInRect(px, py, &BTN_RESET)) {
        return UI_ACTION_RESET;
    }
    return UI_ACTION_NONE;
}
