#ifndef PARSER__PARSER_H
#define PARSER__PARSER_H

#include <inttypes.h>
#include <stdbool.h>

#include "resource.h"

#include "parsers/bmp2.h"
#include "parsers/meb.h"
#include "parsers/msb.h"
#include "parsers/scheduler.h"

typedef enum PARSE_CHUNK_FLAGS {
    PARSE_CHUNK_FLAG_VERBOSE = 0x00000001,
} PARSE_CHUNK_FLAGS;

typedef bool (parse_chunk_filter_func)(const chunk_t* chunk, void* userData);
typedef int (parse_chunk_func)(const chunk_t* chunk, void* userData);

typedef struct parse_chunk_userdata_t {
    // user-supplied
    parse_chunk_filter_func* filter;
    parse_chunk_func* parser;
    PARSE_CHUNK_FLAGS flags;
    void* userData;

    // private
    chunk_t stack[16];
    int stackIndex;
} parse_chunk_userdata_t;

int parseChunks(const uint8_t* datBuf, uint32_t datLen, parse_chunk_userdata_t* userData);

#endif
