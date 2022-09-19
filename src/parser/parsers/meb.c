#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "bytes.h"
#include "resource.h"

#include "meb.h"

int parseMeb(const chunk_t* chunk) {
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
