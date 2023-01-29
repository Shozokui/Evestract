#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "tga.h"

int writeTga(FILE* fp, const texture_data_t* textureData) {

	uint32_t Width = textureData->Width;
	uint32_t Height = textureData->Height;

	if (textureData->PaletteFormat == TEXTURE_PALETTE_FORMAT_B8G8R8A8) {
		// ID Length - 0 bytes
		fputc(0x00, fp);

		// Color Map Type: 1 (Color map present)
		fputc(0x01, fp);

		// Image Type: 1 (Uncompressed, Color-mapped)
		fputc(0x01, fp);

		// Color map starting index (0)
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Color map length (256)
		fputc(0x00, fp);
		fputc(0x01, fp);

		// Color map entry size (32)
		fputc(0x20, fp);

		// X-origin
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Y-origin
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Image width
		fputc(Width & 0xFF, fp);
		fputc(Width >> 8, fp);

		// Image height
		fputc(Height & 0xFF, fp);
		fputc(Height >> 8, fp);

		// Pixel depth
		fputc(0x08, fp);

		// Image Descriptor (upper left origin)
		fputc(0x20, fp);

		// Image Identification (omitted, 0 bytes)

		size_t PaletteSize = 256 * 4;

		size_t written = fwrite(textureData->PaletteData, 1, PaletteSize, fp);
		if (written != PaletteSize) {
			fprintf(stderr, "Texture is truncated (TGA Color Map!!!)!\n");
		}

		uint32_t BitsPerPixel = 8;
		uint32_t TextureSize = BitsPerPixel * Width * Height / 8;

		written = fwrite(textureData->TextureData, 1, TextureSize, fp);
		if (written != TextureSize) {
			printf("aTexture is truncated (TGA Image Data!!!); %zu written, %u expected!\n", written, TextureSize);
		}

		return 0;
	}

	if (textureData->DataFormat == TEXTURE_DATA_FORMAT_B5G5R5A1) {
		// ID Length - 0 bytes
		fputc(0x00, fp);

		// Color Map Type: 0 (No color map)
		fputc(0x00, fp);

		// Image Type: 2 (Uncompressed, True-color)
		fputc(0x02, fp);

		// Color map starting index (0)
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Color map length (0)
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Color map entry size (0)
		fputc(0x00, fp);

		// X-origin
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Y-origin
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Image width
		fputc(Width & 0xFF, fp);
		fputc(Width >> 8, fp);

		// Image height
		fputc(Height & 0xFF, fp);
		fputc(Height >> 8, fp);

		// Pixel depth
		fputc(0x10, fp);

		// Image Descriptor (upper left origin, 1 alpha bit)
		fputc(0x21, fp);

		// Image Identification (omitted, 0 bytes)

		uint32_t BitsPerPixel = 16;
		uint32_t TextureSize = BitsPerPixel * Width * Height / 8;

		size_t written = fwrite(textureData->TextureData, 1, TextureSize, fp);
		if (written != TextureSize) {
			printf("Texture is truncated (TGA Image Data!!!); %zu written, %u expected!\n", written, TextureSize);
		}
	}

	if (textureData->DataFormat == TEXTURE_DATA_FORMAT_B8G8R8A8) {
		// ID Length - 0 bytes
		fputc(0x00, fp);

		// Color Map Type: 0 (No color map)
		fputc(0x00, fp);

		// Image Type: 2 (Uncompressed, True-color)
		fputc(0x02, fp);

		// Color map starting index (0)
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Color map length (0)
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Color map entry size (0)
		fputc(0x00, fp);

		// X-origin
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Y-origin
		fputc(0x00, fp);
		fputc(0x00, fp);

		// Image width
		fputc(Width & 0xFF, fp);
		fputc(Width >> 8, fp);

		// Image height
		fputc(Height & 0xFF, fp);
		fputc(Height >> 8, fp);

		// Pixel depth
		fputc(0x20, fp);

		// Image Descriptor (upper left origin, 8 alpha bits)
		fputc(0x28, fp);

		// Image Identification (omitted, 0 bytes)

		uint32_t BitsPerPixel = 32;
		uint32_t TextureSize = BitsPerPixel * Width * Height / 8;

		size_t written = fwrite(textureData->TextureData, 1, TextureSize, fp);
		if (written != TextureSize) {
			printf("Texture is truncated (TGA Image Data!!!); %zu written, %u expected!\n", written, TextureSize);
		}
	}

	return 0;
}
