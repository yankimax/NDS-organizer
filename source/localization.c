#include <filesystem.h>
#include <dirent.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "localization.h"
#include "text_utils.h"

#define LOCALES_DIRECTORY "nitro:/locales"
#define MAX_LOCALE_LINE_LENGTH 192

typedef struct {
    const char *key;
    size_t offset;
    size_t size;
} LocalizationFieldSpec;

#define FIELD_SPEC_ENTRY(key, member, size) \
    { key, offsetof(LocalizationLanguage, member), sizeof(((LocalizationLanguage *)0)->member) },

static const LocalizationFieldSpec FIELD_SPECS[] = {
    LOCALIZATION_FIELD_MAP(FIELD_SPEC_ENTRY)
};

#undef FIELD_SPEC_ENTRY

#define LOCALIZATION_FIELD_COUNT ((int)(sizeof(FIELD_SPECS) / sizeof(FIELD_SPECS[0])))

static LocalizationLanguage gLanguages[LOCALIZATION_MAX_LANGUAGES];
static int gLanguageCount = 0;
static int gCurrentLanguageIndex = 0;
static bool gInitialized = false;

static int unicodeToCp1251(unsigned codepoint) {
    if (codepoint < 0x80U) {
        return (int)codepoint;
    }
    if (codepoint >= 0x0410U && codepoint <= 0x044FU) {
        return (int)(codepoint - 0x350U);
    }

    switch (codepoint) {
        case 0x0401U: return 0xA8;
        case 0x0451U: return 0xB8;
        case 0x0402U: return 0x80;
        case 0x0452U: return 0x90;
        case 0x0404U: return 0xAA;
        case 0x0454U: return 0xBA;
        case 0x0406U: return 0xB2;
        case 0x0456U: return 0xB3;
        case 0x0407U: return 0xAF;
        case 0x0457U: return 0xBF;
        case 0x0490U: return 0xA5;
        case 0x0491U: return 0xB4;
        case 0x0408U: return 0xA3;
        case 0x0458U: return 0xBC;
        case 0x0409U: return 0x8A;
        case 0x0459U: return 0x9A;
        case 0x040AU: return 0x8C;
        case 0x045AU: return 0x9C;
        case 0x040BU: return 0x8E;
        case 0x045BU: return 0x9E;
        case 0x040FU: return 0x8F;
        case 0x045FU: return 0x9F;
        default: return -1;
    }
}

static void copyText(char *dst, size_t dstSize, const char *src) {
    size_t out = 0;
    size_t i = 0;

    if (dst == NULL || dstSize == 0) {
        return;
    }
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    while (src[i] != '\0' && out + 1 < dstSize) {
        const unsigned char b0 = (unsigned char)src[i];
        unsigned codepoint;
        int cp;

        if (b0 < 0x80U) {
            codepoint = b0;
            i += 1;
        } else if ((b0 & 0xE0U) == 0xC0U) {
            const unsigned char b1 = (unsigned char)src[i + 1];
            if ((b1 & 0xC0U) == 0x80U) {
                codepoint = ((unsigned)(b0 & 0x1FU) << 6) | (unsigned)(b1 & 0x3FU);
                i += 2;
            } else {
                codepoint = b0;
                i += 1;
            }
        } else if ((b0 & 0xF0U) == 0xE0U) {
            const unsigned char b1 = (unsigned char)src[i + 1];
            const unsigned char b2 = (unsigned char)src[i + 2];
            if ((b1 & 0xC0U) == 0x80U && (b2 & 0xC0U) == 0x80U) {
                codepoint = ((unsigned)(b0 & 0x0FU) << 12) |
                            ((unsigned)(b1 & 0x3FU) << 6) |
                            (unsigned)(b2 & 0x3FU);
                i += 3;
            } else {
                codepoint = b0;
                i += 1;
            }
        } else {
            codepoint = b0;
            i += 1;
        }

        cp = unicodeToCp1251(codepoint);
        if (cp < 0) {
            cp = (codepoint < 0x100U) ? (int)codepoint : '?';
        }
        dst[out++] = (char)cp;
    }

    dst[out] = '\0';
}

static void trimRight(char *text) {
    int i;

    if (text == NULL) {
        return;
    }

    i = (int)strlen(text) - 1;
    while (i >= 0 && (text[i] == ' ' || text[i] == '\t' || text[i] == '\r' || text[i] == '\n')) {
        text[i] = '\0';
        i--;
    }
}

static char *trimLeft(char *text) {
    while (*text == ' ' || *text == '\t') {
        text++;
    }
    return text;
}

static bool hasLanguageExtension(const char *filename) {
    size_t len;

    if (filename == NULL) {
        return false;
    }

    len = strlen(filename);
    return len > 4 && strcmp(filename + len - 4, ".lng") == 0;
}

static void normalizeIdFromFilename(char *idOut, size_t idOutSize, const char *filename) {
    size_t i;
    size_t out = 0;

    if (idOut == NULL || idOutSize == 0) {
        return;
    }

    idOut[0] = '\0';
    if (filename == NULL) {
        return;
    }

    for (i = 0; filename[i] != '\0' && filename[i] != '.' && out + 1 < idOutSize; i++) {
        const unsigned char c = (unsigned char)filename[i];

        if (isalnum(c)) {
            idOut[out++] = (char)tolower(c);
        } else if (c == '-' || c == '_') {
            idOut[out++] = '_';
        }
    }
    idOut[out] = '\0';
}

