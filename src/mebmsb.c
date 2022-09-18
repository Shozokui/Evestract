#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>

#include "bytes.h"
#include "ftable.h"
#include "readfile.h"


typedef enum CHUNK_TYPE {
    CHUNK_TYPE_Terminate,
    CHUNK_TYPE_Rmp,
    CHUNK_TYPE_Rmw,
    CHUNK_TYPE_Directory,
    CHUNK_TYPE_Bin,
    CHUNK_TYPE_Generator,
    CHUNK_TYPE_Camera,
    CHUNK_TYPE_Scheduler,
    CHUNK_TYPE_Mtx,
    CHUNK_TYPE_Tim,
    CHUNK_TYPE_TexInfo,
    CHUNK_TYPE_Vum,
    CHUNK_TYPE_Oml,
    CHUNK_TYPE_FileInfo,
    CHUNK_TYPE_Anm,
    CHUNK_TYPE_Rsd,
    CHUNK_TYPE_UnKnown,
    CHUNK_TYPE_Osm,
    CHUNK_TYPE_Skd,
    CHUNK_TYPE_Mtd,
    CHUNK_TYPE_Mld,
    CHUNK_TYPE_Mlt,
    CHUNK_TYPE_Mws,
    CHUNK_TYPE_Mod,
    CHUNK_TYPE_Tim2,
    CHUNK_TYPE_KeyFrame,
    CHUNK_TYPE_Bmp,
    CHUNK_TYPE_Bmp2,
    CHUNK_TYPE_Mzb,
    CHUNK_TYPE_Mmd,
    CHUNK_TYPE_Mep,
    CHUNK_TYPE_D3m,
    CHUNK_TYPE_D3s,
    CHUNK_TYPE_D3a,
    CHUNK_TYPE_DistProg,
    CHUNK_TYPE_VuLineProg,
    CHUNK_TYPE_RingProg,
    CHUNK_TYPE_D3b,
    CHUNK_TYPE_Asn,
    CHUNK_TYPE_Mot,
    CHUNK_TYPE_Skl,
    CHUNK_TYPE_Sk2,
    CHUNK_TYPE_Os2,
    CHUNK_TYPE_Mo2,
    CHUNK_TYPE_Ps2,
    CHUNK_TYPE_Wsd,
    CHUNK_TYPE_Mmb,
    CHUNK_TYPE_Weather,
    CHUNK_TYPE_Meb,
    CHUNK_TYPE_Msb,
    CHUNK_TYPE_Med,
    CHUNK_TYPE_Msh,
    CHUNK_TYPE_Ysh,
    CHUNK_TYPE_Mpb,
    CHUNK_TYPE_Rid,
    CHUNK_TYPE_Wd,
    CHUNK_TYPE_Bgm,
    CHUNK_TYPE_Lfd,
    CHUNK_TYPE_Lfe,
    CHUNK_TYPE_Esh,
    CHUNK_TYPE_Sch,
    CHUNK_TYPE_Sep,
    CHUNK_TYPE_Vtx,
    CHUNK_TYPE_Lwo,
    CHUNK_TYPE_Rme,
    CHUNK_TYPE_Elt,
    CHUNK_TYPE_Rab,
    CHUNK_TYPE_Mtt,
    CHUNK_TYPE_Mtb,
    CHUNK_TYPE_Cib,
    CHUNK_TYPE_Tlt,
    CHUNK_TYPE_PointLightProg,
    CHUNK_TYPE_Mgd,
    CHUNK_TYPE_Mgb,
    CHUNK_TYPE_Sph,
    CHUNK_TYPE_Bmd,
    CHUNK_TYPE_Qif,
    CHUNK_TYPE_Qdt,
    CHUNK_TYPE_Mif,
    CHUNK_TYPE_Mdt,
    CHUNK_TYPE_Sif,
    CHUNK_TYPE_Sdt,
    CHUNK_TYPE_Acd,
    CHUNK_TYPE_Acb,
    CHUNK_TYPE_Afb,
    CHUNK_TYPE_Aft,
    CHUNK_TYPE_Wwd,
    CHUNK_TYPE_NullProg,
    CHUNK_TYPE_Spw,
    CHUNK_TYPE_Fud,
    CHUNK_TYPE_DisgregaterProg,
    CHUNK_TYPE_Smt,
    CHUNK_TYPE_DamValueProg,
    CHUNK_TYPE_Bp,
    CHUNK_TYPE_Unknown1,
    CHUNK_TYPE_Unknown2,
    CHUNK_TYPE_Max,
} CHUNK_TYPE;

