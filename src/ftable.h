#ifndef FTABLE
#define FTABLE

#include <inttypes.h>

typedef struct ftable_entry_t {
    uint16_t id;
    uint8_t version;
} ftable_entry_t;

typedef struct ftable_t {
    ftable_entry_t* entry;
    uint32_t length;
} ftable_t;

int InitFTable(struct ftable_t** pFTable, const char* basepath);
int GetDatPath(struct ftable_t* ftable, char* buf, size_t length, const char* basepath, uint32_t fileId);

#endif
