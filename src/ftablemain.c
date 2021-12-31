
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "ftable.h"

int main(int argc, char* argv[]) {

    // basepath

    if (argc != 2) {
        fprintf(stderr, "usage: %s <basepath>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* basepath = argv[1];
    ftable_t* ftable;

    int ret = InitFTable(&ftable, basepath);
    if (ret < 0) {
        fprintf(stderr, "%s: unable to init FTABLE at %s\n", argv[0], basepath);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ftable->length; i++) {
        char filename[256];
        ret = GetDatPath(ftable, filename, 65536, NULL, i);
        printf("%d,%s\n", i, filename);
    }

    free(ftable);

    return 0;
}
