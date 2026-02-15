#include <nds.h>
#include <stdio.h>
#include <string.h>

#include "font.h"
#include "ui.h"

static const Rect BTN_MODE_CLOCK = {4, 28, 115, 55};
static const Rect BTN_MODE_STOPWATCH = {140, 28, 251, 55};
static const Rect BTN_START_STOP = {10, 92, 82, 119};
static const Rect BTN_RESET = {174, 92, 246, 119};
static const char TXT_MODE[] = "\xD0\xC5\xC6\xC8\xCC";
static const char TXT_CLOCK[] = "\xD7\xC0\xD1\xDB";
static const char TXT_STOPWATCH[] = "\xD1\xC5\xCA\xD3\xCD\xC4\xCE\xCC\xC5\xD0";
static const char TXT_CONTROL[] = "\xD3\xCF\xD0\xC0\xC2\xCB\xC5\xCD\xC8\xC5";
static const char TXT_START[] = "\xD1\xD2\xC0\xD0\xD2";
static const char TXT_STOP[] = "\xD1\xD2\xCE\xCF";
static const char TXT_RESET[] = "\xD1\xC1\xD0\xCE\xD1";
static const char TXT_RUN[] = "\xD0\xC0\xC1\xCE\xD2\xC0\xC5\xD2";
static const char TXT_PAUSE[] = "\xCF\xC0\xD3\xC7\xC0";
static const char TXT_HINT_START[] = "\x53\x54\x41\x52\x54\x3A\x20\xF1\xEC\xE5\xED\xE0\x20\xF2\xE5\xEC\xFB";
static const char TXT_HINT_SELECT[] = "\x53\x45\x4C\x45\x43\x54\x3A\x20\xF0\xE5\xE6\xE8\xEC";
static const char TXT_HINT_A[] = "\x41\x3A\x20\xF1\xF2\xE0\xF0\xF2\x2F\xF1\xF2\xEE\xEF";
static const char TXT_HINT_B[] = "\x42\x3A\x20\xF1\xE1\xF0\xEE\xF1";

static PrintConsole *gBottomScreen = NULL;

static bool pointInRect(int x, int y, const Rect *r) {
    return x >= r->x0 && x <= r->x1 && y >= r->y0 && y <= r->y1;
}

static int centerCol(const char *s) {
    const int len = (int)strlen(s);
    return (len >= 32) ? 0 : ((32 - len) / 2);
}

static void drawButton(const Rect *r, const char *label, bool active) {
    const int col = r->x0 / 8;
    const int row = r->y0 / 8;
    int widthChars = (r->x1 - r->x0 + 1) / 8;
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
    for (int i = 0; i < 16; i++) {
        int shade = (i * 31) / 15;
        if (theme->light) {
            shade = 31 - shade;
        }
        BG_PALETTE_SUB[i] = RGB15(shade, shade, shade);
    }
    BG_PALETTE_SUB[0] = theme->light ? RGB15(31, 31, 31) : RGB15(0, 0, 0);
}

void uiDraw(const AppState *state) {
    consoleSelect(gBottomScreen);
    iprintf("\x1b[2J");

    iprintf("\x1b[1;%dH%s", centerCol(TXT_MODE), TXT_MODE);
    drawButton(&BTN_MODE_CLOCK, TXT_CLOCK, state->mode == MODE_CLOCK);
    drawButton(&BTN_MODE_STOPWATCH, TXT_STOPWATCH, state->mode == MODE_STOPWATCH);

    if (state->mode == MODE_STOPWATCH) {
        const char *status = state->stopwatchRunning ? TXT_RUN : TXT_PAUSE;
        iprintf("\x1b[9;%dH%s", centerCol(TXT_CONTROL), TXT_CONTROL);
        drawButton(&BTN_START_STOP, state->stopwatchRunning ? TXT_STOP : TXT_START, state->stopwatchRunning);
        drawButton(&BTN_RESET, TXT_RESET, false);
        iprintf("\x1b[13;%dH%s", centerCol(status), status);
    }

    iprintf("\x1b[18;1H%s", TXT_HINT_START);
    iprintf("\x1b[19;1H%s", TXT_HINT_SELECT);
    iprintf("\x1b[20;1H%s", TXT_HINT_A);
    iprintf("\x1b[21;1H%s", TXT_HINT_B);
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
