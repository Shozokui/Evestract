
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "datid.h"
#include "dialog.h"
#include "ftable.h"
#include "readfile.h"
#include "text.h"

int main(int argc, char* argv[]) {

    // basepath languageid zone
    // basepath fileid
    // filename

    if (argc != 2 && argc != 3 && argc != 4) {
        fprintf(stderr, "usage: %s <dialog.dat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char filename[65536];
    if (argc == 2) {
        strncpy(filename, argv[1], 65535);
        filename[65535] = '\0';
    } else {
        const char* basepath = argv[1];
        int fileId = 0;

        if (argc == 3) {
            fileId = atoi(argv[2]);
        } else {
            int languageId = atoi(argv[2]);
            int zoneId = atoi(argv[3]);

            fileId = GetEventMessageFileId(languageId, zoneId);
        }

        ftable_t* ftable;

        int ret = InitFTable(&ftable, basepath);
        if (ret < 0) {
            fprintf(stderr, "%s: unable to init FTABLE at %s\n", argv[0], basepath);
            exit(EXIT_FAILURE);
        }

        ret = GetDatPath(ftable, filename, 65536, basepath, fileId);
        if (ret < 1) {
            fprintf(stderr, "%s: unable to query fileId %u\n", argv[0], fileId);
            exit(EXIT_FAILURE);
        }
        free(ftable);
    }

    uint32_t dialogLen;

    uint8_t* dialogBuf = ReadFile(filename, &dialogLen);
    if (dialogBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], filename);
        exit(EXIT_FAILURE);
    }

    struct dialog_t* dialog;

    LoadDialog(&dialog, dialogBuf, dialogLen);

    for (uint32_t i = 0; i < dialog->numEntries; i++) {
        const struct dialog_entry_t* entry = &dialog->entries[i];

        const char* text = GetPrintableDialogText(dialog, i);
        printf("%u: %s\n", entry->id, text);
        FreePrintableText(text);
    }

    UnloadDialog(dialog);

    free(dialogBuf);

    return 0;
}
