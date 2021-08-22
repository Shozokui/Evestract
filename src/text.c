#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "text.h"

const char* GetPrintableText(const uint8_t* rawMessage, uint32_t rawMessageSize) {

    char* message = (char*) calloc(32, rawMessageSize + 1);
    char* pMsg = message;

    // This may need to eventually convert Shift-JIS to UTF-8 or whatever.
    for (uint32_t i = 0; i < rawMessageSize; i++) {
        uint8_t b = rawMessage[i];
        uint32_t b2 = (i+1) < rawMessageSize ? rawMessage[i+1] : 0xffff;

        char tmp[32];

        if (isprint(b)) {
            *pMsg = b;
            pMsg++;
        } else { 
            if (b == 0x81 && b2 == 0x60) {
                // See above about needing Shift-JIS to UTF-8 conversion.
                // WAVE DASH - 0xE3 0x80 0x9C
                tmp[0] = 0xe3;
                tmp[1] = 0x80;
                tmp[2] = 0x9c;
                tmp[3] = 0;
            } else if (b == 0x82 && b2 >= 0x80 && b2 <= 0x89) {
                sprintf(tmp, "[Itm][Param%d]", b2 - 0x80);
            } else if (b == 0x92 && b2 >= 0 && b2 <= 9) {
                sprintf(tmp, "[S_P][Param%d]", b2);
            } else if (b == 0xa && b2 >= 0 && b2 <= 9) {
                sprintf(tmp, "[Num][Param%d]", b2);
            } else if (b == 0xc && b2 >= 0 && b2 <= 9) {
                sprintf(tmp, "[Idx][Param%d]", b2);
            } else {
                sprintf(tmp, "<%02x>", b);

                // don't consume the second byte
                i--;
            }

            strcpy(pMsg, tmp);
            pMsg += strlen(tmp);
            i++;
        }
    }

    return message;
}

int FreePrintableText(const char* text) {
    free((void*) text);
    return 0;
}
