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

    struct event_zone_t* eventZone = (struct event_zone_t*) calloc(1, sizeof(struct event_zone_t));
    if (eventZone == NULL) {
        return -1;
    }

    uint32_t npcCount = lsb32(buf, 0);
    uint32_t lengthOffset = 4;
    uint32_t eventNpcOffset = lengthOffset + 4 * npcCount;

    eventZone->numEvents = npcCount;

    eventZone->events = (struct event_npc_t*) calloc(npcCount, sizeof(struct event_npc_t));
    if (eventZone->events == NULL) {
        return -1;
    }

    for (uint32_t i = 0; i < npcCount; i++) {
        struct event_npc_t* eventNpc = &eventZone->events[i];

        uint32_t eventNpcLength = lsb32(buf, lengthOffset);
        lengthOffset += 4;

        uint32_t offset = eventNpcOffset;
        eventNpcOffset += eventNpcLength;

        uint32_t NPCId = lsb32(buf, offset, 0);
        uint32_t eventCount = lsb32(buf, offset, 4);

        offset += 8;

        eventNpc->NPCId = NPCId;
        eventNpc->numEvents = eventCount;

        eventNpc->events = (struct event_t*) calloc(eventNpc->numEvents, sizeof(struct event_t));
        if (eventNpc->events == NULL) {
            return -1;
        }

        for (uint32_t j = 0; j < eventNpc->numEvents; j++) {
            eventNpc->events[j].pc = lsb16(buf, offset);
            offset += 2;
        }

        for (uint32_t j = 0; j < eventNpc->numEvents; j++) {
            eventNpc->events[j].id = lsb16(buf, offset);
            offset += 2;
        }

        eventNpc->numConstants = lsb32(buf, offset);
        offset += 4;

        eventNpc->constants = (uint32_t*) calloc(eventNpc->numConstants, sizeof(uint32_t));
        if (eventNpc->constants == NULL) {
            return -1;
        }

        for (uint32_t j = 0; j < eventNpc->numConstants; j++) {
            eventNpc->constants[j] = lsb32(buf, offset);
            offset += 4;
        }

        eventNpc->bytecodeSize = lsb32(buf, offset);
        offset += 4;

        eventNpc->bytecode = (uint8_t*) calloc(eventNpc->bytecodeSize, sizeof(uint8_t));
        if (eventNpc->bytecode == NULL) {
            return -1;
        }

        memcpy(&eventNpc->bytecode[0], &buf[offset], eventNpc->bytecodeSize);
    }

    printf("Event NPC Count: %u\n", npcCount);

    for (uint32_t i = 0; i < npcCount; i++) {
        struct event_npc_t* eventNpc = &eventZone->events[i];

        const struct npc_entry_t* entry = FindNPC(npc, eventNpc->NPCId);
        if (entry != NULL) {
            printf("Event NPC %5u NPC \"%s\" (%u)\n", i, entry->name, eventNpc->NPCId);
        } else if (eventNpc->NPCId == 2147483632) {
             printf("Event NPC %5u NPC %s (%u)\n", i, "[Zone]", eventNpc->NPCId);
        } else {
             printf("Event NPC %5u NPC %s (%u)\n", i, "[Unknown]", eventNpc->NPCId);
        }

        printf("Event NPC %5u Event Count: %u\n", i, eventNpc->numEvents);

        for (uint32_t j = 0; j < eventNpc->numEvents; j++) {
            struct event_t* event = &eventNpc->events[j];

            printf("Event NPC %5u Event %5u Id: %5u PC: L%04X\n",
                i,
                j,
                event->id,
                event->pc);
        }

        printf("Event NPC %5u Constant Count: %u\n", i, eventNpc->numConstants);

        for (uint32_t j = 0; j < eventNpc->numConstants; j++) {
            printf("Event NPC %5u Constant %5u: %u\n", i, j, eventNpc->constants[j]);
        }

        printf("Event NPC %5u Code Size: %u\n", i, eventNpc->bytecodeSize);

        printf("Event NPC %5u Code: ", i);
        for (uint32_t k = 0; k < eventNpc->bytecodeSize; k++) {
            printf("%02x", eventNpc->bytecode[k]);
        }
        printf("\n");

        ParseScript(eventZone, i, dialog, npc);
    }

    return 0;
}
