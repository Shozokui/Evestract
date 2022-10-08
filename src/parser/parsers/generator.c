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

typedef void (*disGeneratorInst)(const uint8_t* buf, uint32_t inst, uint32_t instLen);

static void disEnd(const uint8_t* buf, uint32_t inst, uint32_t instLen) {
    (void) buf;
    (void) inst;

    if (instLen != 1) {
        printf("\t\tWARNING: OpCode Length %u unexpected!\n", instLen);
    }

    printf("\t\tEND();\n");
}

static void disGenerateInit(const uint8_t* buf, uint32_t inst, uint32_t instLen) {
    // 01 Flags
    // 02 <Resource Ptr>
    // 03 Resource FourCC
    (void) inst;

    char fourCC[32];

    if (instLen != 12) {
        printf("\t\tWARNING: OpCode Length %u unexpected!\n", instLen);
    }

    getFourCC(&buf[12], fourCC);

    printf("\t\tINIT(0x%08X, %s, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%08X);\n", lsb32(buf, 4), fourCC, lsb32(buf, 16), lsb32(buf, 20), lsb32(buf, 24), lsb32(buf, 28), lsb8(buf, 32), lsb8(buf, 33), lsb8(buf, 34), lsb8(buf, 35), lsb32(buf, 36));
}

static disGeneratorInst disInitFunc[256] = {
    [0x00] = disEnd,
};

static disGeneratorInst disGenerateFunc[256] = {
    [0x00] = disEnd,
    [0x01] = disGenerateInit,
};

static disGeneratorInst disIdleFunc[256] = {
    [0x00] = disEnd,
};

static disGeneratorInst disDieFunc[256] = {
    [0x00] = disEnd,
};

static void disGenerator(const uint8_t* buf, disGeneratorInst* generatorFuncs) {
    for (;;) {
        uint32_t v = lsb32(buf, 0);
        uint32_t inst = v & 0xFF;
        uint32_t instLen = (v >> 8) & 0x1F;
        uint32_t unk0 = v >> 13;

        if (instLen == 0) {
            printf("bad\n");
            return;
        }

        disGeneratorInst func = generatorFuncs[inst];
        if (func != NULL) {
            func(buf, inst, instLen);
        } else {
            printf("\t\t_%02X(%3d", inst, unk0);
            for (uint32_t i = 1; i < instLen; i++) {
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

static void disInit(const uint8_t* buf) {
    disGenerator(buf, disInitFunc);
}

static void disGenerate(const uint8_t* buf) {
    disGenerator(buf, disGenerateFunc);
}

static void disIdle(const uint8_t* buf) {
    disGenerator(buf, disIdleFunc);
}

static void disDie(const uint8_t* buf) {
    disGenerator(buf, disDieFunc);
}

int parseGenerator(const chunk_t* chunk) {
    const uint8_t* buf = chunk->buf;

    uint32_t offInit = lsb32(buf, 0x80);
    uint32_t offGenerate = lsb32(buf, 0x84);
    uint32_t offIdle = lsb32(buf, 0x88);
    uint32_t offDie = lsb32(buf, 0x8C);

    if (offInit > 0) {
        printf("\tInit:\n");
        disInit(&buf[offInit]);
    }

    if (offGenerate > 0) {
        printf("\tGenerate:\n");
        disGenerate(&buf[offGenerate]);
    }

    if (offIdle > 0) {
        printf("\tIdle:\n");
        disIdle(&buf[offIdle]);
    }

    if (offDie > 0) {
        printf("\tDie:\n");
        disDie(&buf[offDie]);
    }

    return 0;
}
