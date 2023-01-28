#ifndef THIRDPARTY__PS2TEXTURE_H
#define THIRDPARTY__PS2TEXTURE_H

#include <inttypes.h>

// Victor Suba's PS2 Texture code, from GSTextureConvert-1.1
// https://web.archive.org/web/20221008102902/https://ps2linux.no-ip.info/playstation2-linux.com/projects/ezswizzle/

// NOTE - uses global memory

// Display buffer modes
void PS2Texture_WriteTexPSMCT32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void PS2Texture_ReadTexPSMCT32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

void PS2Texture_WriteTexPSMCT16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void PS2Texture_ReadTexPSMCT16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

void PS2Texture_WriteTexPSMCT16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void PS2Texture_ReadTexPSMCT16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

// Z Buffer modes
void PS2Texture_ReadTexPSMZ32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void PS2Texture_WriteTexPSMZ32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

void PS2Texture_ReadTexPSMZ16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void PS2Texture_WriteTexPSMZ16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

void PS2Texture_ReadTexPSMZ16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void PS2Texture_WriteTexPSMZ16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

// Texture modes
void PS2Texture_WriteTexPSMT8(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void PS2Texture_ReadTexPSMT8(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

void PS2Texture_WriteTexPSMT4(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void PS2Texture_ReadTexPSMT4(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

// Swizzlers written by Sparky
// https://web.archive.org/web/20221008101555/https://ps2linux.no-ip.info/playstation2-linux.com/docs/howto/display_docef7c.html?docid=75

void PS2DeSwizzle32to8(uint8_t* d, const uint8_t* s, const int32_t width, const int32_t height);
void PS2DeSwizzle32to4(uint8_t* d, const uint8_t* s, const uint32_t width, const uint32_t height);

#endif
