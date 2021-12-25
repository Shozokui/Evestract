#ifndef SCRIPTINT_H
#define SCRIPTINT_H

struct event_zone_t;
struct dialog_t;
struct npc_t;

#define TRACER_JUMP                     0x01
#define TRACER_DATA                     0x02
#define TRACER_END                      0x04
#define TRACER_JUMP_TABLE               0x08
#define TRACER_INT_JUMP_VISITED         0x10
#define TRACER_INT_JUMP_TABLE_VISITED   0x80

#define IS_JUMP(s)                 (((s)->flags & TRACER_JUMP) != 0)
#define IS_DATA(s)                 (((s)->flags & TRACER_DATA) != 0)
#define IS_END(s)                  (((s)->flags & TRACER_END) != 0)
#define IS_JUMP_TABLE(s)           (((s)->flags & TRACER_JUMP_TABLE) != 0)

#define IS_UNVISITED_JUMP(s)       (((s)->flags & (TRACER_JUMP|TRACER_INT_JUMP_VISITED)) == TRACER_JUMP)
#define IS_UNVISITED_JUMP_TABLE(s) (((s)->flags & (TRACER_JUMP_TABLE|TRACER_INT_JUMP_TABLE_VISITED)) == TRACER_JUMP_TABLE)

struct tracer_state_t {
    uint32_t address;
    uint32_t flags;
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

    // First-pass tracer
    struct tracer_state_t* tracerState; // End, Ret
    uint32_t tracerCount;
};

#endif
