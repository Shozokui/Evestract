#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "bytes.h"
#include "resource.h"

#include "msb.h"

int parseMsb(const chunk_t* chunk) {
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
            int x0 = slsb16(buf, offset + 0);
            int y0 = slsb16(buf, offset + 2);

            // upper-right
            int x1 = slsb16(buf, offset + 4);
            int y1 = slsb16(buf, offset + 6);

            // lower-left
            int x2 = slsb16(buf, offset + 8);
            int y2 = slsb16(buf, offset + 10);

            // lower-right
            int x3 = slsb16(buf, offset + 12);
            int y3 = slsb16(buf, offset + 14);

            offset += 16;

            printf("\t\t\t\tX,Y: (%d, %d) (%d, %d) (%d, %d) (%d, %d)\n", x0, y0, x1, y1, x2, y2, x3, y3);

            int uw = slsb16(buf, offset + 0);
            int vh = slsb16(buf, offset + 2);
            int u0 = slsb16(buf, offset + 4);
            int v0 = slsb16(buf, offset + 6);

            int u1 = u0 + uw;
            int v1 = v0 + vh;

            offset += 8;

            printf("\t\t\t\tU,V: (%d, %d) (%d, %d)\n", u0, v0, u1, v1);

            int UVFlags = lsb8(buf, offset);
            offset += 1;

            // U flipped if UVFlags & 1
            // V flipped if UVFlags & 2
            printf("\t\t\t\tUVFlags: %02x\n", UVFlags);

            for (int x = 0; x < 4; x++) {
                int r = lsb8(buf, offset + 0);
                int g = lsb8(buf, offset + 1);
                int b = lsb8(buf, offset + 2);
                int a = lsb8(buf, offset + 3);

                offset += 4;

                printf("\t\t\t\tC%d: #%02x%02x%02x%02x\n", x, r, g, b, a);
            }

            // (below table may have errors, need to reverify)
            // blendval
            // 0 1 2 3    description
            // 0 - - -    blend disabled
            // 1 0 0 -    -
            // 1 0 1 0    One, SrcAlpha, Subtract
            // 1 0 1 1    SrcAlpha, InvSrcAlpha, Add
            // 1 0 1 2    -
            // 1 0 2 0    blend disabled
            // 1 0 2 1    SrcAlpha, One, Add
            // 1 0 2 2    SrcAlpha, Zero, Add
            // 1 1 0 0    InvSrcAlpha, SrcAlpha, Add
            // 1 1 0 1    SrcAlpha, One, RevSubtract
            // 1 1 0 2    -
            // 1 1 1 0    blend disabled
            // 1 1 1 -    -
            // 1 1 2 0    One, SrcAlpha, Add
            // 1 1 2 1    DestColor, SrcAlpha, Add
            // 1 1 2 2    Zero, SrcAlpha, Add
            // 1 2 0 0    -
            // 1 2 0 1    SrcAlpha, One, RevSubtract
            // 1 2 0 2    -
            // 1 2 1 0    One, SrcAlpha, Subtract
            // 1 2 1 1    Zero, InvSrcAlpha, Add
            // 1 2 1 2    -
            // 1 2 2 -    -
            int Blend0 = lsb8(buf, offset + 0);
            int Blend1 = lsb8(buf, offset + 1);
            int Blend2 = lsb8(buf, offset + 2);
            int Blend3 = lsb8(buf, offset + 3);

            offset += 4;

            printf("\t\t\t\tBlendMode: %d, %d, %d, %d\n", Blend0, Blend1, Blend2, Blend3);

            char name[17];
            strncpy(name, (const char*) &buf[offset], 16);
            name[16] = '\0';
            offset += 16;

            printf("\t\t\t\tTexture Name: \"%s\"\n", name);
        }
    }

    return 0;
}
