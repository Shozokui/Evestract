
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "datid.h"
#include "ftable.h"
#include "npc.h"
#include "readfile.h"

int main(int argc, char* argv[]) {

    // basepath zone
    // filename

    if (argc != 2 && argc != 3) {
        fprintf(stderr, "usage: %s <npc.dat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char filename[65536];
    if (argc == 2) {
        strncpy(filename, argv[1], 65535);
        filename[65535] = '\0';
    } else {
        const char* basepath = argv[1];
        int zoneId = atoi(argv[2]);

        int fileId = GetEntityListFileId(zoneId);

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

    uint32_t npcLen;

    uint8_t* npcBuf = ReadFile(filename, &npcLen);
    if (npcBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], filename);
        exit(EXIT_FAILURE);
    }

    struct npc_t* npc;

    LoadNPC(&npc, npcBuf, npcLen);

    for (uint32_t i = 0; i < npc->numEntries; i++) {
        const struct npc_entry_t* entry = &npc->entries[i];
        printf("%u: %s\n", entry->id, entry->name);
    }

    UnloadNPC(npc);

    free(npcBuf);

    return 0;
}
