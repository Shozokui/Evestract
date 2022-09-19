#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "bytes.h"
#include "resource.h"

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

const char* GetChunkTypeName(CHUNK_TYPE type) {
    if (type < CHUNK_TYPE_Max) {
        return ChunkTypeNames[type];
    }
    return "<Invalid>";
}

int LoadChunkedResource(const uint8_t* buf, uint32_t bufLen, int (*callback)(const chunk_t*, void*), void* userData) {

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
