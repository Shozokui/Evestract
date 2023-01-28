
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>

#include "bytes.h"
#include "parser.h"

#include "PS2Texture.h"

#include "mtx.h"

#include "md5.h"
#include "tga.h"


int convertMtx(const chunk_t* chunk) {

    const uint8_t* buf = chunk->buf + 16;
    uint32_t length = chunk->length - 16;

    uint32_t offset = 0;

    char textureName[17];
    memset(textureName, 0, sizeof(textureName));
    strncpy(textureName, cptr8(buf, 0), 16);

    uint32_t Width = lsb16(buf, 0x14);
    uint32_t Height = lsb16(buf, 0x16);

    // Some of these formats require additional handling.
    // For example:
    // - the font texture with a palette for each horizontal half
    // - the 16 bit textures split in half vertically

    uint32_t _0x10 = lsb32(buf, 0x10);
    bool Is4B16P = (_0x10 == 0x00000105);
    bool Is4B32P = (_0x10 == 0x00000106) || (_0x10 == 0x00000206);
    bool Is8B16P = (_0x10 == 0x00000109);
    bool Is8B32P = (_0x10 == 0x0000010A) || (_0x10 == 0x0000210A);
    bool Is16B = (_0x10 == 0x0000000C);
    bool Is32B = (_0x10 == 0x00000110);

    bool IsSwizzled = !((_0x10 == 0x0000210A) || (_0x10 == 0x00000110));

    int TextureGSHeaderLength = 0x80;
    int PaletteGSHeaderLength = 0x60;

    if (_0x10 == 0x00000206) {
        TextureGSHeaderLength += 32;
        PaletteGSHeaderLength += 64 + 96;
    }

    printf("\tMtxChunk \"%4s\"[%02x]: %u bytes\n", chunk->FourCC, chunk->type, chunk->length);
    printf("\tMtx Texture Name: \"%s\"\n", textureName);
    printf("\tMtx Width: %u\n", Width);
    printf("\tMtx Height: %u\n", Height);

    printf("\tTexture? @ %08x:\n", 0x10);
    for (int i = 0x10; i < 0x10 + TextureGSHeaderLength; i += 16) {
        printf("\t\t");
        for (int j = 0; j < 16; j += 4) {
            printf("%02X%02X%02X%02X ", lsb8(buf, i + j + 0), lsb8(buf, i + j + 1), lsb8(buf, i + j + 2), lsb8(buf, i + j + 3));
        }
        printf("\n");
    }

    if (!(Is4B16P || Is4B32P || Is8B16P || Is8B32P || Is16B || Is32B)) {
        printf("\t*** Skipping this texture!\n");
        return 0;
    }

    bool HasPalette = Is4B16P || Is4B32P || Is8B16P || Is8B32P;

    int PaletteSize = 0;
    int BitsPerPixel = 0;
    int DestBitsPerPixel = 0;

    if (Is4B16P) {
        BitsPerPixel = 4;

        // pre-converted for compatability issues?
        DestBitsPerPixel = 8;

        PaletteSize = 16 * 2;
    }

    if (Is4B32P) {
        BitsPerPixel = 4;

        // pre-converted for compatability issues?
        DestBitsPerPixel = 8;

        PaletteSize = 16 * 4;
    }

    if (Is8B16P) {
        BitsPerPixel = 8;
        DestBitsPerPixel = 8;
        PaletteSize = 256 * 2;
    }

    if (Is8B32P) {
        BitsPerPixel = 8;
        DestBitsPerPixel = 8;
        PaletteSize = 256 * 4;
    }

    if (Is16B) {
        BitsPerPixel = 16;
        DestBitsPerPixel = 16;
    }

    if (Is32B) {
        BitsPerPixel = 32;
        DestBitsPerPixel = 32;
    }

    offset = 0x10 + TextureGSHeaderLength;

    uint32_t TextureSize = BitsPerPixel * Width * Height / 8;
    uint32_t DestTextureSize = DestBitsPerPixel * Width * Height / 8;

    uint8_t* tmpBuf = (uint8_t*) calloc(1, DestTextureSize);
    uint8_t* tmpPal = HasPalette ? (uint8_t*) calloc(1, 256 * 4) : NULL;

    const uint8_t* p = &buf[offset];
    offset += TextureSize;

    if (BitsPerPixel == 4) {
        uint8_t* tmpBufOrig = (uint8_t*) calloc(1, DestTextureSize);

        PS2DeSwizzle32to4(tmpBufOrig, p, Width, Height);

        const uint8_t* s = tmpBufOrig;
        uint8_t* d = tmpBuf;

        for (uint32_t y = 0; y < Height; y++) {
            for (uint32_t w = 0; w < Width; w += 2) {
                d[0] = s[0] & 0xF;
                d[1] = s[0] >> 4;

                s++;
                d += 2;
            }
        }

        free(tmpBufOrig);
    } else if (BitsPerPixel == 8) {
        if (IsSwizzled) {
            PS2DeSwizzle32to8(tmpBuf, p, Width, Height);
        } else {
            memcpy(tmpBuf, p, DestTextureSize);
        }
    } else if (BitsPerPixel == 16) {
        const uint8_t* s = p;
        uint8_t* d = tmpBuf;

        for (uint32_t i = 0; i < Width * Height; i++) {
            uint16_t color = s[0] | (s[1] << 8);

            uint16_t dc = (color & 0x83E0) | ((color >> 10) & 0x1F) | ((color & 0x1F) << 10);

            d[0] = dc & 0xFF;
            d[1] = dc >> 8;

            d += 2;
            s += 2;
        }
    } else {
        const uint8_t* s = p;
        uint8_t* d = tmpBuf;

        for (uint32_t i = 0; i < Width * Height; i++) {
            d[0] = s[2];
            d[1] = s[1];
            d[2] = s[0];
            d[3] = s[3];

            d += 4;
            s += 4;
        }
    }

    if (HasPalette) {
        printf("\tPalette? @ %08x:\n", offset);
        for (int i = 0x00; i < PaletteGSHeaderLength; i += 16) {
            printf("\t\t");
            for (int j = 0; j < 16; j += 4) {
                printf("%02X%02X%02X%02X ", lsb8(buf, offset, i + j + 0), lsb8(buf, offset, i + j + 1), lsb8(buf, offset, i + j + 2), lsb8(buf, offset, i + j + 3));
            }
            printf("\n");
        }

        offset += PaletteGSHeaderLength;

        p = &buf[offset];
        offset += PaletteSize;

        if (Is4B16P) {
            for (int i = 0; i < 16; i++) {
                int c = i;

                uint16_t color = (p[c * 2 + 0] << 0) | (p[c * 2 + 1] << 8);

                // pre-converted for compatability issues?
                tmpPal[i * 4 + 0] = ((color >> 10) & 0x1F) << 3;
                tmpPal[i * 4 + 1] = ((color >>  5) & 0x1F) << 3;
                tmpPal[i * 4 + 2] = ((color >>  0) & 0x1F) << 3;
                tmpPal[i * 4 + 3] =  (color >> 15) ? 0xFF : 0x00;
            }
        }

        if (Is4B32P) {
            for (int i = 0; i < 16; i++) {
                int c = i;

                // pre-converted for compatability issues?
                tmpPal[i * 4 + 2] = p[c * 4 + 0];
                tmpPal[i * 4 + 1] = p[c * 4 + 1];
                tmpPal[i * 4 + 0] = p[c * 4 + 2];
                tmpPal[i * 4 + 3] = p[c * 4 + 3];
            }
        }

        if (Is8B16P) {
            for (int i = 0; i < 256; i++) {
                int b0 = i & 0x08;
                int b1 = i & 0x10;
                int c = (i & ~0x18) | (b1 >> 1) | (b0 << 1);

                uint16_t color = (p[c * 2 + 0] << 0) | (p[c * 2 + 1] << 8);

                tmpPal[i * 4 + 0] = ((color >> 10) & 0x1F) << 3;
                tmpPal[i * 4 + 1] = ((color >>  5) & 0x1F) << 3;
                tmpPal[i * 4 + 2] = ((color >>  0) & 0x1F) << 3;
                tmpPal[i * 4 + 3] =  (color >> 15) ? 0xFF : 0x00;
            }
        }

        if (Is8B32P) {
            for (int i = 0; i < 256; i++) {
                int b0 = i & 0x08;
                int b1 = i & 0x10;
                int c = (i & ~0x18) | (b1 >> 1) | (b0 << 1);

                // pre-converted for compatability issues?
                tmpPal[i * 4 + 2] = p[c * 4 + 0];
                tmpPal[i * 4 + 1] = p[c * 4 + 1];
                tmpPal[i * 4 + 0] = p[c * 4 + 2];
                tmpPal[i * 4 + 3] = p[c * 4 + 3];
            }
        }
    }

    printf("\tTrailer? @ %08x:\n", offset);
    for (uint32_t i = offset; i < length; i += 16) {
        printf("\t\t");
        for (int j = 0; j < 16; j += 4) {
            printf("%02X%02X%02X%02X ", lsb8(buf, i + j + 0), lsb8(buf, i + j + 1), lsb8(buf, i + j + 2), lsb8(buf, i + j + 3));
        }
        printf("\n");
    }

    char filename[65536];

    char md5[MD5STR_LENGTH];

    md5str(md5, sizeof(md5), chunk->buf, chunk->length);

    snprintf(filename, sizeof(filename), "%s-%4s-%16s-%s.tga", GetChunkTypeName(chunk->type), chunk->FourCC, textureName, md5);

    texture_data_t textureData;
    memset(&textureData, 0, sizeof(textureData));

    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open \"%s\" for writing!\n", filename);
    }

    if (fp != NULL) {
        textureData.TextureData = tmpBuf;
        textureData.PaletteData = tmpPal;

        textureData.Width = Width;
        textureData.Height = Height;

        if (Is4B16P) {
            // pre-converted for compatability issues?
            textureData.DataFormat = TEXTURE_DATA_FORMAT_I8;

            // pre-converted for compatability issues?
            textureData.PaletteFormat = TEXTURE_PALETTE_FORMAT_B8G8R8A8;
        }

        if (Is4B32P) {
            // pre-converted for compatability issues?
            textureData.DataFormat = TEXTURE_DATA_FORMAT_I8;

            // pre-converted for compatability issues?
            textureData.PaletteFormat = TEXTURE_PALETTE_FORMAT_B8G8R8A8;
        }

        if (Is8B16P) {
            textureData.DataFormat = TEXTURE_DATA_FORMAT_I8;

            // pre-converted for compatability issues?
            textureData.PaletteFormat = TEXTURE_PALETTE_FORMAT_B8G8R8A8;
        }

        if (Is8B32P) {
            textureData.DataFormat = TEXTURE_DATA_FORMAT_I8;
            textureData.PaletteFormat = TEXTURE_PALETTE_FORMAT_B8G8R8A8;
        }

        if (Is16B) {
            textureData.DataFormat = TEXTURE_DATA_FORMAT_B5G5R5A1;
            textureData.PaletteFormat = TEXTURE_PALETTE_FORMAT_NONE;
        }

        if (Is32B) {
            textureData.DataFormat = TEXTURE_DATA_FORMAT_B8G8R8A8;
            textureData.PaletteFormat = TEXTURE_PALETTE_FORMAT_NONE;
        }

        writeTga(fp, &textureData);

        fclose(fp);
    }

    free(tmpPal);
    free(tmpBuf);

    return 0;
}
