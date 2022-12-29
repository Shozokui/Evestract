#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "parser.h"

#include "../readfile.h"
#include "resource.h"


typedef struct userdata_t {
    // placeholder structure, just in case.
    int fileIdx;
} userdata_t;

static bool chunkFilter(const chunk_t* chunk, void* userData) {
    (void) userData;

    switch (chunk->type) {
        case CHUNK_TYPE_Mld:
            return true;

        default:
            return false;
    }
}

static int chunkParser(const chunk_t* chunk, void* userData) {

    (void) userData;

    int ret = 0;

    switch (chunk->type) {
        case CHUNK_TYPE_Mld:
            ret = parseMld(chunk);
            break;

        default:
            break;
    }

    return ret;
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s <chunked.dat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char filename[65536];

    strncpy(filename, argv[1], 65535);
    filename[65535] = '\0';

    uint32_t datLen;

    uint8_t* datBuf = ReadFile(filename, &datLen);
    if (datBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], filename);
        exit(EXIT_FAILURE);
    }

    userdata_t userData = {
        .fileIdx = 0,
    };

    int ret = parseChunks(datBuf, datLen, &(parse_chunk_userdata_t) {
        .filter = chunkFilter,
        .parser = chunkParser,
        .userData = &userData,
        .flags = PARSE_CHUNK_FLAG_VERBOSE,
    });

    if (ret < 0) {
        fprintf(stderr, "%s: errors encountered parsing %s\n", argv[0], filename);
        exit(EXIT_FAILURE);
    }

    free(datBuf);

    return 0;
}
