#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "../ftable.h"
#include "../readfile.h"

#include "bytes.h"
#include "parser.h"
#include "resource.h"

#include "tga.h"

typedef struct chunk_node_t {
    struct chunk_node_t* next;
    struct chunk_node_t* prev;

    chunk_t chunk;

    bool owned;
} chunk_node_t;

typedef struct chunk_root_t {
    chunk_node_t* next;
    chunk_node_t* prev;
} chunk_root_t;

static void chunk_node_init(chunk_root_t* root) {
    root->next = NULL;
    root->prev = NULL;
}

static void chunk_node_deinit(chunk_root_t* root) {
    chunk_node_t* node = root->next;

    while (node != NULL) {
        if (node->owned) {
            free((void*) node->chunk.buf);
        }
        chunk_node_t* next = node->next;
        free(node);
        node = next;
    }

    root->next = NULL;
    root->prev = NULL;
}

static void chunk_node_append(chunk_root_t* root, const chunk_t* chunk, bool owned) {
    chunk_node_t* node = (chunk_node_t*) calloc(1, sizeof(chunk_node_t));

    node->chunk = *chunk;
    node->owned = owned;

    if (root->next == NULL) {
        root->next = node;
        root->prev = node;
    } else {
        node->prev = root->prev;
        root->prev->next = node;
        root->prev = node;
    }
}

static int chunk_node_iterate(chunk_root_t* root, int (*chunk_node_iterator)(const chunk_t* chunk, void* userData), void* userData) {
    chunk_node_t* node = root->next;

    while (node != NULL) {
        chunk_node_iterator(&node->chunk, userData);

        node = node->next;
    }

    return 0;
}

static int createFontD3s(chunk_t* chunk, const char* name) {
    int width;
    int height;

    uint8_t* rgbaBuf = LoadTGARGBAFromFile("./export.tga", &width, &height);

    if (rgbaBuf == NULL) {
        fprintf(stderr, "Failed to read export.tga!\n");
        return -1;
    }

    printf("dimensions: %d x %d\n", width, height);

    int length = chunk_align(16 + 57 + width * height * 4);

    uint8_t* buf = (uint8_t*) calloc(1, length);

    // Standard resource header, 16 bytes
    chunk->length = length;
    chunk->buf = buf;
    chunk->flags = 0;
    chunk->type = CHUNK_TYPE_D3s;

    chunk->FourCC[0] = 'v';
    chunk->FourCC[1] = 'y';
    chunk->FourCC[2] = 'l';
    chunk->FourCC[3] = 'n';
    chunk->FourCC[4] = '\0';

    chunk_write_header(buf, chunk);

    // Bmp2 header, 57 bytes
    uint32_t IsCompressed = 0;
    uint32_t Version = 1;
    uint32_t VersionFlags = (IsCompressed << 7) | (Version << 4);

    uint32_t Width = width;
    uint32_t Height = height;
    uint32_t BitsPerPixel = 32;
    uint32_t PaletteBitsPerPixel = 32;
    // uint32_t Flags = 0;
    // uint32_t FourCC = 0;
    // uint32_t PaletteOffset = 0;
    // uint32_t TextureOffset = 57;
    // uint32_t CompressedTextureOffset = 0;
    // uint32_t UnknownDxt0 = 0;
    // uint32_t UnknownDxt1 = 0;

    buf[16 + 0] = VersionFlags & 0xFF;
    strncpy((char*) &buf[17], name, 16);

    buf[16 + 21] = (Width >> 0) & 0xFF;
    buf[16 + 22] = (Width >> 8) & 0xFF;
    buf[16 + 23] = (Width >> 16) & 0xFF;
    buf[16 + 24] = (Width >> 24) & 0xFF;
    buf[16 + 25] = (Height >> 0) & 0xFF;
    buf[16 + 26] = (Height >> 8) & 0xFF;
    buf[16 + 27] = (Height >> 16) & 0xFF;
    buf[16 + 28] = (Height >> 24) & 0xFF;
    buf[16 + 31] = BitsPerPixel & 0xFF;
    buf[16 + 53] = PaletteBitsPerPixel & 0xFF;

    memcpy(&buf[16 + 57], rgbaBuf, width * height * 4);

    free(rgbaBuf);

    return 0;
}


static int patchD3s(chunk_root_t* chunkRoot, const chunk_t* chunk) {
    chunk_node_append(chunkRoot, chunk, false);

    // Search for the insertion point
    if (strcmp(chunk->FourCC, "font") == 0) {
        int offset = 16;

        char textureName[17];
        memset(textureName, 0, sizeof(textureName));
        strncpy(textureName, cptr8(chunk->buf, offset, 1), 16);

        if (strcmp(textureName, "font    font    ") == 0) {
            chunk_t newChunk;

            int ret = createFontD3s(&newChunk, "font    vlyn    ");

            if (ret >= 0) {
                chunk_node_append(chunkRoot, &newChunk, true);

                // free((void*) newChunk.buf);
                //
                // chunk_clone(chunk, &newChunk);
                //
                // newChunk.FourCC[0] = 'v';
                // newChunk.FourCC[1] = 'l';
                // newChunk.FourCC[2] = 'y';
                // newChunk.FourCC[3] = 'n';
                //
                // strncpy((char*) &newChunk.buf[16 + 1], "font    vlyn    ", 16);
                //
                // chunk_write_header((uint8_t*) newChunk.buf, &newChunk);
                // chunk_node_append(chunkRoot, &newChunk, true);
            }
        }
    }

    return 0;
}