typedef struct chunk_t {
    CHUNK_TYPE type;
    uint32_t length;
    uint32_t flags;

    const uint8_t* buf;

    char FourCC[5];

} chunk_t;

static const char* ChunkTypeNames[] = {
    [0x00] = "Terminate",
    [0x01] = "Rmp",
    [0x02] = "Rmw",
    [0x03] = "Directory",
    [0x04] = "Bin",
    [0x05] = "Generator",
    [0x06] = "Camera",
    [0x07] = "Scheduler",
    [0x08] = "Mtx",
    [0x09] = "Tim",
    [0x0a] = "TexInfo",
    [0x0b] = "Vum",
    [0x0c] = "Oml",
    [0x0d] = "FileInfo",
    [0x0e] = "Anm",
    [0x0f] = "Rsd",
    [0x10] = "UnKnown",
    [0x11] = "Osm",
    [0x12] = "Skd",
    [0x13] = "Mtd",
    [0x14] = "Mld",
    [0x15] = "Mlt",
    [0x16] = "Mws",
    [0x17] = "Mod",
    [0x18] = "Tim2",
    [0x19] = "KeyFrame",
    [0x1a] = "Bmp",
    [0x1b] = "Bmp2",
    [0x1c] = "Mzb",
    [0x1d] = "Mmd",
    [0x1e] = "Mep",
    [0x1f] = "D3m",
    [0x20] = "D3s",
    [0x21] = "D3a",
    [0x22] = "DistProg",
    [0x23] = "VuLineProg",
    [0x24] = "RingProg",
    [0x25] = "D3b",
    [0x26] = "Asn",
    [0x27] = "Mot",
    [0x28] = "Skl",
    [0x29] = "Sk2",
    [0x2a] = "Os2",
    [0x2b] = "Mo2",
    [0x2c] = "Ps2",
    [0x2d] = "Wsd",
    [0x2e] = "Mmb",
    [0x2f] = "Weather",
    [0x30] = "Meb",
    [0x31] = "Msb",
    [0x32] = "Med",
    [0x33] = "Msh",
    [0x34] = "Ysh",
    [0x35] = "Mpb",
    [0x36] = "Rid",
    [0x37] = "Wd",
    [0x38] = "Bgm",
    [0x39] = "Lfd",
    [0x3a] = "Lfe",
    [0x3b] = "Esh",
    [0x3c] = "Sch",
    [0x3d] = "Sep",
    [0x3e] = "Vtx",
    [0x3f] = "Lwo",
    [0x40] = "Rme",
    [0x41] = "Elt",
    [0x42] = "Rab",
    [0x43] = "Mtt",
    [0x44] = "Mtb",
    [0x45] = "Cib",
    [0x46] = "Tlt",
    [0x47] = "PointLightProg",
    [0x48] = "Mgd",
    [0x49] = "Mgb",
    [0x4a] = "Sph",
    [0x4b] = "Bmd",
    [0x4c] = "Qif",
    [0x4d] = "Qdt",
    [0x4e] = "Mif",
    [0x4f] = "Mdt",
    [0x50] = "Sif",
    [0x51] = "Sdt",
    [0x52] = "Acd",
    [0x53] = "Acb",
    [0x54] = "Afb",
    [0x55] = "Aft",
    [0x56] = "Wwd",
    [0x57] = "NullProg",
    [0x58] = "Spw",
    [0x59] = "Fud",
    [0x5a] = "DisgregaterProg",
    [0x5b] = "Smt",
    [0x5c] = "DamValueProg",
    [0x5d] = "Bp",
    [0x5e] = "Unknown1",
    [0x5f] = "Unknown2",
    [0x60] = "Max",
};

static const char* GetChunkTypeName(CHUNK_TYPE type) {
    if (type < CHUNK_TYPE_Max) {
        return ChunkTypeNames[type];
    }
    return "<Invalid>";
}

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

