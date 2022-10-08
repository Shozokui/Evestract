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

static void disEnd(const uint8_t* buf, uint32_t inst, uint32_t instLen, uint32_t iLen0, uint32_t iLen1) {
    (void) buf;
    (void) inst;
    (void) instLen;

    if (instLen != 1) {
        printf("\t\tWARNING: OpCode Length %u unexpected!\n", instLen);
    }

    if (iLen0 != 0 || iLen1 != 0) {
        printf("\t\tEND(%d, %d);\n", iLen0, iLen1);
    } else {
        printf("\t\tEND();\n");
    }
}

static void disIdleEnd(const uint8_t* buf, uint32_t inst, uint32_t instLen, uint32_t iLen0, uint32_t iLen1) {
    (void) buf;
    (void) inst;
    (void) instLen;

    if (instLen != 2) {
        printf("\t\tWARNING: OpCode Length %u unexpected!\n", instLen);
    }

    if (iLen0 != 0 || iLen1 != 0) {
        printf("\t\tEND(%d, %d);\n", iLen0, iLen1);
    } else {
        printf("\t\tEND();\n");
    }
}

static void disIdleInit(const uint8_t* buf, uint32_t inst, uint32_t instLen, uint32_t iLen0, uint32_t iLen1) {
    (void) buf;
    (void) inst;
    (void) instLen;

    if (instLen != 2) {
        printf("\t\tWARNING: OpCode Length %u unexpected!\n", instLen);
    }

    if (iLen0 != 0 || iLen1 != 0) {
        printf("\t\tINIT(%d, %d);\n", iLen0, iLen1);
    } else {
        printf("\t\tINIT();\n");
    }
}

static void disSched02(const uint8_t* buf, uint32_t inst, uint32_t instLen, uint32_t iLen0, uint32_t iLen1) {
    // 02 FourCC
    // 03 <generator ptr>
    (void) inst;
    (void) instLen;

    char fourCC[32];

    if (instLen != 4) {
        printf("\t\tWARNING: OpCode Length %u unexpected!\n", instLen);
    }

    getFourCC(&buf[8], fourCC);

    if (iLen0 != 0 || iLen1 != 0) {
        printf("\t\t_02(%d, %d, %s);\n", iLen0, iLen1, fourCC);
    } else {
        printf("\t\t_02(%s);\n", fourCC);
    }
}

static void disSched03(const uint8_t* buf, uint32_t inst, uint32_t instLen, uint32_t iLen0, uint32_t iLen1) {
    // 02 FourCC
    // 03 <generator ptr>
    (void) inst;
    (void) instLen;

    char fourCC[32];

    if (instLen != 4) {
        printf("\t\tWARNING: OpCode Length %u unexpected!\n", instLen);
    }

    getFourCC(&buf[8], fourCC);

    if (iLen0 != 0 || iLen1 != 0) {
        printf("\t\t_03(%d, %d, %s);\n", iLen0, iLen1, fourCC);
    } else {
        printf("\t\t_03(%s);\n", fourCC);
    }
}

static void disSched0A(const uint8_t* buf, uint32_t inst, uint32_t instLen, uint32_t iLen0, uint32_t iLen1) {
    // 02 FourCC
    // 03 <generator ptr>
    (void) inst;
    (void) instLen;

    char fourCC[32];

    getFourCC(&buf[8], fourCC);

    if (iLen0 != 0 || iLen1 != 0) {
        printf("\t\t_0A(%d, %d, %s, 0x%08X, 0x%08X, 0x%08X, 0x%08X);\n", iLen0, iLen1, fourCC, lsb32(buf, 16), lsb32(buf, 20), lsb32(buf, 24), lsb32(buf, 28));
    } else {
        printf("\t\t_0A(%s, 0x%08X, 0x%08X, 0x%08X, 0x%08X);\n", fourCC, lsb32(buf, 16), lsb32(buf, 20), lsb32(buf, 24), lsb32(buf, 28));
    }
}

