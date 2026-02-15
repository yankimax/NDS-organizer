#ifndef ORGANIZER_UI_H
#define ORGANIZER_UI_H

#include "app.h"

typedef struct {
    int x0;
    int y0;
    int x1;
    int y1;
} Rect;

typedef enum {
    UI_ACTION_NONE = 0,
    UI_ACTION_MODE_CLOCK,
    UI_ACTION_MODE_STOPWATCH,
    UI_ACTION_START_STOP,
    UI_ACTION_RESET
} UiAction;

void uiInit(PrintConsole *bottomScreen);
void uiApplyTheme(const Theme *theme);
void uiDraw(const AppState *state);
UiAction uiActionFromTouch(AppMode mode, int px, int py);

#endif