static int parseMeb(const chunk_t* chunk) {
    const uint8_t* buf = chunk->buf;

    int offset = 16;

    char name[17];
    strncpy(name, (const char*) &buf[offset], 16);
    name[16] = '\0';
    offset += 16;

    printf("\t\tName: \"%s\"\n", name);

    int numElems = lsb8(buf, offset);
    int numChildren = lsb8(buf, offset + 1);
    offset += 16;

    printf("\t\tNumElem: %d\n", numElems);

    for (int i = 0; i < numElems; i++) {
        printf("\t\tElem %d:\n", i);

        int elemSize = lsb16(buf, offset, 0);
        int elemEnd = offset + elemSize;
        int x = lsb16(buf, offset, 2);
        int y = lsb16(buf, offset, 4);
        int _06 = lsb16(buf, offset, 6);
        int _08 = lsb16(buf, offset, 8);
        int w = lsb16(buf, offset, 10);
        int h = lsb16(buf, offset, 12);

        int numSubElem = lsb8(buf, offset, 20);
        int menuHelpLen = lsb8(buf, offset, 21);
        int menuLabelLen = lsb8(buf, offset, 22);

        offset += 32;

        printf("\t\t\tSize: %d\n", elemSize);
        printf("\t\t\tDimensions: (%d, %d) (%d, %d)\n", x, y, x + w, y + h);
        printf("\t\t\t%d %d\n", _06, _08);
        printf("\t\t\tHelp Len: %d\n", menuHelpLen);
        printf("\t\t\tLabel Len: %d\n", menuLabelLen);

        printf("\t\t\tNumSubElem: %d\n", numSubElem);
        for (int j = 0; j < numSubElem; j++) {
            printf("\t\t\tSumElem %d:\n", j);

            int _00 = lsb16(buf, offset, 0);
            int _02 = lsb16(buf, offset, 2);

            offset += 4;

            char name[17];
            strncpy(name, (const char*) &buf[offset], 16);
            name[16] = '\0';
            offset += 16;

            printf("\t\t\t\tSubElem Name: \"%s\"\n", name);
            printf("\t\t\t\t%d, %d\n", _00, _02);
        }

        if (menuHelpLen > 0) {
            strncpy(name, (const char*) &buf[offset], 16);
            name[16] = '\0';

            printf("\t\t\tMenu Help Id: \"%s\"\n", name);
        }

        offset += menuHelpLen + 1;

        if (menuLabelLen > 0) {
             strncpy(name, (const char*) &buf[offset], 16);
            name[16] = '\0';

            printf("\t\t\tMenu Label Id: \"%s\"\n", name);           
        }

        // most likely just structure padding.
        // just in case.
        // offset += menuLabelLen + 1;
        //
        // if (offset != elemEnd) {
        //    printf("\t\t\t%d extra bytes left\n", elemEnd - offset);
        // }

        offset = elemEnd;
    }

    printf("\t\tNumChildren: %d\n", numChildren);

    for (int i = 0; i < numChildren; i++) {
        printf("\t\tChild %d:\n", i);

        int elemSize = lsb16(buf, offset, 0);
        int elemEnd = offset + elemSize;

        int numSubElem = lsb8(buf, offset, 27);
        int menuHelpLen = lsb8(buf, offset, 29);
        int menuLabelLen = lsb8(buf, offset, 30);

        offset += 32;

        printf("\t\t\tSize: %d\n", elemSize);
        printf("\t\t\tHelp Len: %d\n", menuHelpLen);
        printf("\t\t\tLabel Len: %d\n", menuLabelLen);

        printf("\t\t\tNumSubChild: %d\n", numSubElem);
        for (int j = 0; j < numSubElem; j++) {
            printf("\t\t\tSumChild %d:\n", j);

            int _00 = lsb16(buf, offset, 0);
            int _02 = lsb16(buf, offset, 2);

            offset += 4;

            char name[17];
            strncpy(name, (const char*) &buf[offset], 16);
            name[16] = '\0';
            offset += 16;

            printf("\t\t\t\tSubChild Name: \"%s\"\n", name);
            printf("\t\t\t\t%d, %d\n", _00, _02);
        }

        if (menuHelpLen > 0) {
            strncpy(name, (const char*) &buf[offset], 16);
            name[16] = '\0';

            printf("\t\t\tMenu Help Id: \"%s\"\n", name);
        }

        offset += menuHelpLen + 1;

        if (menuLabelLen > 0) {
             strncpy(name, (const char*) &buf[offset], 16);
            name[16] = '\0';

            printf("\t\t\tMenu Label Id: \"%s\"\n", name);           
        }

        // most likely just structure padding.
        // just in case.
        // offset += menuLabelLen + 1;
        //
        // if (offset != elemEnd) {
        //    printf("\t\t\t%d extra bytes left\n", elemEnd - offset);
        // }

        offset = elemEnd;
    }

    return 0;
}

