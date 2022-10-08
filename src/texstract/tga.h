#ifndef TEXSTRACT__TGA_H
#define TEXSTRACT__TGA_H

#include <stdio.h>
#include <inttypes.h>

typedef enum TEXTURE_DATA_FORMAT {
	TEXTURE_DATA_FORMAT_B8G8R8A8,
	TEXTURE_DATA_FORMAT_B5G5R5A1,
	TEXTURE_DATA_FORMAT_I4,
	TEXTURE_DATA_FORMAT_I8,
} TEXTURE_DATA_FORMAT;

typedef enum TEXTURE_PALETTE_FORMAT {
	TEXTURE_PALETTE_FORMAT_NONE,
	TEXTURE_PALETTE_FORMAT_B8G8R8A8,
	TEXTURE_PALETTE_FORMAT_B5G5R5A1,
} TEXTURE_PALETTE_FORMAT;

typedef struct texture_data_t {
	uint8_t* TextureData;
	uint8_t* PaletteData;

	uint32_t Width;
	uint32_t Height;

	TEXTURE_DATA_FORMAT DataFormat;
	TEXTURE_PALETTE_FORMAT PaletteFormat;
} texture_data_t;

int writeTga(FILE* fp, const texture_data_t* texture);

#endif
