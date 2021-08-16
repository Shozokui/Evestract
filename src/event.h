#ifndef EVENT_H
#define EVENT_H

struct dialog_t;
struct npc_t;

int ParseEvent(const uint8_t* buf, uint32_t length, const struct dialog_t* dialog, const struct npc_t* npc);

#endif
