#ifndef ORGANIZER_APP_H
#define ORGANIZER_APP_H

#include <stdbool.h>
#include <nds.h>

typedef enum {
    MODE_CLOCK = 0,
    MODE_STOPWATCH = 1
} AppMode;

typedef struct {
    bool light;
} Theme;

typedef struct {
    AppMode mode;
    bool stopwatchRunning;
    u32 stopwatchFrames;
    int themeIndex;
    Theme theme;
} AppState;

int appRun(void);

#endif
