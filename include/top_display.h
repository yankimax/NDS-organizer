#ifndef ORGANIZER_TOP_DISPLAY_H
#define ORGANIZER_TOP_DISPLAY_H

#include "app.h"

void topDisplayInit(void);
void topDisplayApplyTheme(const Theme *theme);
void topDisplayRenderTime(const char *timeText);
void topDisplayUpdate(void);

#endif
