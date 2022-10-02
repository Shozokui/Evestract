#ifndef TGA_H
#define TGA_H

unsigned char *LoadTGARGBFromFile(const char *filename,
	int *width, int *height);
unsigned char *LoadTGARGBAFromFile(const char *filename,
	int *width, int *height);
int SaveTGARGBToFile(const char *filename, const unsigned char *data,
	int width, int height);
int SaveTGARGBAToFile(const char *filename, const unsigned char *data,
	int width, int height);

#endif
