#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>

#include "bytes.h"
#include "parser.h"

#include "bmp2.h"

#include "md5.h"
#include "resource.h"


typedef struct dds_pixelformat_t {
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwABitMask;
} dds_pixelformat_t;

typedef struct dds_header_t {
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    dds_pixelformat_t ddspf;
    uint32_t dwCaps;
    uint32_t dwCaps2;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
} dds_header_t;

typedef struct dds_t {
    uint32_t dwMagic;
    dds_header_t header;
} dds_t;

static void dds_init(dds_t* dds) {
    memset(dds, 0, sizeof(dds_t));

    dds->dwMagic = 0x20534444; // 0x44445320;
    dds->header.dwSize = sizeof(dds_header_t);
    dds->header.ddspf.dwSize = sizeof(dds_pixelformat_t);
}

int convertBmp2(const chunk_t* chunk) {

    const uint8_t* buf = chunk->buf + 16;
    uint32_t length = chunk->length - 16;

    uint32_t VersionFlags = lsb8(buf, 0);
    uint32_t IsCompressed = VersionFlags >> 7;
    uint32_t Version = (VersionFlags >> 4) & 7;

    char textureName[17];
    memset(textureName, 0, sizeof(textureName));
    strncpy(textureName, cptr8(buf, 1), 16);

    uint32_t Width = lsb32(buf, 1, 20);
    uint32_t Height = lsb32(buf, 1, 24);

    // bits per pixel in the stored version of the texture
    uint32_t BitsPerPixel = lsb8(buf, 1, 30);

    // bits per pixel in the decoded version of the texture
    uint32_t PaletteBitsPerPixel = lsb8(buf, 1, 52);

    uint32_t Flags = 0;
    uint32_t FourCC = 0;
    uint32_t PaletteOffset = 0;
    uint32_t TextureOffset = 0;
    uint32_t CompressedTextureOffset = 0;
    uint32_t UnknownDxt0 = 0;
    uint32_t LinearSize = 0;

    // uint32_t TextureSize = 0;
    // uint32_t PaletteSize = 0;
    uint32_t CompressedTextureSize = 0;

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

        // BPP 16, 24, 32 are not paletted.

        TextureOffset = offset;
    }

    if (IsCompressed) {
        // This flag may be set for textures not actually compressed.
        // Verify against the first four bytes.
        FourCC = lsb32(buf, offset);

        if (FourCC >= 0x44585431 && FourCC <= 0x44585435) {
            // DXT magic present.
            offset += 4;

            UnknownDxt0 = lsb32(buf, offset, 0);
            LinearSize = lsb32(buf, offset, 4);
            offset += 8;

            TextureOffset = 0;
            CompressedTextureOffset = offset;

            if (FourCC == 0x44585431) {
                CompressedTextureSize = 8 * Width * Height / 16;
            } else if (FourCC == 0x44585432 || FourCC == 0x44585433) {
                CompressedTextureSize = 16 * Width * Height / 16;
            } else if (FourCC == 0x44585434 || FourCC == 0x44585435) {
                CompressedTextureSize = 16 * Width * Height / 16;
            }
        } else {
            // Unknown FourCC; assuming not actually compressed.
            FourCC = 0;
        }
    }

    if (FourCC != 0) {
        char filename[65536];

        char md5[MD5STR_LENGTH];

        md5str(md5, sizeof(md5), chunk->buf, chunk->length);

        snprintf(filename, sizeof(filename), "%s-%4s-%16s-%s.dds", GetChunkTypeName(chunk->type), chunk->FourCC, textureName, md5);

        FILE* fp = fopen(filename, "wb");
        if (fp == NULL) {
            fprintf(stderr, "Unable to open \"%s\" for writing!\n", filename);
        }

        if (fp != NULL) {
            // size_t written = fwrite(chunk->buf, 1, chunk->length, fp);
            // if (written != chunk->length) {
            //     fprintf(stderr, "\"%s\" is truncated!\n", filename);
            // }

            dds_t dds;
            dds_init(&dds);

            // Caps
            // Height
            // Width
            // Pixel Format
            // Linear Size
            dds.header.dwFlags = 1 | 2 | 4 | 0x1000 | 0x80000;

            dds.header.dwHeight = Height;
            dds.header.dwWidth = Width;
            dds.header.dwPitchOrLinearSize = LinearSize;

            // Texture
            dds.header.dwCaps = 0x1000;

            dds.header.ddspf.dwFlags = 4;
            dds.header.ddspf.dwFourCC =
                ((FourCC >> 24) & 0xFF) | (((FourCC >> 16) & 0xFF) << 8) | (((FourCC >> 8) & 0xFF) << 16) | (((FourCC >> 0) & 0xFF) << 24);

            // todo - don't look!
            size_t written = fwrite(&dds, 1, sizeof(dds), fp);

            if (written != sizeof(dds)) {
                fprintf(stderr, "\"%s\" is truncated (DDS header!!!)!\n", filename);
            }

            written = fwrite(&buf[CompressedTextureOffset], 1, CompressedTextureSize, fp);
            if (written != CompressedTextureSize) {
                fprintf(stderr, "\"%s\" is truncated (DDS!!!)!\n", filename);
            }

            fclose(fp);
        }
    } else {
        printf("\tBmp2Chunk \"%4s\"[%02x]: %u bytes\n", chunk->FourCC, chunk->type, chunk->length);

        printf("\tBmp2 Texture Name: \"%s\"\n", textureName);
        printf("\tBmp2 Length: %u\n", length);
        printf("\tBmp2 IsCompressed: %u\n", IsCompressed);
        printf("\tBmp2 Version: %u\n", Version);
        printf("\tBmp2 Width: %u\n", Width);
        printf("\tBmp2 Height: %u\n", Height);
        printf("\tBmp2 BitsPerPixel: %u\n", BitsPerPixel);
        printf("\tBmp2 PaletteBitsPerPixel: %u\n", PaletteBitsPerPixel);

        printf("\tBmp2 Flags: %08x\n", Flags);
        printf("\tBmp2 FourCC: %08x\n", FourCC);
        printf("\tBmp2 PaletteOffset: %u\n", PaletteOffset);
        printf("\tBmp2 TextureOffset: %u\n", TextureOffset);
        printf("\tBmp2 CompressedTextureOffset: %u\n", CompressedTextureOffset);

        // compressed image size
        printf("\tBmp2 UnknownDxt0: %u\n", UnknownDxt0);

        // block width in bytes
        printf("\tBmp2 LinearSize: %u\n", LinearSize);
    }

    return 0;
}
