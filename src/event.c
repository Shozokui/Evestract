#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "bytes.h"
#include "dialog.h"
#include "event.h"
#include "npc.h"
#include "script.h"

static const struct npc_entry_t* FindNPC(const struct npc_t* npc, uint32_t npcId) {
    if (npc == NULL) {
        return NULL;
    }
    for (uint32_t i = 0; i < npc->numEntries; i++) {
        if (npc->entries[i].id == npcId) {
            return &npc->entries[i];
        }
    }

    return NULL;
}

int ParseEvent(const uint8_t* buf, uint32_t length, const struct dialog_t* dialog, const struct npc_t* npc) {

    uint32_t blockCount = lsb32(buf, 0);

    uint32_t* blockSizes = (uint32_t*) malloc(blockCount * sizeof(uint32_t));
    uint32_t* blockOffsets = (uint32_t*) malloc(blockCount * sizeof(uint32_t));

    for (uint32_t i = 0; i < blockCount; i++) {
        blockSizes[i] = lsb32(buf, 4 + i * 4);
    }

    for (uint32_t i = 0; i < blockCount; i++) {
        if (i > 0) {
            blockOffsets[i] = blockOffsets[i-1] + blockSizes[i-1];
        } else {
            blockOffsets[i] = 4 + blockCount * 4;
        }
    }

    printf("Event Block Count: %u\n", blockCount);

    for (uint32_t i = 0; i < blockCount; i++) {
        printf("Event Block %5u Offset: %u\n", i, blockOffsets[i]);
        printf("Event Block %5u Size: %u\n", i, blockSizes[i]);

        uint32_t offset = blockOffsets[i];

        uint32_t actorId = lsb32(buf, offset, 0);
        uint32_t eventCount = lsb32(buf, offset, 4);
        offset += 8;

        const struct npc_entry_t* actor = FindNPC(npc, actorId);
        if (actor != NULL) {
            printf("Event Block %5u NPC %s (%u)\n", i, actor->name, actorId);
        } else if (actorId == 2147483632) {
             printf("Event Block %5u NPC %s (%u)\n", i, "[Zone]", actorId);
        } else {
            printf("Event Block %5u NPC Id: %u\n", i, actorId);
        }

        printf("Event Block %5u Event Count: %u\n", i, eventCount);

        uint32_t* eventOffsets = (uint32_t*) malloc(eventCount * sizeof(uint32_t));
        uint32_t* eventSizes = (uint32_t*) malloc(eventCount * sizeof(uint32_t));
        uint32_t* eventIds = (uint32_t*) malloc(eventCount * sizeof(uint32_t));

        for (uint32_t j = 0; j < eventCount; j++) {
            uint32_t eventOffset = lsb16(buf, offset);
            offset += 2;

            eventOffsets[j] = eventOffset;
        }

        for (uint32_t j = 0; j < eventCount; j++) {
            uint32_t eventId = lsb16(buf, offset);
            offset += 2;

            eventIds[j] = eventId;
        }

        for (uint32_t j = 0; j < eventCount; j++) {
            printf("Event Block %5u Event %5u Id: %5u Offset: %5u\n",
                i,
                j,
                eventIds[j],
                eventOffsets[j]);
        }

        uint32_t constantCount = lsb32(buf, offset);
        offset += 4;

        printf("Event Block %5u Constant Count: %u\n", i, constantCount);

        uint32_t* constants = (uint32_t*) calloc(constantCount, sizeof(uint32_t));

        for (uint32_t j = 0; j < constantCount; j++) {
            uint32_t constant = lsb32(buf, offset);
            offset += 4;

            constants[j] = constant;

            printf("Event Block %5u Constant %5u: %u\n", i, j, constant);
        }

        uint32_t sceneSize = lsb32(buf, offset);
        offset += 4;

        printf("Event Block %5u Scene Size: %u\n", i, sceneSize);

        for (uint32_t j = 0; j < eventCount; j++) {
            if (j < (eventCount - 1)) {
                eventSizes[j] = eventOffsets[j+1] - eventOffsets[j];
            } else {
                eventSizes[j] = sceneSize - eventOffsets[j];
            }
        }

        for (uint32_t j = 0; j < eventCount; j++) {
            eventOffsets[j] += offset;
        }

        for (uint32_t j = 0; j < eventCount; j++) {
            offset = eventOffsets[j];
            uint32_t count = eventSizes[j];

            printf("Event Block %5u Event %5u Id: %5u Data:",
                i,
                j,
                eventIds[j]);
            for (uint32_t k = 0; k < count; k++) {
                printf("%02x", lsb8(buf, offset, k));
            }
            printf("\n");

            ParseScript(&buf[offset], count, constants, constantCount, dialog, npc);
        }
    }

    return 0;
}
