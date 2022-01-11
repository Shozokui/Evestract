#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "datid.h"
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

    for (int i = 0; i < 555; i++) {
        char filename[65536];

        int fileId = GetEntityListFileId(i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# entity_list:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetEventFileId(i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetEventMessageFileId(0, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event_message_jp:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetEventMessageFileId(1, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event_message_en:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }
    }

    for (int i = 1000; i < 1032; i++) {
        char filename[65536];

        int fileId = GetEventFileId(i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetEventMessageFileId(0, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event_message_jp:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetEventMessageFileId(1, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event_message_en:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }
    }

    for (int i = 2000; i < 2300; i++) {
        char filename[65536];

        int fileId = GetEntityListFileId(i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# entity_list:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetEventFileId(i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetEventMessageFileId(0, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event_message_jp:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetEventMessageFileId(1, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event_message_en:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }
    }

    const int items[] = { 39, 40, 41, 42, 43, 51, 52, 53 };
    for (int n = 0; n < sizeof(items)/sizeof(items[0]); n++) {
        char filename[65536];

        int i = items[n];

        int fileId = GetBaseFileId(0, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# items_jp:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetBaseFileId(1, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# items_en:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }
    }

    const int battle_messages[] = { 100, 102, 103 };
    for (int n = 0; n < sizeof(battle_messages)/sizeof(battle_messages[0]); n++) {
        char filename[65536];

        int i = battle_messages[n];

        int fileId = GetBaseFileId(0, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event_message_jp:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetBaseFileId(1, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# event_message_en:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }
    }

    const int menus[] = { 89 };
    for (int n = 0; n < sizeof(menus)/sizeof(menus[0]); n++) {
        char filename[65536];

        int i = menus[n];

        int fileId = GetBaseFileId(0, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# menu_jp:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetBaseFileId(1, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# menu_en:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }
    }

    const int dmsg[] = { 92, 94 };
    for (int n = 0; n < sizeof(dmsg)/sizeof(dmsg[0]); n++) {
        char filename[65536];

        int i = dmsg[n];

        int fileId = GetBaseFileId(0, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# dmsg_jp:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }

        fileId = GetBaseFileId(1, i);
        ret = GetDatPath(ftable, filename, sizeof(filename), NULL, fileId);
        if (ret > 0) {
            printf("# dmsg_en:%u, file_id:%u\n", i, fileId);
            printf("%s\n", filename);
        }
    }

    free(ftable);

    return 0;
}
