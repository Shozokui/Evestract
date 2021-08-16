
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "dialog.h"
#include "event.h"
#include "npc.h"

static uint8_t* ReadFile(const char* fileName, uint32_t* bufLen) {

    FILE* fp = fopen(fileName, "rb");
    if (fp == NULL) {
        fprintf(stderr, "unable to open \"%s\"\n",fileName);
        return NULL;
    }

    long ret = fseek(fp, 0, SEEK_END);
    if (ret < 0) {
        fprintf(stderr, "internal error seeking to end \"%s\"\n", fileName);
        fclose(fp);
        return NULL;
    }
    long len = ftell(fp);
    if (len < 0) {
        fprintf(stderr, "internal error getting length \"%s\"\n", fileName);
        fclose(fp);
        return NULL;
    }
    ret = fseek(fp, 0, SEEK_SET);
    if (ret < 0) {
        fprintf(stderr, "internal error seeking to start \"%s\"\n", fileName);
        fclose(fp);
        return NULL;
    }

    uint8_t* buf = (uint8_t*) malloc(len);
    if (buf == NULL) {
        fprintf(stderr, "internal error allocating %ld bytes \"%s\"\n", len, fileName);
        fclose(fp);
        return NULL;
    }

    long rLen = fread(buf, 1, len, fp);
    if (rLen != len) {
        fprintf(stderr, "internal error read \"%s\"\n", fileName);
        free(buf);
        fclose(fp);
        return NULL;       
    }

    fclose(fp);

    *bufLen = (uint32_t) len;
    return buf;
}

int main(int argc, char* argv[]) {

    if (argc != 4) {
        // Wouldn't it be nice if this had knowledge of the DAT file layout.
        fprintf(stderr, "usage: %s <event.dat> <dialog.dat> <npc.dat>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    uint32_t eventLen;
    uint32_t dialogLen;
    uint32_t npcLen;

    uint8_t* eventBuf = ReadFile(argv[1], &eventLen);
    if (eventBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    uint8_t* dialogBuf = ReadFile(argv[2], &dialogLen);
    if (dialogBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], argv[2]);
        exit(EXIT_FAILURE);
    }

    uint8_t* npcBuf = ReadFile(argv[3], &npcLen);
    if (npcBuf == NULL) {
        fprintf(stderr, "%s: unable to read %s\n", argv[0], argv[3]);
        exit(EXIT_FAILURE);
    }

    struct dialog_t* dialog;
    struct npc_t* npc;

    LoadDialog(&dialog, dialogBuf, dialogLen);
    LoadNPC(&npc, npcBuf, npcLen);

    ParseEvent(eventBuf, eventLen, dialog, npc);

    UnloadNPC(npc);

    free(npcBuf);
    free(dialogBuf);
    free(eventBuf);

    return 0;
}
