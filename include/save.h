#ifndef ORGANIZER_SAVE_H
#define ORGANIZER_SAVE_H

#include <stdbool.h>
#include <stddef.h>

bool saveLoadLanguage(char *languageIdOut, size_t languageIdOutSize);
bool saveStoreLanguage(const char *languageId);

#endif
