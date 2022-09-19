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
