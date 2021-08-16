#ifndef SCRIPT_H
#define SCRIPT_H

int ParseScript(const uint8_t* script, uint32_t length, const uint32_t* constants, uint32_t numConstants, const struct dialog_t* dialog, const struct npc_t* npc);

#endif
