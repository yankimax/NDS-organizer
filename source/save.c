#include <fat.h>
#include <nds.h>
#include <stdio.h>
#include <string.h>

#include "save.h"
#include "text_utils.h"

enum {
    SAVE_MAGIC = 0x4F524732u,
    SAVE_VERSION = 1u
};

typedef struct {
    u32 magic;
    u32 version;
    char languageId[24];
} SaveData;

static const char SAVE_PATH[] = "organizer.sav";
static bool gFatChecked = false;
static bool gFatReady = false;

static bool ensureFat(void) {
    if (gFatChecked) {
        return gFatReady;
    }

    gFatChecked = true;
    gFatReady = fatInitDefault();
    return gFatReady;
}

static bool isValidSave(const SaveData *data) {
    return data != NULL &&
           data->magic == SAVE_MAGIC &&
           data->version == SAVE_VERSION;
}

static bool readSave(SaveData *data) {
    FILE *file;
    size_t readCount;

    if (data == NULL) {
        return false;
    }

    file = fopen(SAVE_PATH, "rb");
    if (file == NULL) {
        return false;
    }

    readCount = fread(data, sizeof(*data), 1, file);
    fclose(file);

    return readCount == 1 && isValidSave(data);
}

static bool writeSave(const SaveData *data) {
    FILE *file;
    size_t writeCount;

    if (data == NULL) {
        return false;
    }

    file = fopen(SAVE_PATH, "wb");
    if (file == NULL) {
        return false;
    }

    writeCount = fwrite(data, sizeof(*data), 1, file);
    fclose(file);

    return writeCount == 1;
}

bool saveLoadLanguage(char *languageIdOut, size_t languageIdOutSize) {
    SaveData data = {0};

    if (languageIdOut == NULL || languageIdOutSize == 0) {
        return false;
    }
    if (!ensureFat() || !readSave(&data)) {
        languageIdOut[0] = '\0';
        return false;
    }

    copyStringSafe(languageIdOut, languageIdOutSize, data.languageId);
    return languageIdOut[0] != '\0';
}

bool saveStoreLanguage(const char *languageId) {
    SaveData data = {0};

    if (languageId == NULL || languageId[0] == '\0') {
        return false;
    }
    if (!ensureFat()) {
        return false;
    }

    if (!readSave(&data)) {
        memset(&data, 0, sizeof(data));
        data.magic = SAVE_MAGIC;
        data.version = SAVE_VERSION;
    }

    copyStringSafe(data.languageId, sizeof(data.languageId), languageId);
    return writeSave(&data);
}
