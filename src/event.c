#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "bytes.h"
#include "event.h"
#include "script.h"

int ParseEvent(const uint8_t* buf, uint32_t length) {

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

        printf("Event Block %5u Actor Id: %u\n", i, actorId);
        printf("Event Block %5u Event Count: %u\n", i, eventCount);

        uint32_t* eventOffsets = (uint32_t*) malloc(eventCount * sizeof(uint32_t));
        uint32_t* eventSizes = (uint32_t*) malloc(eventCount * sizeof(uint32_t));
        uint32_t* eventIds = (uint32_t*) malloc(eventCount * sizeof(uint32_t));

        for (uint32_t j = 0; j < eventCount; j++) {
            uint32_t eventOffset = lsb16(buf, offset);
            offset += 2;

            eventOffsets[j] = eventOffset;

            printf("Event Block %5u Event %5u Offset: %u\n", i, j, eventOffset);
        }

        for (uint32_t j = 0; j < eventCount; j++) {
            uint32_t eventId = lsb16(buf, offset);
            offset += 2;

            eventIds[j] = eventId;

            printf("Event Block %5u Event %5u Id: %u\n", i, j, eventId);
        }

        uint32_t referenceCount = lsb32(buf, offset);
        offset += 4;

        printf("Event Block %5u Reference Count: %u\n", i, referenceCount);

        for (uint32_t j = 0; j < referenceCount; j++) {
            uint32_t reference = lsb32(buf, offset);
            offset += 4;

            printf("Event Block %5u Reference %5u: %u\n", i, j, reference);
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

            printf("Event Block %5u Event %5u Data:", i, j);

            for (uint32_t k = 0; k < count; k++) {
                printf("%02x", lsb8(buf, offset, k));
            }
            printf("\n");

            ParseScript(&buf[offset], count);
        }
    }

    return 0;
}
