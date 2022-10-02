#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tga.h"

unsigned char *LoadTGARGBFromFile(const char *filename,
	int *width, int *height)
{
	FILE* fp;
	unsigned char* sbuf;
	unsigned char* dbuf;
	unsigned char* sptr;
	unsigned char* dptr;
	unsigned char header[18];
	int filelength;
	int idlength;
	int datalength;
	int bits;
	int bytes;
	int h_start;
	int h_inc;
	int v_start;
	int v_inc;
	int x, y, w, h;
	int i, j;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf( stderr, "TGA: %s - unable to open file\n",
			filename );
		return NULL;
	}

	/* get the file length */
	fseek(fp, 0, SEEK_END);
	filelength = ftell(fp);
	rewind(fp);

	if( filelength < 18 ) {
		fprintf( stderr, "TGA: %s - header is truncated\n",
			filename );
		return NULL;
	}

	/* read in the header */
	(void) fread(header, 1, 18, fp);

	/* image id */
	idlength = header[0];

	if( filelength < 18 + idlength ) {
		fprintf( stderr, "TGA: %s - image id is truncated\n",
			filename );
		return NULL;
	}

	if (header[1] != 0) {
		/* color map not implemented */
		fprintf( stderr, "TGA: %s - unsupported color map type %d\n",
			filename, header[1] );
		return NULL;
	}

	if (header[2] != 2) {
		/* only type 2 (uncompressed, true-color image) is supported */
		fprintf( stderr, "TGA: %s - unsupported image type %d\n",
			filename, header[2] );
		return NULL;
        }

        /*
	  color map
		2 bytes - first entry index
		2 bytes - colormap length
		1 byte  - color map entry size in bits
	 */

	/* x-origin of image */
	x = header[8]  | (header[9]  << 8);

	/* y-origin of image */
	y = header[10] | (header[11] << 8);

	/* image width in pixels */
	w = header[12] | (header[13] << 8);

	/* image height in pixels */
	h = header[14] | (header[15] << 8);

	(void) x;
	(void) y;

	/* bits per pixel */
	bits = header[16];
	if (bits != 24) {
		fprintf( stderr, "TGA: %s - unsupported pixel depth %d\n",
			filename, bits );
		return NULL;
	}

	/* bytes per pixel */
	bytes = (bits + 7) / 8;

	if (header[17] & (0x80|0x40|0x0f)) {
		/* bits 6/7 are reserved, bits 0-3 are the number of alpha channel bits */
		fprintf( stderr, "TGA: %s - unsupported image descriptor 0x%02x\n",
			filename, header[17] );
		return NULL;
	}

	switch( (header[17] & 0x30) ) {
		default: /* can't happen */
		case 0x00:
			/* left to right, bottom to top ordering */
			h_start = 0;
			h_inc   = bytes;
			v_start = (h - 1) * w * bytes;
			v_inc   = -2 * w * bytes;
			break;

		case 0x10:
			/* right to left, bottom to top ordering */
			h_start = (w - 1) * bytes;
			h_inc   = -bytes;
			v_start = (h - 1) * w * bytes;
			v_inc   = -h_start;
			break;

		case 0x20:
			/* left to right, top to bottom ordering */
			h_start = 0;
			h_inc   = bytes;
			v_start = 0;
			v_inc   = 0;
			break;

		case 0x30:
			/* right to left, top to bottom ordering */
			h_start = (w - 1) * bytes;
			h_inc   = -bytes;
			v_start = 0;
			v_inc   = (w + 1) * bytes;
			break;
	}

	/* uncompressed image size */
	datalength = w * h * bytes;

	/* check image size - note: can't do this with RLE */
	if( filelength < 18 + idlength + datalength ) {
		fprintf( stderr, "TGA: %s - image data is truncated\n",
			filename );
		return NULL;
	}

	/* skip past the image id */
	fseek( fp, idlength, SEEK_CUR );

	/* allocate memory */
	sbuf = (unsigned char *)malloc(datalength);
	if( sbuf == NULL ) {
		fprintf( stderr, "TGA: %s - unable to allocate source memory\n",
			filename );
		return NULL;
	}

	dbuf = (unsigned char *)malloc(datalength);
	if( dbuf == NULL ) {
		fprintf( stderr, "TGA: %s - unable to allocate destination memory\n",
			filename );
		return NULL;
	}

	/* read in the image data */
	fread(sbuf, datalength, 1, fp);
	fclose(fp);

	/* convert the image from BGR with arbitrary ordering to RGB with top-left ordering */
	dptr = dbuf;
	sptr = sbuf + v_start + h_start;
	for (j = 0; j < h; j++) {

		for (i = 0; i < w; i++) {
			//dptr[0] = sptr[2];
			//dptr[1] = sptr[1];
			//dptr[2] = sptr[0];

			dptr[0] = sptr[0];
			dptr[1] = sptr[1];
			dptr[2] = sptr[2];

			dptr += bytes;
			sptr += h_inc;
		}

		sptr += v_inc + h_start;
	}

	free( sbuf );

	*width  = w;
	*height = h;

	return dbuf;
}

