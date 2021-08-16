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
    if (length >= 0x20) {
        if (memcmp(buf, header, 0x20) == 0) {
            structLen = 0x20;
        } else if (memcmp(buf, header, 0x18) == 0) {
            structLen = 0x1c;
        } else {
            fprintf(stderr, "invalid npc dat\n");
            return -1;
        }
    }
    
    if ((length % structLen) != 0) {
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
