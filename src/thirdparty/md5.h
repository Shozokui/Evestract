#ifndef TEXSTRACT__MD5_H
#define TEXSTRACT__MD5_H

#include <stddef.h>
#include <inttypes.h>

#define MD5STR_LENGTH   33

int md5str(char* str, size_t strSize, const uint8_t *s, size_t sSize);

#endif
