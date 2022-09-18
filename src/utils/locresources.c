#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "../datid.h"
#include "../ftable.h"

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

    printf("index,desc,jp_id,jp_fn,en_id,en_fn,de_id,de_fn,fr_id,fr_fn\n");
    for (int i = 0; i < MAX_LOCALIZED_RESOURCES; i++) {
        char filename[65536];

        printf("%d,", i);

        for (int j = 0; j < MAX_LANGUAGES_OLD; j++) {
            int fileId = GetBaseFileId(j, i);
            if (fileId > 0) {
                GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
                printf(",%d,%s", fileId, filename);
            } else {
                printf(",,");
            }
        }

        printf("\n");
    }

    free(ftable);

    return 0;
}
