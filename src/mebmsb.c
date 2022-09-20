#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include "ftable.h"
#include "readfile.h"

#include "parser.h"
#include "resource.h"

static bool menuChunkFilter(const chunk_t* chunk, [[maybe_unused]] void* userData) {
    switch (chunk->type) {
        case CHUNK_TYPE_Bmp2:
        case CHUNK_TYPE_D3s:
        case CHUNK_TYPE_Meb:
        case CHUNK_TYPE_Msb:
        case CHUNK_TYPE_Bp:
            return true;

        default:
            return false;
    }
}

static int menuChunkParser(const chunk_t* chunk, [[maybe_unused]] void* userData) {

    int ret = 0;

    switch (chunk->type) {
        case CHUNK_TYPE_Bmp2:
        case CHUNK_TYPE_D3s:
        case CHUNK_TYPE_Bp:
            ret = parseBmp2(chunk);
            break;
        case CHUNK_TYPE_Meb:
            ret = parseMeb(chunk);
            break;
        case CHUNK_TYPE_Msb:
            ret = parseMsb(chunk);
            break;

        default:
            break;
    }

    return ret;
}

int main(int argc, char* argv[]) {

    // basepath fileid
    // filename

    if (argc != 2 && argc != 3) {
        fprintf(stderr, "usage: %s <chunked.dat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char filename[65536];
    if (argc == 2) {
        strncpy(filename, argv[1], 65535);
        filename[65535] = '\0';
    } else {
        const char* basepath = argv[1];
        int fileId = atoi(argv[2]);

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

    uint32_t datLen;

    uint8_t* datBuf = ReadFile(filename, &datLen);
    if (datBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], filename);
        exit(EXIT_FAILURE);
    }

    int ret = parseChunks(datBuf, datLen, &(parse_chunk_userdata_t) {
        .filter = menuChunkFilter,
        .parser = menuChunkParser,
        .userData = NULL,
    });

    if (ret < 0) {
        fprintf(stderr, "%s: errors encountered parsing %s\n", argv[0], filename);
        exit(EXIT_FAILURE);
    }

    free(datBuf);

    return 0;
}
