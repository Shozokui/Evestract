#ifndef FIRSTPASS_H
#define FIRSTPASS_H

struct event_npc_t;
struct tracer_state_t;
struct vm_t;

struct tracer_state_t* FindTracerState(struct vm_t* vm, uint32_t addr);

void firstpass(struct vm_t* vm, const struct event_npc_t* eventNpc);

#endif
