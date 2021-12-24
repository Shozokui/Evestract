#ifndef SCRIPTINT_H
#define SCRIPTINT_H

struct event_zone_t;
struct dialog_t;
struct npc_t;

struct reference_t {
    uint32_t location;
    uint32_t destination;
    uint32_t end;
    uint32_t active;
};

struct vm_t {
    const uint8_t* code;

    const uint32_t* constants;

    uint32_t pc;
    uint32_t running;
    uint32_t length;
    uint32_t numConstants;

    const struct event_zone_t* eventZone;
    const struct dialog_t* dialog;
    const struct npc_t* npc;

    // Disassembly state tracking.
    uint32_t unsup;

    struct reference_t* addrEnd; // End, Ret
    uint32_t addrEndLen;

    struct reference_t* addrJmp; // Jmp, Call, Cmp, etc.
    uint32_t addrJmpLen;

    struct reference_t* addrData; // Etc...
    uint32_t addrDataLen;
};

#endif
