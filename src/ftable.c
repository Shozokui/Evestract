#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "bytes.h"
#include "ftable.h"
#include "readfile.h"

// Assuming they don't add a ROMA or ROM10 at some point in the future.
#define VTABLE_MAX 10

int InitFTable(struct ftable_t** pFTable, const char* basepath) {
    *pFTable = (struct ftable_t*) calloc(1, sizeof(struct ftable_t));
    if (*pFTable == NULL) {
        return -1;
    }

    for (uint32_t i = 1; i < VTABLE_MAX; i++) {
        char filePath[65536];

#ifdef WIN32
        if (i == 1) {
            snprintf(filePath, sizeof(filePath), "%s\\VTABLE.DAT", basepath);
        } else {
            snprintf(filePath, sizeof(filePath), "%s\\ROM%u\\VTABLE%u.DAT", basepath, i + 1, i + 1);
        }
#else
        if (i == 1) {
            snprintf(filePath, sizeof(filePath), "%s/VTABLE.DAT", basepath);
        } else {
            snprintf(filePath, sizeof(filePath), "%s/ROM%u/VTABLE%u.DAT", basepath, i + 1, i + 1);
        }
#endif

        uint32_t length;
        uint8_t* dat = ReadFile(filePath, &length);
        if (dat == NULL) {
            if (i > 1) {
                continue;
            }
        }

#ifdef WIN32
        if (i == 1) {
            snprintf(filePath, sizeof(filePath), "%s\\FTABLE.DAT", basepath);
        } else {
            snprintf(filePath, sizeof(filePath), "%s\\ROM%u\\FTABLE%u.DAT", basepath, i + 1, i + 1);
        }
#else
        if (i == 1) {
            snprintf(filePath, sizeof(filePath), "%s/FTABLE.DAT", basepath);
        } else {
            snprintf(filePath, sizeof(filePath), "%s/ROM%u/FTABLE%u.DAT", basepath, i + 1, i + 1);
        }
#endif

        uint32_t length2;
        uint8_t* dat2 = ReadFile(filePath, &length2);

        if (dat2 == NULL) {
            free(dat);
            continue;
        }

        if (dat != NULL && length * 2 != length2) {
            fprintf(stderr, "Length mismatch between FTABLE and VTABLE %u\n", i);
            free(dat);
            free(dat2);
            continue;
        }

        if ((*pFTable)->length == 0) {
            if (length == 0) {
                // FTABLE-only mode
                length = length2 / 2;
            }

            (*pFTable)->length = length;

            (*pFTable)->entry = (ftable_entry_t *) calloc(length, sizeof(ftable_entry_t));

            for (uint32_t j = 0; j < length; j++) {
                (*pFTable)->entry[j].id = lsb16(dat2, j * 2);
                (*pFTable)->entry[j].version = dat != NULL ? dat[j] : ((*pFTable)->entry[j].id > 0 ? 1 : 0);
            }
        } else {
            if (length != (*pFTable)->length) {
                fprintf(stderr, "Length mismatch between VTABLE %u\n", i);
                free(dat);
                free(dat2);
                continue;
            }

            for (uint32_t j = 0; j < length; j++) {
                if (dat[j] == i)  {
                    (*pFTable)->entry[j].id = lsb16(dat2, j * 2);
                    (*pFTable)->entry[j].version = dat[j];
                }
            }

            free(dat);
            free(dat2);
        }
    }

    return 0;
}

int GetDatPath(struct ftable_t* ftable, char* buf, size_t length, const char* basepath, uint32_t fileId) {

    buf[0] = '\0';

    if (fileId >= ftable->length) {
        return 0;
    }

    uint8_t version = ftable->entry[fileId].version;
    if (version == 0) {
        return 0;
    }

    uint16_t fileIndex = ftable->entry[fileId].id;

#ifdef WIN32
    if (basepath != NULL) {
        if (version == 1) {
            snprintf(buf, length, "%s\\ROM\\%u\\%u.DAT", basepath, fileIndex >> 7, fileIndex & 0x7f);
        } else {
            snprintf(buf, length, "%s\\ROM%u\\%u\\%u.DAT", basepath, i + 1, fileIndex >> 7, fileIndex & 0x7f);
        }
    } else {
        if (version == 1) {
            snprintf(buf, length, "ROM\\%u\\%u.DAT", fileIndex >> 7, fileIndex & 0x7f);
        } else {
            snprintf(buf, length, "ROM%u\\%u\\%u.DAT", i + 1, fileIndex >> 7, fileIndex & 0x7f);
        }
    }
#else
    if (basepath != NULL) {
        if (version == 1) {
            snprintf(buf, length, "%s/ROM/%u/%u.DAT", basepath, fileIndex >> 7, fileIndex & 0x7f);
        } else {
            snprintf(buf, length, "%s/ROM%u/%u/%u.DAT", basepath, version, fileIndex >> 7, fileIndex & 0x7f);
        }
    } else {
        if (version == 1) {
            snprintf(buf, length, "ROM/%u/%u.DAT", fileIndex >> 7, fileIndex & 0x7f);
        } else {
            snprintf(buf, length, "ROM%u/%u/%u.DAT", version, fileIndex >> 7, fileIndex & 0x7f);
        }
    }
#endif

    if (buf[0] == '\0') {
        return 0;
    }

    return 1;
}
