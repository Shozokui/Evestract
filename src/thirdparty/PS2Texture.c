#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "PS2Texture.h"

// Victor Suba's PS2 Texture code, from GSTextureConvert-1.1
// https://web.archive.org/web/20221008102902/https://ps2linux.no-ip.info/playstation2-linux.com/projects/ezswizzle/

// NOTE - uses global memory

static unsigned int gsmem[1024 * 1024];

static const int block32[32] =
{
	 0,  1,  4,  5, 16, 17, 20, 21,
	 2,  3,  6,  7, 18, 19, 22, 23,
	 8,  9, 12, 13, 24, 25, 28, 29,
	10, 11, 14, 15, 26, 27, 30, 31
};


static const int columnWord32[16] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15
};

void PS2Texture_WriteTexPSMCT32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	unsigned int *src = (unsigned int *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 32;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 32);

			int blockX = px / 8;
			int blockY = py / 8;
			int block  = block32[blockX + blockY * 8];

			int bx = px - blockX * 8;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord32[cx + cy * 8];

			gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw] = *src;
			src++;
		}
	}
}

void PS2Texture_ReadTexPSMCT32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	unsigned int *src = (unsigned int *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 32;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 32);

			int blockX = px / 8;
			int blockY = py / 8;
			int block  = block32[blockX + blockY * 8];

			int bx = px - blockX * 8;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord32[cx + cy * 8];

			*src = gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			src++;
		}
	}
}

static const int blockZ32[32] =
{
	 24, 25, 28, 29, 8, 9, 12, 13,
	 26, 27, 30, 31,10, 11, 14, 15,
	 16, 17, 20, 21, 0, 1, 4, 5,
	 18, 19, 22, 23, 2, 3, 6, 7
};

static const int columnWordZ32[16] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15
};

void PS2Texture_WriteTexPSMZ32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	unsigned int *src = (unsigned int *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 32;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 32);

			int blockX = px / 8;
			int blockY = py / 8;
			int block  = blockZ32[blockX + blockY * 8];

			int bx = px - blockX * 8;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ32[cx + cy * 8];

			gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw] = *src;
			src++;
		}
	}
}

void PS2Texture_ReadTexPSMZ32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	unsigned int *src = (unsigned int *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 32;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 32);

			int blockX = px / 8;
			int blockY = py / 8;
			int block  = blockZ32[blockX + blockY * 8];

			int bx = px - blockX * 8;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ32[cx + cy * 8];

			*src = gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			src++;
		}
	}
}

static const int block16[32] =
{
	 0,  2,  8, 10,
	 1,  3,  9, 11,
	 4,  6, 12, 14,
	 5,  7, 13, 15,
	16, 18, 24, 26,
	17, 19, 25, 27,
	20, 22, 28, 30,
	21, 23, 29, 31
};

static const int columnWord16[32] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
};

static const int columnHalf16[32] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1
};

void PS2Texture_WriteTexPSMCT16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = block16[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord16[cx + cy * 16];
			int ch = columnHalf16[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			dst[ch] = *src;
			src++;
		}
	}
}

void PS2Texture_ReadTexPSMCT16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = block16[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord16[cx + cy * 16];
			int ch = columnHalf16[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			*src = dst[ch];
			src++;
		}
	}
}

static const int blockZ16[32] =
{
	 24,  26,  16, 18,
	 25,  27,  17, 19,
    28,  30, 20, 22,
	 29,  31, 21, 23,
	8, 10, 0, 2,
	9, 11, 1, 3,
	12, 14, 4, 6,
	13, 15, 5, 7
};

static const int columnWordZ16[32] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
};

static const int columnHalfZ16[32] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1
};

void PS2Texture_WriteTexPSMZ16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = blockZ16[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ16[cx + cy * 16];
			int ch = columnHalfZ16[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			dst[ch] = *src;
			src++;
		}
	}
}

void PS2Texture_ReadTexPSMZ16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = blockZ16[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ16[cx + cy * 16];
			int ch = columnHalfZ16[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			*src = dst[ch];
			src++;
		}
	}
}

static const int blockZ16S[32] =
{
	 24,  26,  8, 10,
	 25,  27,  9, 11,
	 16,  18, 0, 2,
	 17,  19, 1, 3,
	28, 30, 12, 14,
	29, 31, 13, 15,
	20, 22, 4, 6,
	21, 23, 5, 7
};

static const int columnWordZ16S[32] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
};

static const int columnHalfZ16S[32] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1
};

void PS2Texture_WriteTexPSMZ16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = blockZ16S[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ16S[cx + cy * 16];
			int ch = columnHalfZ16S[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			dst[ch] = *src;
			src++;
		}
	}
}

void PS2Texture_ReadTexPSMZ16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = blockZ16S[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ16S[cx + cy * 16];
			int ch = columnHalfZ16S[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			*src = dst[ch];
			src++;
		}
	}
}

static const int block16S[32] =
{
	 0,  2, 16, 18,
	 1,  3, 17, 19,
	 8, 10, 24, 26,
	 9, 11, 25, 27,
	 4,  6, 20, 22,
	 5,  7, 21, 23,
	12, 14, 28, 30,
	13, 15, 29, 31
};

