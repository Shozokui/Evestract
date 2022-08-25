#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "bytes.h"
#include "ftable.h"
#include "readfile.h"


typedef struct chunk_t {
    uint32_t type;
    uint32_t length;
    uint32_t flags;

    const uint8_t* buf;

    char FourCC[5];

} chunk_t;

static int LoadChunkedResource(const uint8_t* buf, uint32_t bufLen, int (*callback)(const chunk_t*, void*), void* userData) {

    // Chunk header
    //
    // off   name
    // 00    FourCC
    // 04    Flags | Length | Type
    // 08    zero [internal use only]
    // 0C    zero [internal use only]

    uint32_t bufOff = 0;
    int chunkIndex = 0;
    int ret;

    // The list of resources should be bookended by Rmp and Terminate chunks.
    int ok = 0;
    if (bufLen > 32) {
        if (buf[4] == 1 && buf[5] == 1 && buf[6] == 0 && buf[7] == 0) {
            static const uint8_t end[16] = { 0x65, 0x6E, 0x64, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            if (memcmp(&buf[bufLen-16], end, 16) == 0) {
                ok = 1;
            }
        }
    }

    if (!ok) {
        return -2;
    }

    while (bufOff < bufLen) {
        if ((bufLen - bufOff) < 16) {
            fprintf(stderr, "File underrun\n");
            return -1;
        }

        char FourCC[5];

        FourCC[0] = lsb8(buf, bufOff, 0);
        FourCC[1] = lsb8(buf, bufOff, 1);
        FourCC[2] = lsb8(buf, bufOff, 2);
        FourCC[3] = lsb8(buf, bufOff, 3);
        FourCC[4] = 0;

        uint32_t header = lsb32(buf, bufOff, 4);

        uint32_t type = header & 0x7f;
        uint32_t length = (((header >> 7) & 0x7ffff) << 4) - 16;

        // bits 26 and 27 are internal use only
        uint32_t flags = (header >> 28);

        if ((bufLen - bufOff - 16) < length) {
            // Should only happen in a corrupt DAT.
            fprintf(stderr, "[%d] Skipping chunk [%s] (%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x)\n",
                chunkIndex,
                FourCC,
                lsb8(buf, bufOff, 0),
                lsb8(buf, bufOff, 1),
                lsb8(buf, bufOff, 2),
                lsb8(buf, bufOff, 3),
                lsb8(buf, bufOff, 4),
                lsb8(buf, bufOff, 5),
                lsb8(buf, bufOff, 6),
                lsb8(buf, bufOff, 7),
                lsb8(buf, bufOff, 8),
                lsb8(buf, bufOff, 9),
                lsb8(buf, bufOff, 10),
                lsb8(buf, bufOff, 11),
                lsb8(buf, bufOff, 12),
                lsb8(buf, bufOff, 13),
                lsb8(buf, bufOff, 14),
                lsb8(buf, bufOff, 15)
                );

            return -1;
        } else {
            if (callback != NULL) {
                ret = callback(&(chunk_t) {
                        .type = type,
                        .length = length,
                        .flags = flags,
                        .FourCC = { FourCC[0], FourCC[1], FourCC[2], FourCC[3], FourCC[4] }, // best laid plans, etc.
                        .buf = ptr8(buf, bufOff)
                    },
                    userData);

                if (ret < 0) {
                    return ret;
                }
            }
        }

        bufOff += length + 16;
        chunkIndex++;
    }

    return 0;
}

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

static int parseSchedular(const chunk_t* chunk) {
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

typedef struct parse_chunk_userdata_t {
    chunk_t stack[16];
    int stackIndex;
} parse_chunk_userdata_t;

static int parseChunk(const chunk_t* chunk, void* _userData) {
    parse_chunk_userdata_t* userData = (parse_chunk_userdata_t*) _userData;

    if (chunk->type == 1) {
        // Rmp, push onto the stack
        int index = userData->stackIndex + 1;

        if (index < 0 || (size_t) index >= sizeof(userData->stack) / sizeof(userData->stack[0])) {
            // Overflow
            return -1;
        }

        userData->stack[index] = *chunk;
        userData->stackIndex = index;
    } else if (chunk->type == 0) {
        // Terminate, pop from the stack
        int index = userData->stackIndex - 1;

        if (index < -2) {
            // Underflow
            // (-1 is allowed for the two dats that seem to have an extra Terminate?)
            return -1;
        }

        userData->stackIndex = index;
    } else {
        int index = userData->stackIndex;

        if (index < 0) {
            // Underflow
            // (An extra trailing Terminate is tolerated, but there should never be any actual resources
            // outside of a Rmp/Terminate pair)
            return -1;
        }

        if (chunk->type == 5 || chunk->type == 7) {
            for (int index = 0; index <= userData->stackIndex; index++) {
                const chunk_t* c = &userData->stack[index];

                if (c->flags != 0) {
                    printf("'%s'[%01X] > ", c->FourCC, c->flags);
                } else {
                    printf("'%s' > ", c->FourCC);
                }
            }

            if (chunk->flags != 0) {
                printf("'%s'.%s[%01X]:\n", chunk->FourCC, chunk->type == 5 ? "g" : "s", chunk->flags);
            } else {
                printf("'%s'.%s:\n", chunk->FourCC, chunk->type == 5 ? "g" : "s");
            }

            if (chunk->type == 5) {
                // Generator
            } else if (chunk->type == 7) {
                // Schedular
                parseSchedular(chunk);
            }

            printf("\n");
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {

    // basepath fileid
    // filename

    if (argc != 2 && argc != 3) {
        fprintf(stderr, "usage: %s <chunked.dat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char filename[65536];
    if (argc == 2) {
        strncpy(filename, argv[1], 65535);
        filename[65535] = '\0';
    } else {
        const char* basepath = argv[1];
        int fileId = atoi(argv[2]);

        ftable_t* ftable;

        int ret = InitFTable(&ftable, basepath);
        if (ret < 0) {
            fprintf(stderr, "%s: unable to init FTABLE at %s\n", argv[0], basepath);
            exit(EXIT_FAILURE);
        }

        ret = GetDatPath(ftable, filename, 65536, basepath, fileId);
        if (ret < 1) {
            fprintf(stderr, "%s: unable to query fileId %u\n", argv[0], fileId);
            exit(EXIT_FAILURE);
        }
        free(ftable);
    }

    uint32_t datLen;

    uint8_t* datBuf = ReadFile(filename, &datLen);
    if (datBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], filename);
        exit(EXIT_FAILURE);
    }

    int ret = LoadChunkedResource(datBuf, datLen, &parseChunk, &(parse_chunk_userdata_t) {
        .stackIndex = -1,
    });

    if (ret < 0) {
        fprintf(stderr, "%s: errors encountered parsing %s\n", argv[0], filename);
        exit(EXIT_FAILURE);
    }

    free(datBuf);

    return 0;
}
