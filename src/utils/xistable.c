#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <sys/types.h>
#if __linux__
#include <sys/sysmacros.h>
#endif
#include <unistd.h>
#include <errno.h>

#include "../readfile.h"

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        fprintf(stderr, "usage: %s <FTABLE.DAT> <VTABLE.DAT|none> <STABLE.DAT> <ROM.DAT> <prefix>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *fnFTable = argv[1];
    const char *fnVTable = argv[2];
    const char *fnSTable = argv[3];
    const char *fnRom = argv[4];
    const char *fnPrefix = argv[5];

    uint8_t *ftable = NULL;
    uint32_t fLen = 0;

    uint8_t *vtable = NULL;
    uint32_t vLen = 0;

    uint8_t *stable = NULL;
    uint32_t sLen = 0;

    ftable = ReadFile(fnFTable, &fLen);
    if (ftable == NULL)
    {
        fprintf(stderr, "%s: unable to open \"%s\" for reading\n", argv[0], fnFTable);
        exit(EXIT_FAILURE);
    }

    if (strcmp(fnVTable, "none") != 0)
    {
        vtable = ReadFile(fnVTable, &vLen);
        if (vtable == NULL)
        {
            fprintf(stderr, "%s: unable to open \"%s\" for reading\n", argv[0], fnVTable);
            exit(EXIT_FAILURE);
        }
    }

    stable = ReadFile(fnSTable, &sLen);
    if (stable == NULL)
    {
        fprintf(stderr, "%s: unable to open \"%s\" for reading\n", argv[0], fnSTable);
        exit(EXIT_FAILURE);
    }

    uint32_t entries = fLen / 2;
    if (vtable != NULL && vLen != entries)
    {
        fprintf(stderr, "%s: mismatch between \"%s\" and \"%s\"\n", argv[0], fnFTable, fnVTable);
    }

    uint32_t version = 1;
    if (vtable != NULL)
    {
        for (uint32_t i = 0; i < entries; i++)
        {
            if (vtable[i] > version)
            {
                version = vtable[i];
            }
        }
    }

    uint32_t *pEntries = (uint32_t *)calloc(sizeof(uint32_t), entries);
    if (pEntries == NULL)
    {
        fprintf(stderr, "%s: unable to alloc entry table\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    entries = sLen / 4;
    size_t sOffset = 0;

    FILE *romFp = fopen(fnRom, "rb");
    if (romFp == NULL)
    {
        fprintf(stderr, "%s: unable to open \"%s\" for reading\n", argv[0], fnRom);
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < entries; i++)
    {
        uint32_t sIndex = i;
        uint32_t fileSize = (stable[sIndex * 4 + 0] << 0) | (stable[sIndex * 4 + 1] << 8) | (stable[sIndex * 4 + 2] << 16) | (stable[sIndex * 4 + 3] << 24);
        if (fileSize == 0)
        {
            continue;
        }

        uint32_t romId = i;

        char romtmpbuf[8];
        const char *rombuf = "";

        if (version > 1)
        {
            snprintf(romtmpbuf, 8, "%u", version);
            rombuf = romtmpbuf;
        }

        char filename[65536];
        snprintf(filename, 65536, "%s/ROM%s", fnPrefix, rombuf);

        int ret = mkdir(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (ret == -1)
        {
            if (errno == EEXIST)
            {
                struct stat statbuf;

                ret = stat(filename, &statbuf);

                if (ret == -1)
                {
                    fprintf(stderr, "%s: unable to create directory \"%s\" (2)", argv[0], filename);
                    exit(EXIT_FAILURE);
                }

                if ((statbuf.st_mode & S_IFMT) != S_IFDIR)
                {
                    fprintf(stderr, "%s: unable to create directory \"%s\" (3)", argv[0], filename);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                fprintf(stderr, "%s: unable to create directory \"%s\"", argv[0], filename);
                exit(EXIT_FAILURE);
            }
        }

        snprintf(filename, 65536, "%s/ROM%s/%u", fnPrefix, rombuf, romId >> 7);

        ret = mkdir(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (ret == -1)
        {
            if (errno == EEXIST)
            {
                struct stat statbuf;

                ret = stat(filename, &statbuf);

                if (ret == -1)
                {
                    fprintf(stderr, "%s: unable to create directory \"%s\" (2)", argv[0], filename);
                    exit(EXIT_FAILURE);
                }

                if ((statbuf.st_mode & S_IFMT) != S_IFDIR)
                {
                    fprintf(stderr, "%s: unable to create directory \"%s\" (3)", argv[0], filename);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                fprintf(stderr, "%s: unable to create directory \"%s\"", argv[0], filename);
                exit(EXIT_FAILURE);
            }
        }

        snprintf(filename, 65536, "%s/ROM%s/%u/%u.DAT", fnPrefix, rombuf, romId >> 7, romId & 0x7F);

        printf("%6u/%6u %s: %zu offset, %u bytes\n", sIndex, entries, filename, sOffset, fileSize);

        FILE *fp = fopen(filename, "wb");
        if (fp == NULL)
        {
            fprintf(stderr, "%s: unable to open \"%s\" for writing\n", argv[0], filename);
            exit(EXIT_FAILURE);
        }

        uint8_t *buf = (uint8_t *)malloc(fileSize);
        if (buf == NULL)
        {
            fprintf(stderr, "%s: unable to alloc %u bytes\n", argv[0], fileSize);
            exit(EXIT_FAILURE);
        }

        uint32_t readBytes = fread(buf, 1, fileSize, romFp);
        if (readBytes != fileSize)
        {
            fprintf(stderr, "%s: read %u bytes out of %u from \"%s\"", argv[0], readBytes, fileSize, fnRom);
            exit(EXIT_FAILURE);
        }

        readBytes = fwrite(buf, 1, fileSize, fp);
        if (readBytes != fileSize)
        {
            fprintf(stderr, "%s: wrote %u bytes out of %u from \"%s\"", argv[0], readBytes, fileSize, filename);
            exit(EXIT_FAILURE);
        }

        fclose(fp);

        sOffset += fileSize;
    }

    fclose(romFp);

    free(ftable);
    free(vtable);
    free(stable);

    printf("%zu bytes total.\n", sOffset);

    return 0;
}
