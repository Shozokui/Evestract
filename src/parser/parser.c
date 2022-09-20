#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "parser.h"
#include "resource.h"

static int parseChunk(const chunk_t* chunk, void* _userData) {
    parse_chunk_userdata_t* userData = (parse_chunk_userdata_t*) _userData;

    if (chunk->type == 1) {
        // Rmp, push onto the stack
        int index = userData->stackIndex + 1;

        if (index < 0 || (size_t) index >= sizeof(userData->stack) / sizeof(userData->stack[0])) {
            // Overflow
            return -1;
        }

        userData->stack[index] = *chunk;
        userData->stackIndex = index;
    } else if (chunk->type == 0) {
        // Terminate, pop from the stack
        int index = userData->stackIndex - 1;

        if (index < -2) {
            // Underflow
            // (-1 is allowed for the two dats that seem to have an extra Terminate?)
            return -1;
        }

        userData->stackIndex = index;
    } else {
        int index = userData->stackIndex;

        if (index < 0) {
            // Underflow
            // (An extra trailing Terminate is tolerated, but there should never be any actual resources
            // outside of a Rmp/Terminate pair)
            return -1;
        }

        bool wanted = true;
        if (userData->filter != NULL) {
            wanted = userData->filter(chunk, userData->userData);
        }

        if (wanted) {
            for (int index = 0; index <= userData->stackIndex; index++) {
                const chunk_t* c = &userData->stack[index];

                if (c->flags != 0) {
                    printf("'%s'[%01X] > ", c->FourCC, c->flags);
                } else {
                    printf("'%s' > ", c->FourCC);
                }
            }

            const char* typeName = GetChunkTypeName(chunk->type);

            if (chunk->flags != 0) {
                printf("'%s'.%s[%01X]:\n", chunk->FourCC, typeName, chunk->flags);
            } else {
                printf("'%s'.%s:\n", chunk->FourCC, typeName);
            }

            if (userData->parser != NULL) {
                int ret = userData->parser(chunk, userData->userData);
                if (ret < 0) {
                    return ret;
                }
            }

            printf("\n");
        }
    }

    return 0;
}


int parseChunks(const uint8_t* datBuf, uint32_t datLen, parse_chunk_userdata_t* userData) {
    int ret = LoadChunkedResource(datBuf, datLen, &parseChunk, &(parse_chunk_userdata_t) {
        .filter = userData->filter,
        .parser = userData->parser,

        .stackIndex = -1,
    });

    return ret;
}
