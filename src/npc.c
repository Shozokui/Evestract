#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "bytes.h"
#include "npc.h"

int LoadNPC(struct npc_t** npc, const uint8_t* buf, uint32_t length) {
    const uint8_t header[0x20] = {
        'n', 'o', 'n', 'e',   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0
    };

    // Auto-detect stride.
    uint32_t structLen = 0;

    if (length >= 0x20 && memcmp(buf, header, 0x20) == 0 && (length % 0x20) == 0) {
        structLen = 0x20;
    } else if (length >= 0x1c && memcmp(buf, header, 0x1c) == 0 && (length % 0x1c) == 0) {
        structLen = 0x1c;
    }

    if (structLen == 0) {
        fprintf(stderr, "invalid npc dat\n");
        return -1;
    }

    if ((length % structLen) != 0) {
        // This test was moved earlier to better support older files that
        // would get misreported to be the larger stride. Still not completely
        // robust but this will do for now.
        fprintf(stderr, "bad npc dat length\n");
        return -1;
    }

    uint32_t numEntries = (length / structLen) - 1;

    struct npc_entry_t* entries = (struct npc_entry_t*) calloc(numEntries, sizeof(struct npc_entry_t));
    if (entries == NULL) {
        return -1;
    }

    *npc = (struct npc_t*) calloc(1, sizeof(struct npc_t));
    if (*npc == NULL) {
        free(entries);
        return -1;
    }

    (*npc)->entries = entries;
    (*npc)->numEntries = numEntries;

    for (uint32_t i = 0; i < numEntries; i++) {
        uint32_t offset = (i + 1) * structLen;

        strncpy(entries[i].name, cptr8(buf, offset), structLen - 4);
        entries[i].id = lsb32(buf, offset, structLen - 4);
    }

    return 0;
}

int UnloadNPC(struct npc_t* npc) {
    return 0;   
}

const char* GetNPCName(const struct npc_t* npc, uint32_t npcId) {
    if (npc == NULL) {
        return NULL;
    }
    for (uint32_t i = 0; i < npc->numEntries; i++) {
        if (npc->entries[i].id == npcId) {
            return npc->entries[i].name;
        }
    }

    return NULL;
}
