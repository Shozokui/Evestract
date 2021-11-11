#ifndef SCRIPT_H
#define SCRIPT_H

struct dialog_t;
struct event_zone_t;
struct npc_t;

int ParseScript(const struct event_zone_t* eventZone, uint32_t index, const struct dialog_t* dialog, const struct npc_t* npc);

#endif
