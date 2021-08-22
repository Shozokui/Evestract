#ifndef NPC_H
#define NPC_H

struct npc_entry_t {
    uint32_t id;
    char name[29];
};

struct npc_t {
    struct npc_entry_t* entries;
    uint32_t numEntries;
};

int LoadNPC(struct npc_t** npc, const uint8_t* buf, uint32_t length);
int UnloadNPC(struct npc_t* npc);

const char* GetNPCName(const struct npc_t* npc, uint32_t npcId);

#endif
