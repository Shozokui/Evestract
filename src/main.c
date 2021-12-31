
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "datid.h"
#include "dialog.h"
#include "ftable.h"
#include "event.h"
#include "npc.h"
#include "readfile.h"

int main(int argc, char* argv[]) {

    int verbose = 0;

    // basepath languageid eventZone entityZone messageZone
    // filename filename filename
    if (argc != 4 && argc != 6) {
        fprintf(stderr, "usage: %s <dialog.dat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char eventFileName[65536];
    char entityFileName[65536];
    char eventMessageFileName[65536];
    if (argc == 4) {
        strncpy(eventFileName, argv[1], 65535);
        eventFileName[65535] = '\0';

        strncpy(entityFileName, argv[2], 65535);
        entityFileName[65535] = '\0';

         strncpy(eventMessageFileName, argv[3], 65535);
        eventMessageFileName[65535] = '\0';
    } else {
        const char* basepath = argv[1];

        int languageId = atoi(argv[2]);
        int eventZoneId = atoi(argv[3]);
        int entityZoneId = atoi(argv[4]);
        int messageZoneId = atoi(argv[5]);

        ftable_t* ftable;

        int ret = InitFTable(&ftable, basepath);
        if (ret < 0) {
            fprintf(stderr, "%s: unable to init FTABLE at %s\n", argv[0], basepath);
            exit(EXIT_FAILURE);
        }

        int eventFileId = GetEventFileId(eventZoneId);
        int entityFileId = GetEntityListFileId(entityZoneId);
        int messageFileId = GetEventMessageFileId(languageId, messageZoneId);

        ret = GetDatPath(ftable, eventFileName, 65536, basepath, eventFileId);
        if (ret < 1) {
            fprintf(stderr, "%s: unable to query fileId %u\n", argv[0], eventFileId);
            exit(EXIT_FAILURE);
        }

        ret = GetDatPath(ftable, entityFileName, 65536, basepath, entityFileId);
        if (ret < 1) {
            fprintf(stderr, "%s: unable to query fileId %u\n", argv[0], entityZoneId);
            exit(EXIT_FAILURE);
        }

        ret = GetDatPath(ftable, eventMessageFileName, 65536, basepath, messageFileId);
        if (ret < 1) {
            fprintf(stderr, "%s: unable to query fileId %u\n", argv[0], messageZoneId);
            exit(EXIT_FAILURE);
        }
        free(ftable);
    }

    uint32_t eventLen;
    uint32_t dialogLen;
    uint32_t npcLen;

    uint8_t* eventBuf = ReadFile(eventFileName, &eventLen);
    if (eventBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], eventFileName);
        exit(EXIT_FAILURE);
    }

    uint8_t* dialogBuf = ReadFile(eventMessageFileName, &dialogLen);
    if (dialogBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], eventMessageFileName);
        exit(EXIT_FAILURE);
    }

    uint8_t* npcBuf = ReadFile(entityFileName, &npcLen);
    if (npcBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], entityFileName);
        exit(EXIT_FAILURE);
    }

    struct dialog_t* dialog;
    struct npc_t* npc;

    LoadDialog(&dialog, dialogBuf, dialogLen);
    LoadNPC(&npc, npcBuf, npcLen);

    ParseEvent(eventBuf, eventLen, dialog, npc, verbose);

    UnloadNPC(npc);

    free(npcBuf);
    free(dialogBuf);
    free(eventBuf);

    return 0;
}