static int parseMsb(const chunk_t* chunk) {
    const uint8_t* buf = chunk->buf;

    int offset = 16;

    char name[17];
    strncpy(name, (const char*) &buf[offset], 16);
    name[16] = '\0';
    offset += 16;

    printf("\t\tName: \"%s\"\n", name);

    int numElems = lsb8(buf, offset);
    offset += 1;

    printf("\t\tNumElem: %d\n", numElems);

    for (int i = 0; i < numElems; i++) {
        strncpy(name, (const char*) &buf[offset], 16);
        name[16] = '\0';
        offset += 16;

        printf("\t\t\t\"%s\"\n", name);
    }

    int numChildren = lsb16(buf, offset);
    offset += 2;

    printf("\t\tNumChildren: %d\n", numChildren);

    for (int i = 0; i < numChildren; i++) {
        printf("\t\tChild %d:\n", i);

        int numSubChildren = lsb8(buf, offset);
        offset += 1;

        printf("\t\t\tNumSubChildren: %d\n", numSubChildren);

        for (int j = 0; j < numSubChildren; j++) {

            printf("\t\t\tSubChild %d:\n", j);

            // upper-left
            int x0 = lsb16(buf, offset + 0);
            int y0 = lsb16(buf, offset + 2);

            // upper-right
            int x1 = lsb16(buf, offset + 4);
            int y1 = lsb16(buf, offset + 6);

            // lower-left
            int x2 = lsb16(buf, offset + 8);
            int y2 = lsb16(buf, offset + 10);

            // lower-right
            int x3 = lsb16(buf, offset + 12);
            int y3 = lsb16(buf, offset + 14);

            offset += 16;

            printf("\t\t\t\tX,Y: (%d, %d) (%d, %d) (%d, %d) (%d, %d)\n", x0, y0, x1, y1, x2, y2, x3, y3);

            int _0 = lsb16(buf, offset + 0);
            int _1 = lsb16(buf, offset + 2);
            int _2 = lsb16(buf, offset + 4);
            int _3 = lsb16(buf, offset + 6);

            offset += 8;

            printf("\t\t\t\t%d %d %d %d\n", _0, _1, _2, _3);

            _0 = lsb8(buf, offset);
            offset += 1;

            printf("\t\t\t\t%d\n", _0);

            for (int x = 0; x < 4; x++) {
                int _0 = lsb8(buf, offset + 0);
                int _1 = lsb8(buf, offset + 1);
                int _2 = lsb8(buf, offset + 2);
                int _3 = lsb8(buf, offset + 3);

                offset += 4;

                printf("\t\t\t\t%d %d %d %d\n", _0, _1, _2, _3);
            }

            _0 = lsb8(buf, offset + 0);
            _1 = lsb8(buf, offset + 1);
            _2 = lsb8(buf, offset + 2);
            _3 = lsb8(buf, offset + 3);

            offset += 4;

            printf("\t\t\t\t%d %d %d %d\n", _0, _1, _2, _3);

            char name[17];
            strncpy(name, (const char*) &buf[offset], 16);
            name[16] = '\0';
            offset += 16;

            printf("\t\t\t\tTexture Name: \"%s\"\n", name);
        }
    }

    return 0;
}

static bool menuChunkFilter(const chunk_t* chunk) {
    switch (chunk->type) {
        case CHUNK_TYPE_Meb:
            return true;
        case CHUNK_TYPE_Msb:
            return true;

        default:
            return false;
    }
}

static int menuChunkParser(const chunk_t* chunk) {

    int ret = 0;

    switch (chunk->type) {
        case CHUNK_TYPE_Meb:
            ret = parseMeb(chunk);
            break;
        case CHUNK_TYPE_Msb:
            ret = parseMsb(chunk);
            break;

        default:
            break;
    }

    return ret;
}

typedef bool (parse_chunk_filter_func)(const chunk_t* chunk);
typedef int (parse_chunk_func)(const chunk_t* chunk);

typedef struct parse_chunk_userdata_t {
    parse_chunk_filter_func* filter;
    parse_chunk_func* parser;

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

        bool wanted = true;
        if (userData->filter != NULL) {
            wanted = userData->filter(chunk);
        }

        if (wanted) {
            for (int index = 0; index <= userData->stackIndex; index++) {
                const chunk_t* c = &userData->stack[index];

                if (c->flags != 0) {
                    printf("'%s'[%01X] > ", c->FourCC, c->flags);
                } else {
                    printf("'%s' > ", c->FourCC);
                }
            }

            const char* typeName = GetChunkTypeName(chunk->type);

            if (chunk->flags != 0) {
                printf("'%s'.%s[%01X]:\n", chunk->FourCC, typeName, chunk->flags);
            } else {
                printf("'%s'.%s:\n", chunk->FourCC, typeName);
            }

            if (userData->parser != NULL) {
                int ret = userData->parser(chunk);
                if (ret < 0) {
                    return ret;
                }
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
        .filter = menuChunkFilter,
        .parser = menuChunkParser,

        .stackIndex = -1,
    });

    if (ret < 0) {
        fprintf(stderr, "%s: errors encountered parsing %s\n", argv[0], filename);
        exit(EXIT_FAILURE);
    }

    free(datBuf);

    return 0;
}