static const int columnWord16S[32] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
};

static const int columnHalf16S[32] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1
};

void PS2Texture_WriteTexPSMCT16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = block16S[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord16S[cx + cy * 16];
			int ch = columnHalf16S[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			dst[ch] = *src;
			src++;
		}
	}
}

void PS2Texture_ReadTexPSMCT16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = block16S[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord16S[cx + cy * 16];
			int ch = columnHalf16S[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			*src = dst[ch];
			src++;
		}
	}
}

static const int block8[32] =
{
	 0,  1,  4,  5, 16, 17, 20, 21,
	 2,  3,  6,  7, 18, 19, 22, 23,
	 8,  9, 12, 13, 24, 25, 28, 29,
	10, 11, 14, 15, 26, 27, 30, 31
};

static const int columnWord8[2][64] =
{
	{
		 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
		 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,

		 8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,
		10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7
	},
	{
		 8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,
		10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,

		 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
		 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
	}
};

static const int columnByte8[64] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0,  2, 2, 2, 2, 2, 2, 2, 2,

	1, 1, 1, 1, 1, 1, 1, 1,  3, 3, 3, 3, 3, 3, 3, 3,
	1, 1, 1, 1, 1, 1, 1, 1,  3, 3, 3, 3, 3, 3, 3, 3
};

void PS2Texture_WriteTexPSMT8(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	dbw >>= 1;
	unsigned char *src = (unsigned char *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 128;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 128);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 16;
			int block  = block8[blockX + blockY * 8];

			int bx = px - (blockX * 16);
			int by = py - (blockY * 16);

			int column = by / 4;

			int cx = bx;
			int cy = by - column * 4;
			int cw = columnWord8[column & 1][cx + cy * 16];
			int cb = columnByte8[cx + cy * 16];

			unsigned char *dst = (unsigned char *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			dst[cb] = *src++;
		}
	}
}

void PS2Texture_ReadTexPSMT8(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
    dbw >>= 1;
	unsigned char *src = (unsigned char *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 128;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 128);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 16;
			int block  = block8[blockX + blockY * 8];

			int bx = px - blockX * 16;
			int by = py - blockY * 16;

			int column = by / 4;

			int cx = bx;
			int cy = by - column * 4;
			int cw = columnWord8[column & 1][cx + cy * 16];
			int cb = columnByte8[cx + cy * 16];

			unsigned char *dst = (unsigned char *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			*src = dst[cb];
			src++;
		}
	}
}

static const int block4[32] =
{
	 0,  2,  8, 10,
	 1,  3,  9, 11,
	 4,  6, 12, 14,
	 5,  7, 13, 15,
	16, 18, 24, 26,
	17, 19, 25, 27,
	20, 22, 28, 30,
	21, 23, 29, 31
};

static const int columnWord4[2][128] =
{
	{
		 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
		 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,

		 8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,
		10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7
	},
	{
		 8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,
		10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,

		 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
		 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
	}
};

static const int columnByte4[128] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  2, 2, 2, 2, 2, 2, 2, 2,  4, 4, 4, 4, 4, 4, 4, 4,  6, 6, 6, 6, 6, 6, 6, 6,
	0, 0, 0, 0, 0, 0, 0, 0,  2, 2, 2, 2, 2, 2, 2, 2,  4, 4, 4, 4, 4, 4, 4, 4,  6, 6, 6, 6, 6, 6, 6, 6,

	1, 1, 1, 1, 1, 1, 1, 1,  3, 3, 3, 3, 3, 3, 3, 3,  5, 5, 5, 5, 5, 5, 5, 5,  7, 7, 7, 7, 7, 7, 7, 7,
	1, 1, 1, 1, 1, 1, 1, 1,  3, 3, 3, 3, 3, 3, 3, 3,  5, 5, 5, 5, 5, 5, 5, 5,  7, 7, 7, 7, 7, 7, 7, 7
};

void PS2Texture_WriteTexPSMT4(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	dbw >>= 1;
	unsigned char *src = (unsigned char *)data;
	int startBlockPos = dbp * 64;

	bool odd = false;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 128;
			int pageY = y / 128;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 128);
			int py = y - (pageY * 128);

			int blockX = px / 32;
			int blockY = py / 16;
			int block  = block4[blockX + blockY * 4];

			int bx = px - blockX * 32;
			int by = py - blockY * 16;

			int column = by / 4;

			int cx = bx;
			int cy = by - column * 4;
			int cw = columnWord4[column & 1][cx + cy * 32];
			int cb = columnByte4[cx + cy * 32];

			unsigned char *dst = (unsigned char *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];

			if(cb & 1)
			{
				if(odd)
					dst[cb >> 1] = (dst[cb >> 1] & 0x0f) | ((*src) & 0xf0);
				else
					dst[cb >> 1] = (dst[cb >> 1] & 0x0f) | (((*src) << 4) & 0xf0);
			}
			else
			{
				if(odd)
					dst[cb >> 1] = (dst[cb >> 1] & 0xf0) | (((*src) >> 4) & 0x0f);
				else
					dst[cb >> 1] = (dst[cb >> 1] & 0xf0) | ((*src) & 0x0f);
			}

			if(odd)
				src++;

			odd = !odd;
		}
	}
}

