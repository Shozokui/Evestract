#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "bytes.h"
#include "dialog.h"
#include "text.h"

static int LoadDialog16(struct dialog_t** dialog, const uint8_t* buf, uint32_t length) {

    if (length < 4) {
        printf("# Invalid event message file\n");
        return -1;
    }

    if (lsb24(buf, 0) + 4 != length) {
        printf("# Invalid event message file\n");
        return -1;
    }

    if (lsb8(buf, 3) != 0 && lsb8(buf, 3) != 0x10) {
        printf("# Invalid event message file\n");
        return -1;
    }

    uint32_t mask = (lsb8(buf, 3) == 0x10) ? 0x80808080 : 0;
    uint32_t numEntries = 0;

    if (length >= 8) {
        uint32_t start = 4 + (lsb16(buf, 4) ^ mask);
        numEntries = (start - 4) / 2;
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
        uint32_t offset = 4 + (lsb16(buf, 4, i * 2) ^ mask);
        const uint8_t* ptr = ptr8(buf, offset);

        if (offset >= length) {
            printf("# Invalid event message file\n");
            return -1;
        }

        uint32_t entryLen = GetEventMessageSize(ptr, length - offset, mask & 0xff);

        struct dialog_entry_t* entry = &entries[i];

        entry->id = i;
        entry->length = entryLen;

        entry->text = (uint8_t*) calloc(1, entryLen + 1);

        memcpy(entry->text, ptr, entryLen);
        for (uint32_t j = 0; j < entryLen; j++) {
            entry->text[j] ^= mask;
        }
    }

    return 0;
}

int LoadDialog(struct dialog_t** dialog, const uint8_t* buf, uint32_t length) {

    if (length < 4) {
        printf("# Invalid event message file\n");
        return -1;
    }

    if (lsb24(buf, 0) + 4 != length) {
        printf("# Invalid event message file\n");
        return -1;
    }

    if (lsb8(buf, 3) != 0 && lsb8(buf, 3) != 0x10) {
        printf("# Invalid event message file\n");
        return -1;
    }

    uint32_t mask = (lsb8(buf, 3) == 0x10) ? 0x80808080 : 0;
    uint32_t numEntries = 0;

    if (length >= 8) {
        // Check for an older format with 16-bit offsets
        uint32_t l0 = lsb16(buf, 4) ^ (mask & 0xFFFF);
        uint32_t l1 = lsb16(buf, 6) ^ (mask & 0xFFFF);

        if (l0 < l1) {
            printf("# Trying LoadDialog16...\n");
            return LoadDialog16(dialog, buf, length);
        }

        uint32_t start = 4 + (lsb32(buf, 4) ^ mask);
        numEntries = (start - 4) / 4;
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
        uint32_t offset = 4 + (lsb32(buf, 4, i * 4) ^ mask);
        const uint8_t* ptr = ptr8(buf, offset);

        if (offset >= length) {
            // Pointing to the end of the file COULD be treated as empty string
            // Some files in the Promathia install are like this
            printf("# Invalid event message file\n");
            return -1;
        }

        uint32_t entryLen = GetEventMessageSize(ptr, length - offset, mask & 0xff);

        struct dialog_entry_t* entry = &entries[i];

        entry->id = i;
        entry->length = entryLen;

        entry->text = (uint8_t*) calloc(1, entryLen + 1);

        memcpy(entry->text, ptr, entryLen);
        for (uint32_t j = 0; j < entryLen; j++) {
            entry->text[j] ^= mask;
        }
    }

    return 0;
}

int UnloadDialog(struct dialog_t* dialog) {
    return 0;
}

const char* GetPrintableDialogText(const struct dialog_t* dialog, uint32_t index) {
    if (index < dialog->numEntries) {
        const struct dialog_entry_t* entry = &dialog->entries[index];
        return GetPrintableText(entry->text, entry->length);
    }

    return NULL;
}