unsigned char *LoadTGARGBAFromFile(const char *filename,
	int *width, int *height)
{
	FILE *fp;
	unsigned char *buf;
	unsigned char *sptr;
	unsigned char header[18];
	int len;
	int x, y, w, h;
	int i, j;

	/* TODO - update this with the above */

	fp = fopen(filename, "rb");
	if (fp == NULL)
		return NULL;

	fread(header, 1, 18, fp);

	/* TODO handle comment */

	fseek(fp, 0, SEEK_END);
	len = ftell(fp) - 18;
	rewind(fp);
	fseek(fp, 18, SEEK_SET);

	buf = (unsigned char *)malloc(len);
	fread(buf, len, 1, fp);
	fclose(fp);

	/* TODO: verify length here */

	len = header[0];

	if (header[1] != 0)
		return NULL;

	if (header[2] != 2)
		return NULL;

	x = header[8]  | (header[9]  << 8);
	y = header[10] | (header[11] << 8);
	w = header[12] | (header[13] << 8);
	h = header[14] | (header[15] << 8);

	(void) x;
	(void) y;

	if (header[16] != 32)
		return NULL;

/* TODO handle byte 17.. */
/*
	if (header[17] != ((1<<5)|8))
		return NULL;
*/

	*width  = w;
	*height = h;

	/* swizzle the data (swap R and B so that BGR becomes RGB) */
	sptr = buf;
	for (j = 0; j < *height; j++) {
		for (i = 0; i < *width; i++) {
			// hacks for xi
			sptr[0] >>= 1;
			sptr[1] >>= 1;
			sptr[2] >>= 1;
			sptr[3] >>= 1;

			//unsigned char tmp;

			///tmp = sptr[0];
			///sptr[0] = sptr[2];
			///sptr[2] = tmp;

			sptr += 4;
		}
	}

	return buf;
}

int SaveTGARGBToFile(const char *filename, const unsigned char *data,
	int width, int height)
{
	FILE *fp;
	unsigned char header[18];
	unsigned char *buf;
	unsigned char *dptr;
	const unsigned char *sptr;
	int i, j;

	/* open the file for writing */
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		return -1;
	}

	/* allocate a temporary buffer we can swizzle into... */
	buf = (unsigned char *)malloc(width * height * 3);
	if (buf == NULL) {
		fclose(fp);

		return -1;
	}

	/* swizzle the data (swap R and B so that RGB becomes BGR) */
	sptr = data;
	dptr = buf;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			dptr[0] = sptr[2]; /* B */
			dptr[1] = sptr[1]; /* G */
			dptr[2] = sptr[0]; /* R */

			sptr += 3;
			dptr += 3;
		}
	}

	/* zero out the header */
	memset(header, 0, sizeof(header));

	header[ 2] = 2; /* data type 2 */

	header[12] = (width >> 0) & 0xFF; /* lo part of width */
	header[13] = (width >> 8) & 0xFF; /* hi part of width */

	header[14] = (height >> 0) & 0xFF; /* lo part of height */
	header[15] = (height >> 8) & 0xFF; /* hi part of height */

	header[16] = 24; /* image pixel size */

	/* image descriptor byte: attribute bit count, orientation: upper left */
	header[17] = (1<<5) | (0 & 0xf);

	/* write the 18 byte header */
	fwrite(header, 1, 18, fp);

	/* write the swizzled image data (width * height * 3 bytes worth) */
	fwrite(buf, 1, width * height * 3, fp);

	fclose(fp);
	free(buf);

	return 0;
}

int SaveTGARGBAToFile(const char *filename, const unsigned char *data,
	int width, int height)
{
	FILE *fp;
	unsigned char header[18];
	unsigned char *buf;
	unsigned char *dptr;
	const unsigned char *sptr;
	int i, j;

	/* open the file for writing */
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		return -1;
	}

	/* allocate a temporary buffer we can swizzle into... */
	buf = (unsigned char *)malloc(width * height * 4);
	if (buf == NULL) {
		fclose(fp);

		return -1;
	}

	/* swizzle the data (swap R and B so that RGBA becomes BGRA) */
	sptr = data;
	dptr = buf;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			dptr[0] = sptr[2]; /* B */
			dptr[1] = sptr[1]; /* G */
			dptr[2] = sptr[0]; /* R */
			dptr[3] = sptr[3]; /* A */

			sptr += 4;
			dptr += 4;
		}
	}

	/* zero out the header */
	memset(header, 0, sizeof(header));

	header[ 2] = 2; /* data type 2 */

	header[12] = (width >> 0) & 0xFF; /* lo part of width */
	header[13] = (width >> 8) & 0xFF; /* hi part of width */

	header[14] = (height >> 0) & 0xFF; /* lo part of height */
	header[15] = (height >> 8) & 0xFF; /* hi part of height */

	header[16] = 32; /* image pixel size */

	 /* image descriptor byte: attribute bit count, orientation: upper left */
	header[17] = (1<<5) | (8 & 0xf);

	/* write the 18 byte header */
	fwrite(header, 1, 18, fp);

	/* write the swizzled image data (width * height * 4 bytes worth) */
	fwrite(buf, 1, width * height * 4, fp);

	fclose(fp);
	free(buf);

	return 0;
}
