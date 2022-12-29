#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "bytes.h"
#include "resource.h"

#include "mld.h"


int parseMld(const chunk_t* chunk) {
    printf("\tMld \"%4s\"[%02x]: %u bytes (%08X)\n", chunk->FourCC, chunk->type, chunk->length, chunk->flags);

    const uint8_t* buf = chunk->buf + 16;

    printf("\t00: %u\n", lsb32(buf, 0x00));
    printf("\t04: %u\n", lsb32(buf, 0x04));
    printf("\t08: %u\n", lsb32(buf, 0x08));
    printf("\t0C: %u\n", lsb32(buf, 0x0C));
    printf("\t10: %u\n", lsb32(buf, 0x10));
    printf("\t14: %u\n", lsb32(buf, 0x14));
    printf("\t18: %u\n", lsb32(buf, 0x18));
    printf("\t1C: %u\n", lsb32(buf, 0x1C));
    printf("\t20: %u\n", lsb32(buf, 0x20));
    printf("\t24: %u\n", lsb32(buf, 0x24));
    printf("\t28: %u\n", lsb32(buf, 0x28));
    printf("\t2C: %u\n", lsb32(buf, 0x2C));
    printf("\t30: %u\n", lsb32(buf, 0x30));
    printf("\t34: %u\n", lsb32(buf, 0x34));
    printf("\t38: %u\n", lsb32(buf, 0x38));
    printf("\t3C: %u\n", lsb32(buf, 0x3C));

    printf("\t40: %u\n", lsb32(buf, 0x40));
    printf("\t44: %08X\n", lsb32(buf, 0x44));

    return 0;
}
