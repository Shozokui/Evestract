#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>

#include "bytes.h"
#include "parser.h"

static void getFourCC(const uint8_t* buf, char* out) {
    uint8_t a = lsb8(buf);
    uint8_t b = lsb8(buf, 1);
    uint8_t c = lsb8(buf, 2);
    uint8_t d = lsb8(buf, 3);

    if (!(isalnum(a) || ispunct(a) || a == ' ') || !(isalnum(b) || ispunct(b) || b == ' ') || !(isalnum(c) || ispunct(c) || c == ' ') || !(isalnum(d) || ispunct(d) || d == ' ')) {
        sprintf(out, "#$%02x%02x%02x%02x", a, b, c, d);
    } else {
        sprintf(out, "#'%c%c%c%c'", a, b, c, d);
    }
}

typedef void (*disSchedInst)(const uint8_t* buf, uint32_t inst, uint32_t instLen, uint32_t iLen0, uint32_t iLen1);

static void disSched02(const uint8_t* buf, uint32_t inst, uint32_t instLen, uint32_t iLen0, uint32_t iLen1) {
    // 02 FourCC
    // 03 <generator ptr>
    (void) inst;
    (void) instLen;

    char fourCC[32];

    getFourCC(&buf[8], fourCC);

    if (iLen0 != 0 || iLen1 != 0) {
        printf("\t\t_02(%d, %d, %s);\n", iLen0, iLen1, fourCC);
    } else {
        printf("\t\t_02(%s);\n", fourCC);
    }
}

static disSchedInst disSchedFunc[256] = {
    [0x02] = disSched02,
};

static void disSched(const uint8_t* buf) {
    for (;;) {
        uint32_t inst = buf[0];
        uint32_t instLen = buf[1];
        uint32_t unk0 = lsb16(buf, 0x02);

        uint32_t iLen0 = lsb16(buf, 0x04);
        uint32_t iLen1 = lsb16(buf, 0x06);

        if (unk0 != 0) {
            printf("\t\tUnk0: %04X\n", unk0);
        }

        disSchedInst func = disSchedFunc[inst];
        if (func != NULL) {
            func(buf, inst, instLen, iLen0, iLen1);
        } else {
            printf("\t\t_%02X(%d, %d", inst, iLen0, iLen1);
            for (uint32_t i = 2; i < instLen; i++) {
                printf(", 0x%08X", lsb32(buf, i * 4));
            }
            printf(");\n");
        }

        buf += (instLen * 4);

        if (inst == 0) {
            break;
        }
    }
}

int parseScheduler(const chunk_t* chunk) {
    const uint8_t* buf = chunk->buf;

    uint32_t aOff = lsb32(buf, 0x20);
    uint32_t bOff = lsb32(buf, 0x24);
    uint32_t cOff = lsb32(buf, 0x28);
    uint32_t sLen = lsb32(buf, 0x2C);

    printf("\tsLen: %u\n", sLen);

    printf("\taOff: %u\n", aOff);
    disSched(&buf[aOff]);

    printf("\tbOff: %u\n", bOff);
    disSched(&buf[bOff]);

    printf("\tcOff: %u\n", cOff);
    disSched(&buf[cOff]);

    return 0;
}
