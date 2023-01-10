#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "bytes.h"
#include "resource.h"

#include "vum.h"


int parseVum(const chunk_t* chunk) {
    printf("\t%s \"%4s\"[%02x]: %u bytes (%08X)\n", GetChunkTypeName(chunk->type), chunk->FourCC, chunk->type, chunk->length, chunk->flags);

    const uint8_t* vum = chunk->buf + 16;

    uint32_t Version = lsb8(vum, 0x00);

    uint16_t PolygonCount = lsb16(vum, 0x06);

    uint32_t Layer0 = lsb32(vum, 0x08);
    uint32_t Layer1 = lsb32(vum, 0x0C);

    // versions 2+ only
    // collision mesh
    uint32_t CollisionMesh = lsb32(vum, 0x10);

    // versions 3+ only
    // not needed for extraction-- reserved space to hold pointers to texture objects
    uint32_t TexturePointerOffset = lsb32(vum, 0x14);

    // versions 4+ only
    // used to patch texture settings into model data without having to iterate through the model data
    uint32_t TextureFixupCount = lsb32(vum, 0x18);
    uint32_t TextureFixupOffset = lsb32(vum, 0x1C);

    char modelName[17];

    memset(modelName, 0, sizeof(modelName));
    strncpy(modelName, cptr8(vum, 0x40), 16);

    uint32_t NumTextures = lsb32(vum, 0x50);
    uint32_t TextureNameOffset = lsb32(vum, 0x54);

    printf("\tModel Name: \"%s\"\n", modelName);

    printf("\tVersion: %u\n", Version);

    printf("\tPolygon Count: %u\n", PolygonCount);

    printf("\tBBoxL: %f, %f, %f, %f\n", lsbf32(vum, 0x20) * 0.0625, lsbf32(vum, 0x24) * 0.0625, lsbf32(vum, 0x28) * 0.0625, lsbf32(vum, 0x2C) * 0.0625);
    printf("\tBBoxR: %f, %f, %f, %f\n", lsbf32(vum, 0x30) * 0.0625, lsbf32(vum, 0x34) * 0.0625, lsbf32(vum, 0x38) * 0.0625, lsbf32(vum, 0x3C) * 0.0625);

    printf("\tAttach0: %u\n", Layer0);

    uint32_t NumAttach0 = lsb32(vum, Layer0);
    printf("\tNumAttach0: %u\n", NumAttach0);

    for (uint32_t i = 0; i < NumAttach0; i++) {
        uint32_t offset = lsb32(vum, Layer0 + 4, i * 4);

        printf("\t\tOffset: %u\n", offset);

        const uint8_t* part = &vum[offset];

        printf("\t\t\tBound: %f, %f, %f, %f\n", lsbf32(part, 0x00), lsbf32(part, 0x04), lsbf32(part, 0x08), lsbf32(part, 0x0C));
        printf("\t\t\tBBoxL: %f, %f, %f, %f\n", lsbf32(part, 0x10), lsbf32(part, 0x14), lsbf32(part, 0x18), lsbf32(part, 0x1C));
        printf("\t\t\tBBoxR: %f, %f, %f, %f\n", lsbf32(part, 0x20), lsbf32(part, 0x24), lsbf32(part, 0x28), lsbf32(part, 0x2C));

        uint32_t Index = lsb32(part, 0x30);
        printf("\t\t\tIndex: %u\n", Index);

        const uint8_t* model = &part[0x40];

        printf("\t\t\t%u\n", lsb16(model, 0));
        printf("\t\t\t%04X\n", lsb16(model, 2));
        printf("\t\t\t%08X\n", lsb32(model, 4));

        printf("\t\t\t%08X\n", lsb32(model, 8));
        printf("\t\t\t%08X\n", lsb32(model, 12));
        printf("\t\t\t%08X\n", lsb32(model, 16));
        printf("\t\t\t%08X\n", lsb32(model, 20));
        printf("\t\t\t%08X\n", lsb32(model, 24));
        printf("\t\t\t%08X\n", lsb32(model, 28));
        printf("\t\t\t%08X\n", lsb32(model, 32));
        printf("\t\t\t%08X\n", lsb32(model, 36));
        printf("\t\t\t%08X\n", lsb32(model, 40));
        printf("\t\t\t%08X\n", lsb32(model, 44));
        printf("\t\t\t%08X\n", lsb32(model, 48));
        printf("\t\t\t%08X\n", lsb32(model, 52));
        printf("\t\t\t%08X\n", lsb32(model, 56));
        printf("\t\t\t%08X\n", lsb32(model, 60));
        printf("\t\t\t%08X\n", lsb32(model, 64));
        printf("\t\t\t%08X\n", lsb32(model, 68));
        printf("\t\t\t%08X\n", lsb32(model, 72));
        printf("\t\t\t%08X\n", lsb32(model, 76));
        printf("\t\t\t%08X\n", lsb32(model, 80));
        printf("\t\t\t%08X\n", lsb32(model, 84));
        printf("\t\t\t%08X\n", lsb32(model, 88));
        printf("\t\t\t%08X\n", lsb32(model, 92));


    }

    printf("\tAttach1: %u\n", Layer1);

    uint32_t NumAttach1 = lsb32(vum, Layer1);
    printf("\tNumAttach1: %u\n", NumAttach1);

    for (uint32_t i = 0; i < NumAttach1; i++) {
        uint32_t offset = lsb32(vum, Layer1 + 4, i * 4);

        printf("\t\tOffset: %u\n", offset);

        const uint8_t* part = &vum[offset];

        printf("\t\t\tBound: %f, %f, %f, %f\n", lsbf32(part, 0x00), lsbf32(part, 0x04), lsbf32(part, 0x08), lsbf32(part, 0x0C));
        printf("\t\t\tBBoxL: %f, %f, %f, %f\n", lsbf32(part, 0x10), lsbf32(part, 0x14), lsbf32(part, 0x18), lsbf32(part, 0x1C));
        printf("\t\t\tBBoxR: %f, %f, %f, %f\n", lsbf32(part, 0x20), lsbf32(part, 0x24), lsbf32(part, 0x28), lsbf32(part, 0x2C));

        uint32_t Index = lsb32(part, 0x30);
        printf("\t\t\tIndex: %u\n", Index);
    }

    printf("\tAttach2: %u\n", CollisionMesh);

    uint32_t NumAttach2 = lsb32(vum, CollisionMesh);
    printf("\tNumAttach2: %u\n", NumAttach2);

    for (uint32_t i = 0; i < NumAttach2; i++) {
        uint32_t offset = lsb32(vum, CollisionMesh + 4, i * 4);

        printf("\t\tOffset: %u\n", offset);

        const uint8_t* part = &vum[offset];

        printf("\t\t\tBound: %f, %f, %f, %f\n", lsbf32(part, 0x00), lsbf32(part, 0x04), lsbf32(part, 0x08), lsbf32(part, 0x0C));
        printf("\t\t\tBBoxL: %f, %f, %f, %f\n", lsbf32(part, 0x10), lsbf32(part, 0x14), lsbf32(part, 0x18), lsbf32(part, 0x1C));
        printf("\t\t\tBBoxR: %f, %f, %f, %f\n", lsbf32(part, 0x20), lsbf32(part, 0x24), lsbf32(part, 0x28), lsbf32(part, 0x2C));

        uint32_t Index = lsb32(part, 0x30);
        printf("\t\t\tIndex: %u\n", Index);
    }

    printf("\tTexturePointerOffset: %u\n", TexturePointerOffset);

    printf("\tTextureFixupCount: %u\n", TextureFixupCount);
    printf("\tTextureFixupOffset: %u\n", TextureFixupOffset);

    printf("\tNum Textures: %u\n", NumTextures);
    printf("\tTextureNameOffset: %u\n", TextureNameOffset);

    for (uint32_t i = 0; i < NumTextures; i++) {
        char textureName[17];
        memset(textureName, 0, sizeof(textureName));
        strncpy(textureName, cptr8(vum, TextureNameOffset, 16 * i), 16);

        printf("\t\t\"%s\"\n", textureName);
    }

    return 0;
}
