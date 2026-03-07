#ifndef ORGANIZER_TEXT_UTILS_H
#define ORGANIZER_TEXT_UTILS_H

#include <stddef.h>

static inline void copyStringSafe(char *dst, size_t dstSize, const char *src) {
    size_t i;

    if (dst == NULL || dstSize == 0) {
        return;
    }
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    for (i = 0; i + 1 < dstSize && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

#endif