void PS2Texture_ReadTexPSMT4(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	dbw >>= 1;
	unsigned char *src = (unsigned char *)data;
	int startBlockPos = dbp * 64;

	bool odd = false;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 128;
			int pageY = y / 128;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 128);
			int py = y - (pageY * 128);

			int blockX = px / 32;
			int blockY = py / 16;
			int block  = block4[blockX + blockY * 4];

			int bx = px - blockX * 32;
			int by = py - blockY * 16;

			int column = by / 4;

			int cx = bx;
			int cy = by - column * 4;
			int cw = columnWord4[column & 1][cx + cy * 32];
			int cb = columnByte4[cx + cy * 32];

			unsigned char *dst = (unsigned char *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];

			if(cb & 1)
			{
				if(odd)
					*src = ((*src) & 0x0f) | (dst[cb >> 1] & 0xf0);
				else
					*src = ((*src) & 0xf0) | ((dst[cb >> 1] >> 4) & 0x0f);
			}
			else
			{
				if(odd)
					*src = ((*src) & 0x0f) | ((dst[cb >> 1] << 4) & 0xf0);
				else
					*src = ((*src) & 0xf0) | (dst[cb >> 1] & 0x0f);
			}

			if(odd)
				src++;

			odd = !odd;
		}
	}
}

// Swizzlers written by Sparky
// https://web.archive.org/web/20221008101555/https://ps2linux.no-ip.info/playstation2-linux.com/docs/howto/display_docef7c.html?docid=75

#if 0
void PS2DeSwizzle32to8(uint8_t* d, const uint8_t* s, const int32_t width, const int32_t height) {
   // this function works for the following resolutions
   // Width:       any multiple of 16 smaller then or equal to 4096
   // Height:      any multiple of 4 smaller then or equal to 4096

   // the texels must be uploaded as a 32bit texture
   // width_32bit = width_8bit / 2
   // height_32bit = height_8bit / 2
   // remember to adjust the mapping coordinates when
   // using a dimension which is not a power of two

   for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
         const int32_t block_location = (y&(~0xf))*width + (x&(~0xf))*2;
         const int32_t swap_selector = (((y+2)>>2)&0x1)*4;
         const int32_t posY = (((y&(~3))>>1) + (y&1))&0x7;
         const int32_t column_location = posY*width*2 + ((x+swap_selector)&0x7)*4;

         const int32_t byte_num = ((y>>1)&1) + ((x>>2)&2);     // 0,1,2,3

        uint8_t srcPix = s[block_location + column_location + byte_num];

        d[y*width+x] = srcPix;
      }
   }
}

void PS2DeSwizzle32to4(uint8_t* d, const uint8_t* s, const uint32_t width, const uint32_t height) {
   // this function works for the following resolutions
   // Width:       32, 64, 96, 128, any multiple of 128 smaller then or equal to 4096
   // Height:      16, 32, 48, 64, 80, 96, 112, 128, any multiple of 128 smaller then or equal to 4096

   // the texels must be uploaded as a 32bit texture
   // width_32bit = height_4bit / 2
   // height_32bit = width_4bit / 4
   // remember to adjust the mapping coordinates when
   // using a dimension which is not a power of two

   for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++) {
         // get the pen
         const uint32_t index = y*width+x;

         // swizzle
         const uint32_t pageX = x & (~0x7f);
         const uint32_t pageY = y & (~0x7f);

         const uint32_t pages_horz = (width+127)/128;
         const uint32_t pages_vert = (height+127)/128;

         const uint32_t page_number = (pageY/128)*pages_horz + (pageX/128);

         const uint32_t page32Y = (page_number/pages_vert)*32;
         const uint32_t page32X = (page_number%pages_vert)*64;

         const uint32_t page_location = page32Y*height*2 + page32X*4;

         const uint32_t locX = x & 0x7f;
         const uint32_t locY = y & 0x7f;

         const uint32_t block_location = ((locX&(~0x1f))>>1)*height + (locY&(~0xf))*2;
         const uint32_t swap_selector = (((y+2)>>2)&0x1)*4;
         const uint32_t posY = (((y&(~3))>>1) + (y&1))&0x7;

         const uint32_t column_location = posY*height*2 + ((x+swap_selector)&0x7)*4;

         const uint32_t byte_num = (x>>3)&3;     // 0,1,2,3
         const uint32_t bits_set = (y>>1)&1;     // 0,1            (lower/upper 4 bits)

         const uint8_t srcVal = s[page_location + block_location + column_location + byte_num];
         const uint8_t srcPix = (srcVal & (0xf << (bits_set*4))) >> (bits_set*4);

         uint8_t* destPtr = &d[(index>>1)];
         uint8_t destMask = 0xF << ((index&1)*4);

         destPtr[0] = (destPtr[0] & ~destMask) | (srcPix << ((index&1)*4));
      }
   }
}
#endif
