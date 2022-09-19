#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>

#include "bytes.h"
#include "parser.h"

int parseBmp2(const chunk_t* chunk) {

    printf("\tBmp2Chunk \"%4s\"[%02x]: %u bytes\n", chunk->FourCC, chunk->type, chunk->length);

    const uint8_t* buf = chunk->buf + 16;
    uint32_t length = chunk->length;

    uint32_t VersionFlags = lsb8(buf, 0);
    uint32_t IsCompressed = VersionFlags >> 7;
    uint32_t Version = (VersionFlags >> 4) & 7;

    char textureName[17];
    memset(textureName, 0, sizeof(textureName));
    strncpy(textureName, cptr8(buf, 1), 16);

    uint32_t Width = lsb32(buf, 1, 20);
    uint32_t Height = lsb32(buf, 1, 24);
    uint32_t BitsPerPixel = lsb8(buf, 1, 30);
    uint32_t PaletteBitsPerPixel = lsb8(buf, 1, 52);
    uint32_t Flags = 0;
    uint32_t FourCC = 0;
    uint32_t PaletteOffset = 0;
    uint32_t TextureOffset = 0;
    uint32_t CompressedTextureOffset = 0;
    uint32_t UnknownDxt0 = 0;
    uint32_t UnknownDxt1 = 0;

    // uint32_t TextureSize = 0;
    // uint32_t PaletteSize = 0;
    uint32_t CompressedTextureSize = 0;

    printf("\tBmp2 Texture Name: \"%s\"\n", textureName);
    printf("\tBmp2 Length: %u\n", length);
    printf("\tBmp2 IsCompressed: %u\n", IsCompressed);
    printf("\tBmp2 Version: %u\n", Version);
    printf("\tBmp2 Width: %u\n", Width);
    printf("\tBmp2 Height: %u\n", Height);
    printf("\tBmp2 BitsPerPixel: %u\n", BitsPerPixel);
    printf("\tBmp2 PaletteBitsPerPixel: %u\n", PaletteBitsPerPixel);

    uint32_t offset = 57;

    if (Version == 3) {
        Flags = lsb32(buf, offset);
        offset += 4;
    }
    if (Version != 2) {
        if (BitsPerPixel == 4) {
            PaletteOffset = offset;

            offset += 16 * PaletteBitsPerPixel / 8;
        }
        if (BitsPerPixel == 8) {
            PaletteOffset = offset;

            offset += 256 * PaletteBitsPerPixel / 8;
        }

        TextureOffset = offset;
        offset += BitsPerPixel * Width * Height / 8;
    }

    if (IsCompressed && offset < length - 4) {
        FourCC = lsb32(buf, offset);
        offset += 4;

        if (FourCC >= 0x44585431 && FourCC <= 0x44585435) {
            UnknownDxt0 = lsb32(buf, offset, 0);
            UnknownDxt1 = lsb32(buf, offset, 4);
            offset += 8;

            CompressedTextureOffset = offset;

            if (FourCC == 0x44585431) {
                CompressedTextureSize = 8 * Width * Height / 16;
            } else if (FourCC == 0x44585432 || FourCC == 0x44585433) {
                CompressedTextureSize = 16 * Width * Height / 16;
            } else if (FourCC == 0x44585434 || FourCC == 0x44585435) {
                CompressedTextureSize = 16 * Width * Height / 16;
            }

            offset += CompressedTextureSize;
        }
    }

    printf("\tBmp2 Flags: %08x\n", Flags);
    printf("\tBmp2 FourCC: %08x\n", FourCC);
    printf("\tBmp2 PaletteOffset: %u\n", PaletteOffset);
    printf("\tBmp2 TextureOffset: %u\n", TextureOffset);
    printf("\tBmp2 CompressedTextureOffset: %u\n", CompressedTextureOffset);

    // compressed image size
    printf("\tBmp2 UnknownDxt0: %u\n", UnknownDxt0);

    // block width in bytes
    printf("\tBmp2 UnknownDxt1: %u\n", UnknownDxt1);

    // printf("\tBmp2 Remainder: %d\n", (int32_t) length - (int32_t) offset);

    return 0;
}
