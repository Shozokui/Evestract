#ifndef EVENT_H
#define EVENT_H

struct dialog_t;
struct npc_t;

struct event_t {
    uint16_t id;
    uint16_t pc;
};

struct event_npc_t {
    uint32_t NPCId;

    uint32_t* constants;
    uint32_t numConstants;

    struct event_t* events;
    uint32_t numEvents;

    uint8_t* bytecode;
    uint32_t bytecodeSize;
};

struct event_zone_t {
    struct event_npc_t* events;
    uint32_t numEvents;
};

int ParseEvent(const uint8_t* buf, uint32_t length, const struct dialog_t* dialog, const struct npc_t* npc);

#endif