static void disSched0B(const uint8_t* buf, uint32_t inst, uint32_t instLen, uint32_t iLen0, uint32_t iLen1) {
    // 02 FourCC
    // 03 <generator ptr>
    (void) inst;
    (void) instLen;

    char fourCC[32];

    getFourCC(&buf[8], fourCC);

    if (iLen0 != 0 || iLen1 != 0) {
        printf("\t\t_0B(%d, %d, %s, 0x%08X, 0x%08X, 0x%08X, 0x%08X);\n", iLen0, iLen1, fourCC, lsb32(buf, 16), lsb32(buf, 20), lsb32(buf, 24), lsb32(buf, 28));
    } else {
        printf("\t\t_0B(%s, 0x%08X, 0x%08X, 0x%08X, 0x%08X);\n", fourCC, lsb32(buf, 16), lsb32(buf, 20), lsb32(buf, 24), lsb32(buf, 28));
    }
}

static void disSched73(const uint8_t* buf, uint32_t inst, uint32_t instLen, uint32_t iLen0, uint32_t iLen1) {
    // 02 FourCC
    // 03 <generator ptr>
    (void) inst;
    (void) instLen;

    char fourCC[32];

    getFourCC(&buf[8], fourCC);

    if (iLen0 != 0 || iLen1 != 0) {
        printf("\t\t_73(%d, %d, %s);\n", iLen0, iLen1, fourCC);
    } else {
        printf("\t\t_73(%s);\n", fourCC);
    }
}

static disSchedInst disInitFunc[256] = {
    [0x00] = disEnd,
};

static disSchedInst disIdleFunc[256] = {
    [0x00] = disIdleEnd,
    [0x01] = disIdleInit,
    [0x02] = disSched02,
    [0x03] = disSched03,
    [0x0A] = disSched0A,
    [0x0B] = disSched0B,
    [0x73] = disSched73,
};

static disSchedInst disDieFunc[256] = {
    [0x00] = disEnd,
};

static void disInit(const uint8_t* buf) {
    for (;;) {
        uint32_t v = lsb32(buf, 0);
        uint32_t inst = v & 0xFF;
        uint32_t instLen = (v >> 8) & 0x1F;
        uint32_t unk0 = v >> 13;

        uint32_t iLen0 = 0; //lsb16(buf, 0x04);
        uint32_t iLen1 = 0; //lsb16(buf, 0x06);

        if (unk0 != 0) {
            printf("\t\tUnk0: %04X\n", unk0);
        }

        disSchedInst func = disInitFunc[inst];
        if (func != NULL) {
            func(buf, inst, instLen, iLen0, iLen1);
        } else {
            printf("\t\t_%02X(%d, %d", inst, iLen0, iLen1);
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

static void disIdle(const uint8_t* buf) {
    for (;;) {
        uint32_t v = lsb32(buf, 0);
        uint32_t inst = v & 0xFF;
        uint32_t instLen = (v >> 8) & 0x1F;
        uint32_t unk0 = v >> 13;

        uint32_t iLen0 = lsb16(buf, 0x04);
        uint32_t iLen1 = lsb16(buf, 0x06);

        if (unk0 != 0) {
            printf("\t\tUnk0: %04X\n", unk0);
        }

        disSchedInst func = disIdleFunc[inst];
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

static void disDie(const uint8_t* buf) {
    for (;;) {
        uint32_t v = lsb32(buf, 0);
        uint32_t inst = v & 0xFF;
        uint32_t instLen = (v >> 8) & 0x1F;
        uint32_t unk0 = v >> 13;

        uint32_t iLen0 = 0; //lsb16(buf, 0x04);
        uint32_t iLen1 = 0; //lsb16(buf, 0x06);

        if (unk0 != 0) {
            printf("\t\tUnk0: %04X\n", unk0);
        }

        disSchedInst func = disDieFunc[inst];
        if (func != NULL) {
            func(buf, inst, instLen, iLen0, iLen1);
        } else {
            printf("\t\t_%02X(%d, %d", inst, iLen0, iLen1);
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

int parseScheduler(const chunk_t* chunk) {
    const uint8_t* buf = chunk->buf;

    uint32_t offInit = lsb32(buf, 0x20);
    uint32_t offIdle = lsb32(buf, 0x24);
    uint32_t offDie = lsb32(buf, 0x28);
    uint32_t sLen = lsb32(buf, 0x2C);

    printf("\tsLen: %u\n", sLen);

    printf("\tInit:\n");
    disInit(&buf[offInit]);

    printf("\tIdle:\n");
    disIdle(&buf[offIdle]);

    printf("\tDie:\n");
    disDie(&buf[offDie]);

    return 0;
}
