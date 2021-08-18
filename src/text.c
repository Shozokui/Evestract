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

        if (isprint(b)) {
            *pMsg = b;
            pMsg++;
        } else if (b == 0x82 && (i+1) < rawMessageSize && rawMessage[i+1] >= 0x80 && rawMessage[i+1] <= 0x89) {
            char tmp[32];
            sprintf(tmp, "[Itm][Param%d]", rawMessage[i+1] - 0x80);
            strcpy(pMsg, tmp);
            pMsg += strlen(tmp);
            i++;
        } else if (b == 0x92 && (i+1) < rawMessageSize && rawMessage[i+1] >= 0 && rawMessage[i+1] <= 9) {
            char tmp[32];
            sprintf(tmp, "[S_P][Param%d]", rawMessage[i+1]);
            strcpy(pMsg, tmp);
            pMsg += strlen(tmp);
            i++;
        } else if (b == 0xa && (i+1) < rawMessageSize && rawMessage[i+1] >= 0 && rawMessage[i+1] <= 9) {
            char tmp[32];
            sprintf(tmp, "[Num][Param%d]", rawMessage[i+1]);
            strcpy(pMsg, tmp);
            pMsg += strlen(tmp);
            i++;
        } else if (b == 0xc && (i+1) < rawMessageSize && rawMessage[i+1] >= 0 && rawMessage[i+1] <= 9) {
            char tmp[32];
            sprintf(tmp, "[Idx][Param%d]", rawMessage[i+1]);
            strcpy(pMsg, tmp);
            pMsg += strlen(tmp);
            i++;
        } else {
            char tmp[32];
            sprintf(tmp, "<%02x>", b);
            strcpy(pMsg, tmp);
            pMsg += strlen(tmp);
        }
    }

    return message;
}

int FreePrintableText(const char* text) {
    free((void*) text);
    return 0;
}
