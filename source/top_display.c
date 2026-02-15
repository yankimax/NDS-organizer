#include <nds.h>
#include <stdio.h>
#include <string.h>

#include "digits.h"
#include "top_display.h"

enum {
    MAX_TIME_CHARS = 8,
    DIGIT_TILE_SIZE = (32 * 32) / 4
};

static u16 *digitSlots[MAX_TIME_CHARS];
static int slotGlyph[MAX_TIME_CHARS];

static int glyphIndexFromChar(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c == ':') {
        return 10;
    }
    return 10;
}

static int glyphAdvance(char c) {
    if (c == ':') {
        return 20;
    }
    return 26;
}

static void copyGlyphToSlot(int slot, int glyphIndex) {
    const u16 *src = (const u16 *)digitsTiles + (glyphIndex * DIGIT_TILE_SIZE);
    memcpy(digitSlots[slot], src, DIGIT_TILE_SIZE * sizeof(u16));
    slotGlyph[slot] = glyphIndex;
}

void topDisplayInit(void) {
    oamInit(&oamMain, SpriteMapping_1D_32, false);
    for (int i = 0; i < MAX_TIME_CHARS; i++) {
        digitSlots[i] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
        slotGlyph[i] = -1;
        oamSetHidden(&oamMain, i, true);
    }
}

void topDisplayApplyTheme(const Theme *theme) {
    const u16 fg = theme->light ? RGB15(0, 0, 0) : RGB15(31, 31, 31);
    const u16 bg = theme->light ? RGB15(31, 31, 31) : RGB15(0, 0, 0);

    for (int i = 0; i < 16; i++) {
        SPRITE_PALETTE[i] = (i == 0) ? RGB15(0, 0, 0) : fg;
    }

    BG_PALETTE[0] = bg;
}

void topDisplayRenderTime(const char *timeText) {
    const int len = (int)strlen(timeText);
    const int spacing = 1;
    int totalWidth = 0;
    for (int i = 0; i < len; i++) {
        totalWidth += glyphAdvance(timeText[i]);
    }
    if (len > 1) {
        totalWidth += (len - 1) * spacing;
    }
    const int startX = (256 - totalWidth) / 2;
    const int y = 80;
    int x = startX;

    for (int i = 0; i < MAX_TIME_CHARS; i++) {
        if (i >= len) {
            oamSetHidden(&oamMain, i, true);
            continue;
        }

        const int glyph = glyphIndexFromChar(timeText[i]);
        if (slotGlyph[i] != glyph) {
            copyGlyphToSlot(i, glyph);
        }

        oamSet(&oamMain,
               i,
               x,
               y,
               0,
               0,
               SpriteSize_32x32,
               SpriteColorFormat_16Color,
               digitSlots[i],
               -1,
               false,
               false,
               false,
               false,
               false);

        x += glyphAdvance(timeText[i]) + spacing;
    }
}

void topDisplayUpdate(void) {
    oamUpdate(&oamMain);
}
