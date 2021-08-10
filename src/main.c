
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "event.h"

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s <file.dat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        fprintf(stderr, "%s: unable to open \"%s\"\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    long ret = fseek(fp, 0, SEEK_END);
    if (ret < 0) {
        fprintf(stderr, "%s: internal error seeking to end \"%s\"\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }
    long len = ftell(fp);
    if (len < 0) {
        fprintf(stderr, "%s: internal error getting length \"%s\"\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }
    ret = fseek(fp, 0, SEEK_SET);
    if (ret < 0) {
        fprintf(stderr, "%s: internal error seeking to start \"%s\"\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    uint8_t* buf = (uint8_t*) malloc(len);
    if (buf == NULL) {
        fprintf(stderr, "%s: internal error allocating %ld bytes \"%s\"\n", argv[0], len, argv[1]);
        exit(EXIT_FAILURE);
    }

    long rLen = fread(buf, 1, len, fp);
    if (rLen != len) {
        fprintf(stderr, "%s: internal error read \"%s\"\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);       
    }

    fclose(fp);

    ParseEvent(buf, len);

    free(buf);

    return 0;
}