static int patchFontMsb(chunk_t* chunk) {
    uint8_t* buf = (uint8_t*) chunk->buf;

    int offset = 16;

    // name
    offset += 16;

    // number of elements
    int numElems = lsb8(buf, offset);
    offset += 1;

    // element names
    for (int i = 0; i < numElems; i++) {
        char textureName[17];
        memset(textureName, 0, sizeof(textureName));
        strncpy(textureName, cptr8(chunk->buf, offset), 16);
        int TextureNameOffset = offset;
        offset += 16;

        if (strcmp(textureName, "font    font    ") == 0) {
            // Patch the texture name.
            memcpy(cptr8(chunk->buf, TextureNameOffset), "font    vlyn    ", 16);
        }
    }

    // number of children
    int numChildren = lsb16(buf, offset);
    offset += 2;

    for (int i = 0; i < numChildren; i++) {

        // number of subchildren
        int numSubChildren = lsb8(buf, offset);
        offset += 1;

        for (int j = 0; j < numSubChildren; j++) {

            // X/Y Output Dimensions
            offset += 16;

            // UV Offsets
            int UVOffset = offset;
            offset += 8;

            // UV Flags
            offset += 1;

            // Colors
            offset += 16;

            // Blend Mode
            offset += 4;

            // Texture Name
            char textureName[17];
            memset(textureName, 0, sizeof(textureName));
            strncpy(textureName, cptr8(chunk->buf, offset), 16);
            int TextureNameOffset = offset;
            offset += 16;

            if (strcmp(textureName, "font    font    ") == 0) {
                // printf("Found correct texture, patching UV...\n");

                // 4X Scale the UV Offsets
                int uw = slsb16(buf, UVOffset + 0) * 4;
                int vh = slsb16(buf, UVOffset + 2) * 4;
                int u0 = slsb16(buf, UVOffset + 4) * 4;
                int v0 = slsb16(buf, UVOffset + 6) * 4;

                buf[UVOffset + 0] = (uw >> 0) & 0xFF;
                buf[UVOffset + 1] = (uw >> 8) & 0xFF;
                buf[UVOffset + 2] = (vh >> 0) & 0xFF;
                buf[UVOffset + 3] = (vh >> 8) & 0xFF;
                buf[UVOffset + 4] = (u0 >> 0) & 0xFF;
                buf[UVOffset + 5] = (u0 >> 8) & 0xFF;
                buf[UVOffset + 6] = (v0 >> 0) & 0xFF;
                buf[UVOffset + 7] = (v0 >> 8) & 0xFF;

                // Patch the texture name.
                memcpy(cptr8(chunk->buf, TextureNameOffset), "font    vlyn    ", 16);
            }
        }
    }

    return 0;
}

static int patchMsb(chunk_root_t* chunkRoot, const chunk_t* chunk) {

    // Search for the insertion point
    if (strcmp(chunk->FourCC, "font") == 0) {
        int offset = 16;

        // printf("Found font Msb...\n");

        char name[17];
        strncpy(name, cptr8(chunk->buf, offset), 16);
        name[16] = '\0';

        if (strcmp(name, "font    fontshp ") == 0) {
            // printf("Name matches, starting patch process...\n");

            chunk_t clone;

            // Direct clone, size will not change.
            chunk_clone(chunk, &clone);

            patchFontMsb(&clone);

            chunk_node_append(chunkRoot, &clone, true);
            return 0;
        }
    }

    chunk_node_append(chunkRoot, chunk, false);
    return 0;
}

static int menuChunkParser(const chunk_t* chunk, void* userData) {

    chunk_root_t* chunkRoot = (chunk_root_t*) userData;

    int ret = 0;

    switch (chunk->type) {
        case CHUNK_TYPE_D3s:
            ret = patchD3s(chunkRoot, chunk);
            break;

        case CHUNK_TYPE_Msb:
            ret = patchMsb(chunkRoot, chunk);
            break;

        default:
            chunk_node_append(chunkRoot, chunk, false);
            break;
    }

    return ret;
}

static int menuChunkIterator(const chunk_t* chunk, void* userData) {
    FILE* fp = (FILE*) userData;

    (void) fwrite(chunk->buf, chunk->length, 1, fp);

    return 0;
}

int main(int argc, char* argv[]) {

    // basepath fileid
    // filename

    if (argc != 3 && argc != 4) {
        fprintf(stderr, "usage: %s <chunked.dat> <modified.dat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char filename[65536];
    const char* dest = NULL;

    if (argc == 3) {
        strncpy(filename, argv[1], 65535);
        filename[65535] = '\0';

        dest = argv[2];
    } else {
        const char* basepath = argv[1];
        int fileId = atoi(argv[2]);

        dest = argv[3];

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

    chunk_root_t chunkRoot;
    chunk_node_init(&chunkRoot);

    int ret = parseChunks(datBuf, datLen, &(parse_chunk_userdata_t) {
        .filter = NULL,
        .parser = menuChunkParser,
        .userData = &chunkRoot,
    });

    if (ret < 0) {
        fprintf(stderr, "%s: errors encountered parsing %s\n", argv[0], filename);
        exit(EXIT_FAILURE);
    }

    FILE* fp = fopen(dest, "wb");
    if (dest == NULL) {
        fprintf(stderr, "%s: unable to open \"%s\" for writing\n", argv[0], dest);
        exit(EXIT_FAILURE);
    }

    chunk_node_iterate(&chunkRoot, menuChunkIterator, (void*) fp);
    fclose(fp);

    chunk_node_deinit(&chunkRoot);

    free(datBuf);

    return 0;
}