static int findLanguageIndexById(const char *id) {
    int i;

    if (id == NULL || id[0] == '\0') {
        return -1;
    }

    for (i = 0; i < gLanguageCount; i++) {
        if (strcmp(gLanguages[i].id, id) == 0) {
            return i;
        }
    }
    return -1;
}

static int findFieldIndex(const char *key) {
    int i;

    for (i = 0; i < LOCALIZATION_FIELD_COUNT; i++) {
        if (strcmp(FIELD_SPECS[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

static bool parseLanguageFile(const char *path,
                              const char *languageId,
                              LocalizationLanguage *outLanguage) {
    FILE *file;
    LocalizationLanguage parsed;
    char line[MAX_LOCALE_LINE_LENGTH];
    bool fieldSeen[LOCALIZATION_FIELD_COUNT] = {false};
    int seenCount = 0;

    if (path == NULL || languageId == NULL || outLanguage == NULL) {
        return false;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        return false;
    }

    memset(&parsed, 0, sizeof(parsed));
    copyStringSafe(parsed.id, sizeof(parsed.id), languageId);

    while (fgets(line, sizeof(line), file) != NULL) {
        char *key;
        char *value;
        char *eq;
        int fieldIndex;
        char *dst;

        trimRight(line);
        key = trimLeft(line);

        if (key[0] == '\0' || key[0] == '#' || key[0] == ';') {
            continue;
        }

        eq = strchr(key, '=');
        if (eq == NULL) {
            continue;
        }

        *eq = '\0';
        value = trimLeft(eq + 1);
        trimRight(key);
        trimRight(value);

        fieldIndex = findFieldIndex(key);
        if (fieldIndex < 0) {
            continue;
        }

        dst = ((char *)&parsed) + FIELD_SPECS[fieldIndex].offset;
        copyText(dst, FIELD_SPECS[fieldIndex].size, value);

        if (!fieldSeen[fieldIndex]) {
            fieldSeen[fieldIndex] = true;
            seenCount++;
        }
    }

    fclose(file);

    if (seenCount != LOCALIZATION_FIELD_COUNT) {
        return false;
    }

    *outLanguage = parsed;
    return true;
}

static void addOrReplaceLanguage(const LocalizationLanguage *language) {
    const int existingIndex = findLanguageIndexById(language->id);

    if (existingIndex >= 0) {
        gLanguages[existingIndex] = *language;
        return;
    }

    if (gLanguageCount < LOCALIZATION_MAX_LANGUAGES) {
        gLanguages[gLanguageCount++] = *language;
    }
}

static bool loadLanguagesFromNitroFS(void) {
    DIR *dir;
    struct dirent *entry;

    if (!nitroFSInit(NULL)) {
        return false;
    }

    dir = opendir(LOCALES_DIRECTORY);
    if (dir == NULL) {
        return false;
    }

    while ((entry = readdir(dir)) != NULL) {
        char languageId[LOCALIZATION_ID_SIZE];
        char path[160];
        LocalizationLanguage parsed;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (!hasLanguageExtension(entry->d_name)) {
            continue;
        }

        normalizeIdFromFilename(languageId, sizeof(languageId), entry->d_name);
        if (languageId[0] == '\0') {
            continue;
        }

        if (snprintf(path, sizeof(path), "%s/%s", LOCALES_DIRECTORY, entry->d_name) >= (int)sizeof(path)) {
            continue;
        }

        if (!parseLanguageFile(path, languageId, &parsed)) {
            continue;
        }

        addOrReplaceLanguage(&parsed);
    }

    closedir(dir);
    return true;
}

static void sortLanguagesByName(void) {
    int i;
    int j;

    for (i = 0; i < gLanguageCount - 1; i++) {
        for (j = i + 1; j < gLanguageCount; j++) {
            if (strcmp(gLanguages[i].name, gLanguages[j].name) > 0) {
                LocalizationLanguage tmp = gLanguages[i];
                gLanguages[i] = gLanguages[j];
                gLanguages[j] = tmp;
            }
        }
    }
}

bool localizationInit(void) {
    gLanguageCount = 0;
    gCurrentLanguageIndex = 0;
    gInitialized = false;
    memset(gLanguages, 0, sizeof(gLanguages));

    if (!loadLanguagesFromNitroFS()) {
        return false;
    }

    sortLanguagesByName();
    if (gLanguageCount <= 0) {
        return false;
    }

    gInitialized = true;
    return true;
}

int localizationCount(void) {
    return gInitialized ? gLanguageCount : 0;
}

const LocalizationLanguage *localizationGet(int index) {
    if (!gInitialized || index < 0 || index >= gLanguageCount) {
        return NULL;
    }
    return &gLanguages[index];
}

const LocalizationLanguage *localizationCurrent(void) {
    if (!gInitialized || gLanguageCount <= 0) {
        return NULL;
    }
    return &gLanguages[gCurrentLanguageIndex];
}

int localizationCurrentIndex(void) {
    if (!gInitialized || gLanguageCount <= 0) {
        return -1;
    }
    return gCurrentLanguageIndex;
}

const char *localizationCurrentId(void) {
    const LocalizationLanguage *current = localizationCurrent();
    return (current != NULL) ? current->id : "";
}

bool localizationSetCurrentByIndex(int index) {
    if (!gInitialized || index < 0 || index >= gLanguageCount) {
        return false;
    }

    gCurrentLanguageIndex = index;
    return true;
}

bool localizationSetCurrentById(const char *languageId) {
    const int index = findLanguageIndexById(languageId);

    if (!gInitialized || index < 0) {
        return false;
    }

    gCurrentLanguageIndex = index;
    return true;
}
