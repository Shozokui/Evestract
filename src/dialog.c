#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "bytes.h"
#include "dialog.h"

int LoadDialog(struct dialog_t** dialog, const uint8_t* buf, uint32_t length) {

    // Don't know if this is the correct way to count the number of strings.
    uint32_t numEntries = 0;
    uint32_t prev = 0;
    for (uint32_t i = 4; i < (length & ~3); i += 4) {
        uint32_t offset = lsb32(buf, i) ^ 0x80808080;
        if (offset >= length || offset < prev) {
            break;
        }

        prev = offset;
        numEntries++;
    }

    struct dialog_entry_t* entries = (struct dialog_entry_t*) calloc(numEntries, sizeof(struct dialog_entry_t));
    if (entries == NULL) {
        return -1;
    }

    *dialog = (struct dialog_t*) calloc(1, sizeof(struct dialog_t));
    if (*dialog == NULL) {
        free(entries);
        return -1;
    }

    (*dialog)->entries = entries;
    (*dialog)->numEntries = numEntries;

    for (uint32_t i = 0; i < numEntries; i++) {
        uint32_t offset = 4 + (lsb32(buf, 4, i * 4) ^ 0x80808080);

        uint32_t nextOffset = (i + 1) == numEntries
            ? length
            : 4 + (lsb32(buf, 8, i * 4) ^ 0x80808080);

        uint32_t entryLen = nextOffset - offset;

        struct dialog_entry_t* entry = &entries[i];

        entry->id = i;
        entry->length = entryLen;

        const uint8_t* ptr = ptr8(buf, offset);
        entry->text = (uint8_t*) calloc(1, entryLen + 1);

        memcpy(entry->text, ptr, entryLen);
        for (uint32_t j = 0; j < entryLen; j++) {
            entry->text[j] ^= 0x80;
        }

#if 0
        printf("%u: ", i);

        for (uint32_t j = 0; j < entryLen; j++) {
            uint8_t b = entry->text[j];

            if (isprint(b)) {
                printf("%c", b);
            } else if (b == 0xc && (j+1) < entryLen && entry->text[j+1] >= 1 && entry->text[j+1] <= 8) {
                char tmp[16];
                sprintf(tmp, "[Param%d]", entry->text[j+1] - 1);
                printf("%s", tmp);
            } else {
                char tmp[16];
                sprintf(tmp, "<%02x>", b);
                printf("%s", tmp);
            }
        }

        printf("\n");
#endif
    }

    return 0;
}
