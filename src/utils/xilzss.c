#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include "../readfile.h"


int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s <stable> <input> <output>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    uint8_t* stable;
    uint32_t sLen;

    stable = ReadFile(argv[1], &sLen);
    if (stable == NULL) {
        fprintf(stderr, "%s: unable to open \"%s\" for reading\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    FILE* ifp = fopen(argv[2], "rb");
    if (ifp == NULL) {
        fprintf(stderr, "%s: unable to open \"%s\" for reading\n", argv[0], argv[2]);
        exit(EXIT_FAILURE);
    }

    FILE* ofp = fopen(argv[3], "wb");
    if (ofp == NULL) {
        fprintf(stderr, "%s: unable to open \"%s\" for writing\n", argv[0], argv[3]);
        exit(EXIT_FAILURE);
    }

    unsigned int bits = 0;
    unsigned int cmd = 0;

    #define windowsize 4096
    #define windowmask (windowsize - 1)

    uint8_t window[windowsize];
    unsigned int winpos = 0;

    size_t ipos = 0;
    size_t opos = 0;

    size_t spos = 0;
    size_t slen = 0;
    unsigned int sind = 0;

    for (unsigned int i = 0; i < windowsize; i++) {
        window[i] = 0;
    }

    slen = stable[0] | (stable[1] << 8) | (stable[2] << 16) | (stable[3] << 24);

    for (;;) {
        while (spos >= slen) {
            if (slen > 0) {
                printf("ROM/%u/%u.DAT, ipos:%zu opos:%zu spos:%zu slen:%zu\n", sind >> 7, sind & 0x7F, ipos, opos, spos, slen);
            }

            sind++;
            spos = 0;
            slen = stable[sind*4 + 0] | (stable[sind*4 + 1] << 8) | (stable[sind*4 + 2] << 16) | (stable[sind*4 + 3] << 24);

            bits = 0;

            for (;;) {
                int a = fgetc(ifp);
                if (a != 0) {
                    fseek(ifp, -1, SEEK_CUR);

                    if (a < 0x80) {
                        // todo - add proper eof detection to end decompression
                        printf("Odd...%zu, %d\n", ipos, a);
                    }

                    break;
                } else {
                    ipos++;
                }
            }
        }

        if (bits == 0) {
            int c = fgetc(ifp);
            if (c < 0) {
                break;
            }

            ipos++;

            cmd = c;
            bits = 8;
        }

        cmd <<= 1;
        bits--;

        if (cmd & 0x100) {
            int c = fgetc(ifp);
            if (c == -1) {
                fprintf(stderr, "%s: input empty when expecting literal\n", argv[0]);
                exit(EXIT_FAILURE);
            }

            window[winpos] = c;
            winpos = (winpos + 1) & windowmask;

            ipos++;
            opos++;
            spos++;

            fputc(c, ofp);
        } else {
            int w0 = fgetc(ifp);
            int w1 = fgetc(ifp);

            if (w0 == -1 || w1 == -1) {
                fprintf(stderr, "%s: input empty when expecting window\n", argv[0]);
                exit(EXIT_FAILURE);
            }

            unsigned int length = (w0 >> 4) + 3;
            unsigned int offset = ((w0 & 0x0f) << 8) | w1;

            ipos += 2;
            opos += length;
            spos += length;

            for (unsigned int i = 0; i < length; i++) {
                uint8_t c = window[(winpos - offset) & windowmask];

                window[winpos] = c;
                winpos = (winpos + 1) & windowmask;

                fputc(c, ofp);
            }
        }
    }

    fclose(ofp);
    fclose(ifp);
}
