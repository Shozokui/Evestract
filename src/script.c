#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "bytes.h"
#include "dialog.h"
#include "event.h"
#include "firstpass.h"
#include "npc.h"
#include "script.h"
#include "scriptint.h"
#include "text.h"

typedef void (*OpcodeFunc)(struct vm_t*);

static char tmpBuf[32][256];

static uint8_t getImm8(const struct vm_t* vm, uint32_t off) {
   return lsb8(vm->code, vm->pc, off);
}

static uint16_t getImm16(const struct vm_t* vm, uint32_t off) {
   return lsb16(vm->code, vm->pc, off);
}

static uint32_t getImm32(const struct vm_t* vm, uint32_t off) {
   return lsb32(vm->code, vm->pc, off);
}

#define VAR_FLAGS_INT           0x0001
#define VAR_FLAGS_HEX           0x0002
#define VAR_FLAGS_FLOAT_1000    0x0004
#define VAR_FLAGS_RADIANS       0x0008
#define VAR_FLAGS_FLOAT         0x0010
#define VAR_FLAGS_FLOAT_10      0x0020
#define VAR_FLAGS_FLOATX10      0x0040

static const char* getVar16NameWithFlags(const struct vm_t* vm, uint32_t off, uint32_t flags) {
    uint16_t addr = lsb16(vm->code, vm->pc, off);

    // type hints
    uint32_t IsSignedInt = (flags & VAR_FLAGS_INT) != 0;
    uint32_t IsHex = (flags & VAR_FLAGS_HEX) != 0;
    uint32_t IsFloat100 = (flags & VAR_FLAGS_FLOAT_1000) != 0;
    uint32_t IsRadians = (flags & VAR_FLAGS_RADIANS) != 0;
    uint32_t IsFloat = (flags & VAR_FLAGS_FLOAT) != 0;
    uint32_t IsFloat10 = (flags & VAR_FLAGS_FLOAT_10) != 0;
    uint32_t IsFloatX10 = (flags & VAR_FLAGS_FLOATX10) != 0;

    char* buf = &tmpBuf[off][0];
    if (addr == 0x1000) {
        return "SelectedOption";
    } else if (addr == 0x1001) {
        return "EventResult";
    } else if (addr >= 0x1002 && addr <= 0x1009) {
        sprintf(buf, "Param%d", addr - 0x1002);
    } else if (addr == 0x7f00) {
        // LocalWorkZone - int(Position * 1000.0)
        sprintf(buf, "Position.x");
    } else if (addr == 0x7f01) {
        // LocalWorkZone - int(Position * 1000.0)
        sprintf(buf, "Position.y");
    } else if (addr == 0x7f02) {
        // LocalWorkZone - int(Position * 1000.0)
        sprintf(buf, "Position.z");
    } else if (addr == 0x7f03) {
        // LocalWorkZone  - int(Rotation.y * 4096.0)
        sprintf(buf, "Rotation.y");
    } else if (addr == 0x7f06) {
        // Read only - returns 0 if the script isn't set to run on the player
        sprintf(buf, "PlayerMainJobOr0");
    } else if (addr == 0x7f07) {
        // Read only
        sprintf(buf, "EntityRace");
    } else if (addr == 0x7f08) {
        // Read only - returns 1 if the script isn't set to run on the player
        sprintf(buf, "PlayerMainJobLevelOr1");
    } else if (addr == 0x7f0a) {
        // Read only
        sprintf(buf, "EntityId");
    } else if (addr == 0x7f0b) {
        // Read only - bit 0x19 of Flags1
        sprintf(buf, "EntityGender");
    } else if (addr == 0x7f80) {
        // Read only - int(Position * 1000.0)
        sprintf(buf, "PlayerIntPosition.x");
    } else if (addr == 0x7f81) {
        // Read only - int(Position * 1000.0)
        sprintf(buf, "PlayerIntPosition.y");
    } else if (addr == 0x7f82) {
        // Read only - int(Position * 1000.0)
        sprintf(buf, "PlayerIntPosition.z");
    } else if (addr == 0x7f83) {
        // Read only - int(Position.w * 4096.0)
        sprintf(buf, "PlayerIntPosition.w");
    } else if (addr == 0x7f86) {
        // Read only
        sprintf(buf, "PlayerMainJob");
    } else if (addr == 0x7f87) {
        // Read only
        sprintf(buf, "PlayerRace");
    } else if (addr == 0x7f88) {
        // Read only
        sprintf(buf, "PlayerMainJobLevel");
    } else if (addr == 0x7f8a) {
        // Read only
        sprintf(buf, "PlayerEntityId");
    } else if (addr == 0x7f8b) {
        // Read only - bit 0x19 of Flags1
        sprintf(buf, "PlayerGender");
    } else if (addr >= 0x8000 && addr < (0x8000 + vm->numConstants)) {
        uint32_t constant = vm->constants[addr - 0x8000];
        if (IsFloat) {
            sprintf(buf, "#%.5f", (float) (int32_t) constant);
        } else if (IsFloatX10) {
            sprintf(buf, "#%.5f", (float) (int32_t) constant * 10.0);
        } else if (IsFloat10) {
            sprintf(buf, "#%.5f", (float) (int32_t) constant * 0.1);
        } else if (IsFloat100) {
            sprintf(buf, "#%.5f", (float) (int32_t) constant * 0.001);
        } else if (IsRadians) {
            sprintf(buf, "#%.5f", (float) (int32_t) constant * 6.283 * 0.0002441406);
        } else if (constant == 0x40000000) {
            return "OptionCancel";
        } else if (IsSignedInt || constant >= 0xffffff00) {
            sprintf(buf, "#%d", (int32_t) constant);
        } else if (IsHex || constant >= 0x7fffffc0) {
            sprintf(buf, "#$%08x", constant);
        } else {
            sprintf(buf, "#%u", constant);
        }
    } else {
        sprintf(buf, "[%04x]", addr);
    }

    return buf;
}

static const char* getVar16Name(const struct vm_t* vm, uint32_t off) {
    return getVar16NameWithFlags(vm, off, 0);
}

static const char* getVar16NameInt(const struct vm_t* vm, uint32_t off) {
    return getVar16NameWithFlags(vm, off, VAR_FLAGS_INT);
}

static const char* getVar16NameFloat(const struct vm_t* vm, uint32_t off) {
    return getVar16NameWithFlags(vm, off, VAR_FLAGS_FLOAT);
}

static const char* getVar16NameFloatX10(const struct vm_t* vm, uint32_t off) {
    return getVar16NameWithFlags(vm, off, VAR_FLAGS_FLOATX10);
}

static const char* getVar16NameFloat10(const struct vm_t* vm, uint32_t off) {
    return getVar16NameWithFlags(vm, off, VAR_FLAGS_FLOAT_10);
}

static const char* getVar16NameFloat100(const struct vm_t* vm, uint32_t off) {
    return getVar16NameWithFlags(vm, off, VAR_FLAGS_FLOAT_1000);
}

static const char* getVar16NameRadians(const struct vm_t* vm, uint32_t off) {
    return getVar16NameWithFlags(vm, off, VAR_FLAGS_RADIANS);
}

static const char* getVar32Name(struct vm_t* vm, uint32_t off) {
    uint32_t addr = getImm32(vm, off);

    char* buf = &tmpBuf[off][0];

    // When used as an NPC, values below 0xff000000
    // return the script target.

    const char* npcName = GetNPCName(vm->npc, addr);
    if (npcName != NULL) {
        sprintf(buf, "#%u[\"%s\"]", addr, npcName);
    } else if (addr >= 0x7fffff00 && addr < 0x80000000) {
        switch (addr) {
            case 0x7fffffc0:
            case 0x7ffffff0:
            case 0x7ffffff9:
                sprintf(buf, "[Player]");
                break;
            case 0x7ffffff8:
                sprintf(buf, "[NPC]");
                break;

            case 0x7ffffff1:
                sprintf(buf, "[PartyMember2]");
                break;
            case 0x7ffffff2:
                sprintf(buf, "[PartyMember3]");
                break;
            case 0x7ffffff3:
                sprintf(buf, "[PartyMember4]");
                break;
            case 0x7ffffff4:
                sprintf(buf, "[PartyMember5]");
                break;
            case 0x7ffffff5:
                sprintf(buf, "[PartyMember6]");
                break;

            case 0x7fffffc1:
                sprintf(buf, "[AllianceMember2]");
                break;
            case 0x7fffffc2:
                sprintf(buf, "[AllianceMember3]");
                break;
            case 0x7fffffc3:
                sprintf(buf, "[AllianceMember4]");
                break;
            case 0x7fffffc4:
                sprintf(buf, "[AllianceMember5]");
                break;
            case 0x7fffffc5:
                sprintf(buf, "[AllianceMember6]");
                break;
            case 0x7fffffc6:
                sprintf(buf, "[AllianceMember7]");
                break;
            case 0x7fffffc7:
                sprintf(buf, "[AllianceMember8]");
                break;
            case 0x7fffffc8:
                sprintf(buf, "[AllianceMember9]");
                break;
            case 0x7fffffc9:
                sprintf(buf, "[AllianceMember10]");
                break;
            case 0x7fffffca:
                sprintf(buf, "[AllianceMember11]");
                break;
            case 0x7fffffcb:
                sprintf(buf, "[AllianceMember12]");
                break;
            case 0x7fffffcc:
                sprintf(buf, "[AllianceMember13]");
                break;
            case 0x7fffffcd:
                sprintf(buf, "[AllianceMember14]");
                break;
            case 0x7fffffce:
                sprintf(buf, "[AllianceMember15]");
                break;
            case 0x7fffffcf:
                sprintf(buf, "[AllianceMember16]");
                break;
            case 0x7fffffd0:
                sprintf(buf, "[AllianceMember17]");
                break;
            case 0x7fffffd1:
                sprintf(buf, "[AllianceMember18]");
                break;

            default:
                sprintf(buf, "[%08x]", addr);
        }
    } else {
        sprintf(buf, "#$%08x", addr);
    }

    return buf;
}

static const char* getVar32FourCC(struct vm_t* vm, uint32_t off) {
    uint32_t a = getImm8(vm, off + 0);
    uint32_t b = getImm8(vm, off + 1);
    uint32_t c = getImm8(vm, off + 2);
    uint32_t d = getImm8(vm, off + 3);

    char* buf = &tmpBuf[off][0];

    if (!(isalnum(a) || ispunct(a) || a == ' ') || !(isalnum(b) || ispunct(b) || b == ' ') || !(isalnum(c) || ispunct(c) || c == ' ') || !(isalnum(d) || ispunct(d) || d == ' ')) {
        sprintf(buf, "#$%02x%02x%02x%02x", a, b, c, d);
    } else {
        sprintf(buf, "#'%c%c%c%c'", a, b, c, d);
    }

    return buf;
}

static const char* getEventLabelByAddrAndIndex(struct vm_t* vm, uint32_t addr, uint32_t off, uint32_t index) {
    char* buf = &tmpBuf[off][0];

    // Fallback value.
    sprintf(buf, "#%d",  index);

    for (uint32_t i = 0; i < vm->eventZone->numEvents; i++) {
        const struct event_npc_t* eventNpc = &vm->eventZone->events[i];

        if (eventNpc->NPCId == addr) {
            if (index < eventNpc->numEvents) {
                const struct event_t* event = &eventNpc->events[index];

                sprintf(buf, "EventEntryPoint_%u_%u.L%04X", eventNpc->NPCId, event->id, event->pc);
            }
        }
    }

    return buf;
}

static const char* getVar16Message(const struct vm_t* vm, uint32_t off) {
    if (vm->dialog != NULL) {
        uint32_t addr = lsb16(vm->code, vm->pc, off);

        if (addr >= 0x8000 && (addr - 0x8000) < vm->numConstants) {
            uint32_t constant = vm->constants[addr - 0x8000];

            if (constant < vm->dialog->numEntries) {
                return GetPrintableDialogText(vm->dialog, constant);
            }
        }
    }

    return NULL;
}

static void OpcodeUNSUP(struct vm_t* vm) {
    vm->running = 0;
}

static void Opcode00(struct vm_t* vm) {
    printf("END\n");

    vm->pc += 1;
}

static void Opcode01(struct vm_t* vm) {
    printf("JMP .L%04X\n",
        getImm16(vm, 1));

    vm->pc += 3;
}

static void Opcode02(struct vm_t* vm) {
    // CMP A, B, Op, Dest

    uint8_t param = getImm8(vm, 5);

    const char* Op = "<#>";

    // Need to redo in a more-intuitive syntax.
    switch (param & 0x0f) {
        case 0:
            Op = "!=";
            break;
        case 1:
        case 7:
            Op = "==";
            break;
        case 2:
            Op = "<=";
            break;
        case 3:
            Op = ">=";
            break;
        case 4:
            Op = "<";
            break;
        case 5:
            Op = ">";
            break;

        // NOT CURRENTLY USED
        case 6:
        case 9:
            // (A & B) == 0
            // no common bits
            // Was once used in an old Doll Festival event that no longer exists.
            Op = "!&";
            break;
        case 8:
            // (A | B) == 0
            // both parameters have to be zero
            // No known uses.
            Op = "!|";
            break;
        case 10:
            // (A & ~B) == 0
            // No known uses.
            Op = "~!&";
            break;
        default:
            vm->running = 0;
            return;
    }

    // *** Official Name: IF
    printf("CMP %s %s %s, .L%04X\n",
        getVar16Name(vm, 1),
        Op,
        getVar16Name(vm, 3),
        getImm16(vm, 6));

    vm->pc += 8;
}

static void Opcode03(struct vm_t* vm) {
    printf("MOV %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode04(struct vm_t* vm) {
    printf("NOP %s\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
}

static void Opcode05(struct vm_t* vm) {
    printf("MOV %s, #1\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
}

static void Opcode06(struct vm_t* vm) {
    printf("MOV %s, #0\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
}

static void Opcode07(struct vm_t* vm) {
    printf("ADD %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode08(struct vm_t* vm) {
    // SUB X, Y -> X = X - Y
    printf("SUB %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode09(struct vm_t* vm) {
    printf("BITSET %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode0A(struct vm_t* vm) {
    printf("BITCLR %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode0B(struct vm_t* vm) {
    printf("INC %s\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
}

static void Opcode0C(struct vm_t* vm) {
    printf("DEC %s\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
}

static void Opcode0D(struct vm_t* vm) {
    printf("AND %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode0E(struct vm_t* vm) {
    printf("OR %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode0F(struct vm_t* vm) {
    printf("XOR %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode10(struct vm_t* vm) {
    printf("ASL %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode11(struct vm_t* vm) {
    printf("ASR %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode12(struct vm_t* vm) {
    printf("RAND %s\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
}

static void Opcode13(struct vm_t* vm) {
    printf("RAND %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode14(struct vm_t* vm) {
    // MUL L <- L * R
    printf("MUL %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode15(struct vm_t* vm) {
    // DIV Num <- Num / Div
    printf("DIV %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode16(struct vm_t* vm) {
    printf("SIN %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16NameRadians(vm, 3),
        getVar16Name(vm, 5));
    vm->pc += 7;
}

static void Opcode17(struct vm_t* vm) {
    printf("COS %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16NameRadians(vm, 3),
        getVar16Name(vm, 5));
    vm->pc += 7;
}

static void Opcode18(struct vm_t* vm) {
    printf("ATAN %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5));
    vm->pc += 7;
}

static void Opcode19(struct vm_t* vm) {
    printf("SWAP %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode1A(struct vm_t* vm) {
    printf("CALL .L%04X\n",
        getImm16(vm, 1));

    vm->pc += 3;
}

static void Opcode1B(struct vm_t* vm) {
    printf("RET\n");

    vm->pc += 1;
}

static void Opcode1C(struct vm_t* vm) {
    printf("WAIT %s\n",
        getVar16NameFloat(vm, 1));
    vm->pc += 3;
}

static void Opcode1D(struct vm_t* vm) {
    const char* message = getVar16Message(vm, 1);

    if (message != NULL) {
        printf("NPCSAY %s // %s\n",
            getVar16Name(vm, 1),
            message);
        FreePrintableText(message);
    } else {
        printf("NPCSAY %s\n",
            getVar16Name(vm, 1));
    }

    vm->pc += 3;
}

static void Opcode1E(struct vm_t* vm) {
    printf("LOOKAT %s\n",
        getVar32Name(vm, 1));
    vm->pc += 5;
}

static void Opcode1F(struct vm_t* vm) {
    uint32_t param = lsb8(vm->code, vm->pc, 1);

    // *** Official Name: MOVE

    if (param == 0) {
        printf("MOVETO %s, %s, %s\n",
            getVar16NameFloat100(vm, 2),
            getVar16NameFloat100(vm, 4),
            getVar16NameFloat100(vm, 6));

        vm->pc += 8;
    } else if (param == 1) {
        printf("MOVETO WAIT\n");

        vm->pc += 2;
    } else {
        vm->running = 0;
    }
}

static void Opcode20(struct vm_t* vm) {
    printf("BLOCKMENU %02x\n",
        lsb8(vm->code, vm->pc, 1));

    vm->pc += 2;
}

static void Opcode21(struct vm_t* vm) {
    printf("FINISHEVENT\n");
    vm->pc += 1;
}

static void Opcode22(struct vm_t* vm) {
    printf("Opcode22 %02x\n",
        lsb8(vm->code, vm->pc, 1));

    vm->pc += 2;
}

static void Opcode23(struct vm_t* vm) {
    // Text was just shown -- wait for a continue / cancel button press.
    // Cancelling will end the event.

    // *** Official Name: MESWAIT

    printf("WAITFORINPUT\n");
    vm->pc += 1;
}

static void Opcode24(struct vm_t* vm) {
    // Show a dialog with options.
    const char* message = getVar16Message(vm, 1);

    // *** Official Name: QUERY

    if (message != NULL) {
        printf("PROMPT %s, %s, %s // %s\n",
            getVar16Name(vm, 1),
            getVar16Name(vm, 3),
            getVar16Name(vm, 5),
            message);
        FreePrintableText(message);
    } else {
        printf("PROMPT %s, %s, %s\n",
            getVar16Name(vm, 1),
            getVar16Name(vm, 3),
            getVar16Name(vm, 5));
    }

    vm->pc += 7;
}

static void Opcode25(struct vm_t* vm) {
    // A dialog was just shown -- wait for an option select / cancel button press.
    // Cancelling will end the event.

    // *** Official Name: QUERYWAIT

    printf("WAITFORPROMPT\n");
    vm->pc += 1;
}

static void Opcode26(struct vm_t* vm) {
    // Getting here will lock up the event.
    printf("HALT\n");
    vm->pc += 1;
}

static void Opcode27(struct vm_t* vm) {
    uint32_t addr = getImm32(vm, 2);

    // Starts an event for an npc, only halting if there are
    // no free instances. The caller does not wait for the
    // event to complete.
    printf("STARTEVENT1 #%d, %s, %s\n",
        getImm8(vm, 1),
        getVar32Name(vm, 2),
        getEventLabelByAddrAndIndex(vm, addr, 6, lsb8(vm->code, vm->pc, 6)));

    vm->pc += 7;
}

static void Opcode28(struct vm_t* vm) {
    uint32_t addr = getImm32(vm, 2);

    // *** Official Name: REQSW

    // Starts an event for an npc, waiting for the event to complete.
    printf("STARTEVENT2 #%d, %s, %s\n",
        getImm8(vm, 1),
        getVar32Name(vm, 2),
        getEventLabelByAddrAndIndex(vm, addr, 6, lsb8(vm->code, vm->pc, 6)));

    vm->pc += 7;
}

static void Opcode29(struct vm_t* vm) {
    uint32_t addr = getImm32(vm, 2);

    // *** Official Name: REQEW

    // Starts an event for an npc, waiting for the event to complete.
    // The caller has an extra validity check over V2 to verify
    // that the npc is still valid before waiting.
    printf("STARTEVENT3 #%d, %s, %s\n",
        getImm8(vm, 1),
        getVar32Name(vm, 2),
        getEventLabelByAddrAndIndex(vm, addr, 6, lsb8(vm->code, vm->pc, 6)));

    vm->pc += 7;
}

static void Opcode2A(struct vm_t* vm) {
    // Waits while there are any events running on the target
    // at an equal or higher priority.
    printf("WAITFOREVENTS #%d, %s\n",
        // Baseline priority
        getImm8(vm, 1),
        // Entity to test
        getVar32Name(vm, 2));

    vm->pc += 6;
}

static void Opcode2B(struct vm_t* vm) {
    const char* message = getVar16Message(vm, 5);

    if (message != NULL) {
        printf("NPCSAY2 %s, %s // %s\n",
            getVar32Name(vm, 1),
            getVar16Name(vm, 5),
            message);
        FreePrintableText(message);
    } else {
        printf("NPCSAY2 %s, %s\n",
            getVar32Name(vm, 1),
            getVar16Name(vm, 5));
    }

    vm->pc += 7;
}

static void Opcode2C(struct vm_t* vm) {

    // *** Official Name: SCHEDULOR

    printf("SCHEDULER %s, %s, %s\n",
        getVar32Name(vm, 1),
        getVar32Name(vm, 5),
        getVar32FourCC(vm, 9));

    vm->pc += 13;
}

static void Opcode2D(struct vm_t* vm) {

    // *** Official Name: MAPSCHEDULOR

    printf("MAPSCHEDULER %s, %s, %s\n",
        getVar32Name(vm, 1),
        getVar32Name(vm, 5),
        getVar32FourCC(vm, 9));

    vm->pc += 13;
}

static void Opcode2E(struct vm_t* vm) {
    printf("Opcode2E\n");

    vm->pc += 1;
}

static void Opcode2F(struct vm_t* vm) {
    printf("Opcode2F %02x, %s\n",
        lsb8(vm->code, vm->pc, 1),
        getVar32Name(vm, 2));

    vm->pc += 6;
}

static void Opcode30(struct vm_t* vm) {
    printf("REMAINBLOCKED\n");

    vm->pc += 1;
}

static void Opcode31(struct vm_t* vm) {
    uint32_t param = lsb8(vm->code, vm->pc, 1);

    // *** Official Name: SMOVE

    // MOVETO3 interpolates x, y, z, ending after # frames
    if (param == 0) {
        printf("MOVETO3 %s, %s, %s, %s\n",
            getVar16NameFloat100(vm, 2),
            getVar16NameFloat100(vm, 4),
            getVar16NameFloat100(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 1) {
        printf("MOVETO3 WAIT\n");

        vm->pc += 2;
    } else {
        vm->running = 0;
    }
}

static void Opcode32(struct vm_t* vm) {
    printf("SETMOVEMENTSPEED %s\n",
        getVar16NameFloat10(vm, 1));
    vm->pc += 3;
}

static void Opcode33(struct vm_t* vm) {
    printf("Opcode33 %02x\n",
        lsb8(vm->code, vm->pc, 1));
    vm->pc += 2;
}

static void Opcode34(struct vm_t* vm) {
    // Loads a zone.
    printf("LOADZONE %s\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
}

static void Opcode35(struct vm_t* vm) {
    // Loads a zone.
    printf("LOADZONE2 %s\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
}

static void Opcode36(struct vm_t* vm) {
    // Set entity event position
    printf("SETPOSITION %s, %s, %s\n",
        getVar16NameFloat100(vm, 1),
        getVar16NameFloat100(vm, 3),
        getVar16NameFloat100(vm, 5));

    vm->pc += 7;
}

static void Opcode37(struct vm_t* vm) {
    // Set entity event position and rotation
    printf("SETPOSITION2 %s, %s, %s, %s\n",
        getVar16NameFloat100(vm, 1),
        getVar16NameFloat100(vm, 3),
        getVar16NameFloat100(vm, 5),
        getVar16NameRadians(vm, 7));
    vm->pc += 9;
}

static void Opcode38(struct vm_t* vm) {
    printf("SETEVENTFLAGS %s\n",
        getVar16NameWithFlags(vm, 1, VAR_FLAGS_HEX));

    vm->pc += 3;
}

static void Opcode39(struct vm_t* vm) {
    printf("SETROTATION %s\n",
        getVar16NameRadians(vm, 1));

    vm->pc += 3;
}

static void Opcode3A(struct vm_t* vm) {
    printf("GETROTATION %s, %s\n",
        getVar32Name(vm, 1),
        getVar16Name(vm, 5));
    vm->pc += 7;
}

static void Opcode3B(struct vm_t* vm) {
    printf("GETPOSITION %s, %s, %s, %s\n",
        getVar32Name(vm, 1),
        getVar16Name(vm, 5),
        getVar16Name(vm, 7),
        getVar16Name(vm, 9));

    vm->pc += 11;
}

static void Opcode3C(struct vm_t* vm) {
    // [P1+(P2>>5)] |= (1<<(P2&31))
    // P3 is size of bit array in bytes
    // Operation is ignored if (P2>>5)>=P3
    printf("BITARRAYSET %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5));

    vm->pc += 7;
}

static void Opcode3D(struct vm_t* vm) {
    // [P1+(P2>>5)] &= ~(1<<(P2&31))
    // P3 is size of bit array in bytes
    // Operation is ignored if (P2>>5)>=P3
    printf("BITARRAYCLR %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5));

    vm->pc += 7;
}

static void Opcode3E(struct vm_t* vm) {
    // Jump to P3 if [P1+(P2>>5)] & (1<<(P2&31)) is true
    printf("BITARRAYTST %s, %s, .L%04X\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getImm16(vm, 5));

    vm->pc += 7;
}

static void Opcode3F(struct vm_t* vm) {
    // MOD D <- L % R
    printf("MOD %s, %s %% %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5));

    vm->pc += 7;
}

static void Opcode40(struct vm_t* vm) {

    // *** Official Name: SETBITWORK

    // Creates a bitmask to merge two values
    // MASK = ((1 << (P2 - P1)) - 1) << P1
    // P3 = ((P4 << P1) & MASK) | (P3 & ~MASK)
    //
    // BITMASK P3 <- P4, [P1, P2]
    printf("BITMASK %s, %s, b[%s, %s]\n",
        getVar16Name(vm, 5),
        getVar16Name(vm, 7),
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 9;
}

static void Opcode41(struct vm_t* vm) {

    // *** Official Name: GETBITWORK

    // Creates a bitmask to extract a value
    // MASK = ((1 << (P2 - P1)) - 1) << P1
    // P3 = (P4 & MASK) >> P1
    //
    // BITUNMASK P3 <- P4, [P1, P2]
    printf("BITUNMASK %s, %s, b[%s, %s]\n",
        getVar16Name(vm, 5),
        getVar16Name(vm, 7),
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 9;
}

static void Opcode42(struct vm_t* vm) {
    printf("Opcode42\n");
    vm->pc += 1;
}

static void Opcode43(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("UPDATEEVENT\n");
        vm->pc += 2;
    } else if (param == 1) {
        printf("UPDATEEVENT WAIT\n");
        vm->pc += 2;
    } else {
        vm->running = 0;
    }
}

static void Opcode44(struct vm_t* vm) {
    // Jump to P2 if P1 is a valid EntityId in the entity table
    printf("ISENTITY %s, .L%04X\n",
        getVar16Name(vm, 1),
        getImm16(vm, 3));

    vm->pc += 5;
}

static void Opcode45(struct vm_t* vm) {

    // *** Official Name: LOADSCHEDULER

    printf("LOADSCHEDULER %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11),
        getVar16Name(vm, 15));

    vm->pc += 17;
}

static void Opcode46(struct vm_t* vm) {
    uint8_t param = lsb8(vm->code, vm->pc, 1);

    if (param == 0) {
        printf("Opcode46 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 1) {
        printf("Opcode46 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 2) {
        printf("Opcode46 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else {
        vm->running = 0;
    }
}

static void Opcode47(struct vm_t* vm) {
    uint32_t param = getImm8(vm, 1);

    if (param == 0) {
        // Send a Warp Request (0x5c) packet
        printf("WARPPLAYER %s, %s, %s, %s\n",
            getVar16NameFloat100(vm, 2),
            getVar16NameFloat100(vm, 4),
            getVar16NameFloat100(vm, 6),
            getVar16NameRadians(vm, 8));

        vm->pc += 10;
    } else if (param == 1) {
        printf("WARPPLAYER WAIT\n");

        vm->pc += 2;
    } else {
        vm->running = 0;
    }
}

static void Opcode48(struct vm_t* vm) {
    const char* message = getVar16Message(vm, 1);

    if (message != NULL) {
        printf("PRINT %s // %s\n",
            getVar16Name(vm, 1),
            message);
        FreePrintableText(message);
    } else {
        printf("PRINT %s\n",
            getVar16Name(vm, 1));
    }

    vm->pc += 3;
}

static void Opcode49(struct vm_t* vm) {
    const char* message = getVar16Message(vm, 5);

    if (message != NULL) {
        printf("PRINT2 %s, %s // %s\n",
            getVar32Name(vm, 1),
            getVar16Name(vm, 5),
            message);
        FreePrintableText(message);
    } else {
        printf("PRINT2 %s, %s\n",
            getVar32Name(vm, 1),
            getVar16Name(vm, 5));
    }

    vm->pc += 7;
}

static void Opcode4A(struct vm_t* vm) {

    // *** Official Name: DTURA

    printf("DTURA %s, %s\n",
        getVar32Name(vm, 1),
        getVar32Name(vm, 5));

    vm->pc += 9;
}

static void Opcode4B(struct vm_t* vm) {
    printf("Opcode4B %s, %s\n",
        getVar32Name(vm, 1),
        getVar16NameRadians(vm, 5));

    vm->pc += 7;
}

static void Opcode4C(struct vm_t* vm) {
    printf("OPENDOOR\n");

    vm->pc += 1;
}

static void Opcode4D(struct vm_t* vm) {
    printf("CLOSEDOOR\n");

    vm->pc += 1;
}

static void Opcode4E(struct vm_t* vm) {
    printf("Opcode4E %02x, %s\n",
        lsb8(vm->code, vm->pc, 1),
        getVar32Name(vm, 2));

    vm->pc += 6;
}

static void Opcode4F(struct vm_t* vm) {
    printf("Opcode4F %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void Opcode50(struct vm_t* vm) {

    // *** Official Name: ENDSCHEDULOR

    printf("ENDSCHEDULER %s, %s, %s\n",
        getVar32Name(vm, 1),
        getVar32Name(vm, 5),
        getVar32FourCC(vm, 9));

    vm->pc += 13;
}

static void Opcode51(struct vm_t* vm) {

    // *** Official Name: ENDMAPSCHEDULOR

    printf("ENDMAPSCHEDULER %s, %s, %s\n",
        getVar32Name(vm, 1),
        getVar32Name(vm, 5),
        getVar32FourCC(vm, 9));

    vm->pc += 13;
}

static void Opcode52(struct vm_t* vm) {

    // *** Official Name: ENDLOADSCHEDULER

    printf("ENDLOADSCHEDULER %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void Opcode53(struct vm_t* vm) {

    // *** Official Name: WAITSCHEDULOR

    printf("WAITSCHEDULER %s, %s, %s\n",
        getVar32Name(vm, 1),
        getVar32Name(vm, 5),
        getVar32FourCC(vm, 9));

    vm->pc += 13;
}

static void Opcode54(struct vm_t* vm) {

    // *** Official Name: WAITMAPSCHEDULOR

    printf("WAITMAPSCHEDULER %s, %s, %s\n",
        getVar32Name(vm, 1),
        getVar32Name(vm, 5),
        getVar32FourCC(vm, 9));

    vm->pc += 13;
}

static void Opcode55(struct vm_t* vm) {

    // *** Official Name: WAITLOADSCHEDULER

    printf("WAITLOADSCHEDULER %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void Opcode56(struct vm_t* vm) {
    printf("Opcode56 %s\n",
        getVar32Name(vm, 1));

    vm->pc += 5;
}

static void Opcode57(struct vm_t* vm) {
    printf("Opcode57 %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void Opcode58(struct vm_t* vm) {
    printf("Opcode58\n");

    vm->pc += 1;
}

static void Opcode59(struct vm_t* vm) {
    uint32_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("Opcode59 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 1) {
        printf("Opcode59 %02x, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar16Name(vm, 6));
        vm->pc += 8;
    } else if (param == 2) {
        printf("Opcode59 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 3) {
        printf("Opcode59 %02x, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar16Name(vm, 6));
        vm->pc += 8;
    } else if (param == 4) {
        printf("Opcode59 %02x, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar16Name(vm, 6));
        vm->pc += 8;
    } else if (param == 5) {
        printf("Opcode59 %02x, %s, #$%02x\n",
            param,
            getVar32Name(vm, 2),
            getImm8(vm, 6));
        vm->pc += 7;
    } else if (param == 6) {
        printf("Opcode59 %02x, %s\n",
            param,
            getVar32Name(vm, 2));
        vm->pc += 6;
    } else if (param == 7) {
        printf("Opcode59 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 8) {
        printf("Opcode59 %02x, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar16Name(vm, 6));
        vm->pc += 8;
    } else {
        vm->running = 0;
    }
}

static void Opcode5A(struct vm_t* vm) {
    uint32_t param = getImm8(vm, 1);

    // *** Official Name: MOVE2

    if (param == 0) {
        printf("MOVETO2 %s, %s, %s\n",
            getVar16NameFloat100(vm, 2),
            getVar16NameFloat100(vm, 4),
            getVar16NameFloat100(vm, 6));

        vm->pc += 8;
    } else if (param == 1) {
        printf("MOVETO2 WAIT\n");
        vm->pc += 2;
    } else {
        vm->running = 0;
    }
}

static void Opcode5B(struct vm_t* vm) {

    // *** Official Name: LOADEXTSCHEDULER

    printf("LOADEXTSCHEDULER %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void Opcode5C(struct vm_t* vm) {
    uint32_t param = getImm8(vm, 1);

    if (param <= 7) {
        printf("Opcode5C %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param >= 0x80 && param <= 0x87) {
        printf("Opcode5C %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param >= 0xa0 && param <= 0xa1) {
        printf("Opcode5C %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else {
        vm->running = 0;
    }
}

static void Opcode5D(struct vm_t* vm) {
    printf("Opcode5D %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));

    vm->pc += 5;
}

static void Opcode5E(struct vm_t* vm) {
    printf("Opcode5E %s\n",
        getVar32FourCC(vm, 1));

    vm->pc += 5;
}

static void Opcode5F(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0 || param == 1) {
        printf("Opcode5F %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 2) {
        printf("Opcode5F %02x, %s\n",
            param,
            getVar32Name(vm, 2));
        vm->pc += 6;
    } else if (param == 3) {
        printf("Opcode5F %02x, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar32Name(vm, 4),
            getVar32Name(vm, 8),
            getVar32FourCC(vm, 12));
        vm->pc += 16;
    } else if (param == 4) {
        printf("Opcode5F %02x, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar32Name(vm, 4),
            getVar32Name(vm, 8),
            getVar32FourCC(vm, 12));
        vm->pc += 16;
    } else if (param == 5) {
        printf("Opcode5F %02x, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar32Name(vm, 4),
            getVar32Name(vm, 8),
            getVar32FourCC(vm, 12),
            getVar16Name(vm, 16));
        vm->pc += 18;
    } else if (param == 6) {
        printf("Opcode5F %02x, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar32Name(vm, 4),
            getVar32Name(vm, 8),
            getVar32FourCC(vm, 12),
            getVar16Name(vm, 16));
        vm->pc += 18;
    } else if (param == 7) {
        printf("Opcode5F %02x, %s, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar32Name(vm, 6),
            getVar32FourCC(vm, 10));
        vm->pc += 14;
    } else {
        vm->running = 0;
    }
}

static void Opcode60(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0 || param == 1) {
        printf("Opcode60 %02x // %02x%02x\n",
            param,
            getImm8(vm, 2),
            getImm8(vm, 3)
            );

        // The last two bytes are ignored
        vm->pc += 4;
    } else if (param == 2) {
        printf("Opcode60 %02x, %s\n",
            param,
            getVar32FourCC(vm, 2));
        vm->pc += 6;
    } else {
        vm->running = 0;
    }
}

static void Opcode61(struct vm_t* vm) {
    printf("Opcode61 %02x\n",
        getImm8(vm, 1));

    vm->pc += 2;
}

static void Opcode62(struct vm_t* vm) {

    // *** Official Name: LOADEVENTSCHEDULER

    printf("LOADEVENTSCHEDULER %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11),
        getVar16Name(vm, 15));

    vm->pc += 17;
}

static void Opcode63(struct vm_t* vm) {
    printf("Opcode63 %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void Opcode64(struct vm_t* vm) {
    printf("Opcode64 %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5),
        getVar16Name(vm, 7),
        getVar16Name(vm, 9));

    vm->pc += 11;
}

static void Opcode65(struct vm_t* vm) {

    // *** Official Name: GETDISTANCEAA

    printf("GETDISTANCE %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7));

    vm->pc += 11;
}

static void Opcode66(struct vm_t* vm) {

    // *** Official Name: LOADEXTSCHEDULER2

    printf("LOADEXTSCHEDULER2 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void Opcode67(struct vm_t* vm) {
    printf("Opcode67 %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));

    vm->pc += 5;
}

static void Opcode68(struct vm_t* vm) {
    printf("Opcode68\n");

    vm->pc += 1;
}

static void Opcode69(struct vm_t* vm) {
    printf("Opcode69 %02x, %s\n",
        getImm8(vm, 1),
        getVar16Name(vm, 2));

    vm->pc += 4;
}

static void Opcode6A(struct vm_t* vm) {
    printf("Opcode6A %s, %s, %s\n",
        getVar16NameFloat100(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5));

    vm->pc += 7;
}

static void Opcode6B(struct vm_t* vm) {
    printf("Opcode6B %s, %s\n",
        getVar32FourCC(vm, 1),
        getVar32Name(vm, 5));

    vm->pc += 9;
}

static void Opcode6C(struct vm_t* vm) {

    // *** Official Name: TRANSPAR

    printf("TRANSPAR %s, %s, %s\n",
        getVar32Name(vm, 1),
        getVar16Name(vm, 5),
        getVar16Name(vm, 7));

    vm->pc += 9;
}

static void Opcode6D(struct vm_t* vm) {
    // Doesn't do anything.
    printf("Opcode6D\n");

    vm->pc += 7;
}

static void Opcode6E(struct vm_t* vm) {
    printf("EMOTE %s, %s\n",
        getVar32Name(vm, 1),
        getVar16Name(vm, 5));

    vm->pc += 7;
}

static void Opcode6F(struct vm_t* vm) {
    printf("WAIT16\n");

    vm->pc += 1;
}

static void Opcode70(struct vm_t* vm) {
    printf("Opcode70\n");

    vm->pc += 1;
}

static void Opcode71(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("UPDATEEVENTSTRING WAITFORINPUT\n");
        vm->pc += 2;
    } else if (param == 1) {
        printf("UPDATEEVENTSTRING\n");
        vm->pc += 2;
    } else if (param == 2) {
        printf("UPDATEEVENTSTRING WAIT\n");
        vm->pc += 2;
    } else if (param == 3) {
        printf("Opcode71 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 16) {
        printf("READPASSWORD NUMBER, %s\n",
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 17) {
        printf("READPASSWORD STORENUMBER, %s\n",
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 19) {
        printf("Opcode71 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 18) {
        printf("Opcode71 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));
        vm->pc += 6;
    } else if (param == 32) {
        printf("Opcode71 %02x, %s, %s, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10),
            getVar16Name(vm, 12),
            getVar16Name(vm, 14));
        vm->pc += 16;
    } else if (param == 33) {
        printf("Opcode71 %02x\n",
            param);
        vm->pc += 2;
    } else if (param == 48) {
        printf("Opcode71 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 49) {
        printf("Opcode71 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 50) {
        printf("Opcode71 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));
        vm->pc += 6;
    } else if (param == 64) {
        printf("Opcode71 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 65) {
        printf("Opcode71 %02x, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6));
        vm->pc += 8;
    } else if (param == 80) {
        printf("Opcode71 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 81) {
        printf("Opcode71 %02x\n",
            param);
        vm->pc += 2;
    } else if (param == 82) {
        printf("Opcode71 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else if (param == 83) {
        printf("Opcode71 %02x\n",
            param);
        vm->pc += 2;
    } else if (param == 84) {
        printf("Opcode71 %02x, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));
        vm->pc += 10;
    } else if (param == 85) {
        printf("Opcode71 %02x, %s\n",
            param,
            getVar16Name(vm, 2));
        vm->pc += 4;
    } else {
        vm->running = 0;
    }
}

static void Opcode72(struct vm_t* vm) {

    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        // Load the appropriate Weather resource
        // for the given zone.
        printf("LOADWEATHER %s\n",
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 1) {
        // Read the current weather forecast
        // for the given zone and store in
        // Param0-Param2.
        printf("WEATHER %s, %s\n",
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else {
        vm->running = 0;
    }
}

static void Opcode73(struct vm_t* vm) {

    // *** Official Name: LOADMAGICSCHEDULOR

    printf("LOADMAGICSCHEDULER %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7));

    vm->pc += 11;
}

static void Opcode74(struct vm_t* vm) {
    printf("Opcode74 %02x\n",
        getImm8(vm, 1));

    vm->pc += 2;
}

static void Opcode75(struct vm_t* vm) {
    uint32_t param = lsb8(vm->code, vm->pc, 1);

    if (param == 0) {
        printf("SETSUBREGIONID %s\n",
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 1) {
        printf("SETSUBREGIONID WAIT\n");

        vm->pc += 2;
    } else if (param == 2) {
        // Sneaky. This assumes that the instruction is following
        // a SETSUBREGIONID/SETSUBREGIONID WAIT pair.
        vm->pc = vm->pc - 6;

        printf("SETSUBREGIONID SEND %s\n",
            getVar16Name(vm, 2));

        vm->pc += 8;
    } else {
        vm->running = 0;
    }
}

static void Opcode76(struct vm_t* vm) {
    printf("Opcode76 %s\n",
        getVar32Name(vm, 1));

    vm->pc += 5;
}

static void Opcode77(struct vm_t* vm) {
    // Not quite...?
    // Needs more investigation
    printf("SETWEATHER0 %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));

    vm->pc += 5;
}

static void Opcode78(struct vm_t* vm) {
    printf("RESETWEATHER\n");

    vm->pc += 1;
}

static void Opcode79(struct vm_t* vm) {
    uint32_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("Opcode79 %02x, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar32Name(vm, 6));

        vm->pc += 10;
    } else if (param == 1) {
        printf("Opcode79 %02x, %s, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar32Name(vm, 6),
            getVar32Name(vm, 10));

        vm->pc += 12;
    } else if (param == 2) {
        printf("Opcode79 %02x, %s, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else {
        vm->running = 0;
    }
}

static void Opcode7A(struct vm_t* vm) {
    uint32_t param = lsb8(vm->code, vm->pc, 1);

    if (param == 0) {
        printf("RESETEVENTS %s\n",
            getVar32Name(vm, 2));

        vm->pc += 6;
    } else if (param == 1) {
        uint32_t addr = getImm32(vm, 2);

        printf("RESETEVENT %s, %s\n",
            getVar32Name(vm, 2),
            getEventLabelByAddrAndIndex(vm, addr, 6, lsb8(vm->code, vm->pc, 6)));

        vm->pc += 7;
    } else if (param == 2) {
        printf("SETLOCAL %s\n",
            getVar32Name(vm, 2));

        vm->pc += 6;
    } else if (param == 3) {
        printf("RESETLOCAL\n");

        vm->pc += 2;
    } else if (param == 4) {
        // Name is likely inaccurate...tbd.
        printf("SETPRIORITY %s, #%d, #%2d\n",
            getVar32Name(vm, 3),
            lsb8(vm->code, vm->pc, 2),
            lsb8(vm->code, vm->pc, 7));

        vm->pc += 8;
    } else if (param == 5) {
        // Name is likely inaccurate...tbd.
        printf("RESETPRIORITY %s\n",
            getVar32Name(vm, 2));

        vm->pc += 6;
    } else {
        vm->running = 0;
    }
}

static void Opcode7B(struct vm_t* vm) {
    printf("Opcode7B %s\n",
        getVar32Name(vm, 1));

    vm->pc += 5;
}

static void Opcode7C(struct vm_t* vm) {
    printf("Opcode7C %02x, %s\n",
        lsb8(vm->code, vm->pc, 1),
        getVar32Name(vm, 2));

    vm->pc += 6;
}

static void Opcode7D(struct vm_t* vm) {
    printf("Opcode7D %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void Opcode7E(struct vm_t* vm) {
    uint8_t param = getImm16(vm, 1);

    if (param == 0 || param == 1 || param == 2 || param == 4 || param == 5 || param == 8) {
        printf("Opcode7E %02x, %s\n",
            param,
            getVar32Name(vm, 2));
        vm->pc += 6;
    } else if (param == 3) {
        printf("Opcode7E %02x, %s, %s, %s, %s, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10),
            getVar16Name(vm, 12),
            getVar16Name(vm, 14));
        vm->pc += 16;
    } else if (param == 6) {
        printf("Opcode7E %02x, %s, %s, %s, %s, %s, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10),
            getVar16Name(vm, 12),
            getVar16Name(vm, 14),
            getVar16Name(vm, 16));
        vm->pc += 18;
    } else if (param == 7) {
        printf("Opcode7E %02x, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar16Name(vm, 6));
        vm->pc += 8;
    } else  {
        vm->running = 0;
    }
}

static void Opcode7F(struct vm_t* vm) {
    // A dialog was just shown -- wait for an option select / cancel button press.
    // Cancelling will NOT end the event.

    // *** Official Name: QUERYWAIT2

    printf("WAITFORPROMPTNOCANCEL\n");

    vm->pc += 1;
}

static void Opcode80(struct vm_t* vm) {
    printf("Opcode80 %s\n",
        getVar32Name(vm, 1));

    vm->pc += 5;
}

static void Opcode81(struct vm_t* vm) {
    printf("Opcode81 %02x, %s\n",
        lsb8(vm->code, vm->pc, 1),
        getVar32Name(vm, 2));

    vm->pc += 6;
}

static void Opcode82(struct vm_t* vm) {
    // Jump if entity is in the specified RID
    printf("INRID %s, .L%04X\n",
        getVar32FourCC(vm, 1),
        getImm16(vm, 5));

    vm->pc += 7;
}

static void Opcode83(struct vm_t* vm) {
    // P1 <- number of earth seconds since Mon Dec 31 2001 15:00:00 GMT+0000
    printf("GETEARTHSECONDS %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void Opcode84(struct vm_t* vm) {
    printf("Opcode84\n");

    vm->pc += 1;
}

static void Opcode85(struct vm_t* vm) {
    printf("Opcode85\n");

    vm->pc += 1;
}

static void Opcode86(struct vm_t* vm) {
    printf("Opcode86 %02x, %s\n",
        getImm8(vm, 1),
        getVar32Name(vm, 2));

    vm->pc += 6;
}

static void Opcode87(struct vm_t* vm) {
    // 0 - world pass
    // 1 - wait
    // 2 - gold world pass
    printf("WORLDPASS %02x\n",
        getImm8(vm, 1));

    vm->pc += 2;
}

static void Opcode88(struct vm_t* vm) {
    // 0 - world pass
    // 1 - wait
    // 2 - gold world pass
    printf("WORLDPASSPURCHASE %02x\n",
        getImm8(vm, 1));

    vm->pc += 2;
}

static void Opcode89(struct vm_t* vm) {
    printf("Opcode89 %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void Opcode8A(struct vm_t* vm) {
    printf("Opcode8A\n");

    vm->pc += 1;
}

static void Opcode8B(struct vm_t* vm) {
    // !!!
    uint8_t buf[17];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &vm->code[vm->pc + 9], 16);

    const char* text = GetPrintableText(buf, 16);

    printf("Opcode8B %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16NameFloat100(vm, 5),
        getVar16NameFloat100(vm, 7),
        text);

    FreePrintableText(text);

    vm->pc += 25;
}

static void Opcode8C(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("SYNTHSUGGESTION %02x, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6));

        vm->pc += 8;
    } else if (param == 1) {
        printf("SYNTHSUGGESTION WAIT\n");

        vm->pc += 2;
    } else if (param == 2) {
        printf("SYNTHSUGGESTION %02x, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10));

        vm->pc += 12;
    } else if (param == 3) {
        printf("SYNTHSUGGESTION %02x, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 4) {
        // Campaign Ops
        printf("SYNTHSUGGESTION %02x, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 5) {
        // Campaign Ops
        printf("SYNTHSUGGESTION %02x, %s, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10),
            getVar16Name(vm, 12));

        vm->pc += 14;
    } else {
        vm->running = 0;
    }
}

static void Opcode8D(struct vm_t* vm) {
    printf("Opcode8D %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));

    vm->pc += 5;
}

static void Opcode8E(struct vm_t* vm) {
    printf("Opcode8E\n");

    vm->pc += 1;
}

static void Opcode8F(struct vm_t* vm) {
    printf("Opcode8F\n");

    vm->pc += 1;
}

static void Opcode90(struct vm_t* vm) {
    printf("Opcode90\n");

    vm->pc += 1;
}

static void Opcode91(struct vm_t* vm) {
    printf("SETANIMATIONSPEED %s\n",
        getVar16NameFloatX10(vm, 1));

    vm->pc += 3;
}

static void Opcode92(struct vm_t* vm) {
    printf("Opcode92 %02x, %s\n",
        lsb8(vm->code, vm->pc, 1),
        getVar32Name(vm, 2));

    vm->pc += 6;
}

static void Opcode93(struct vm_t* vm) {
    printf("Opcode93 %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void Opcode94(struct vm_t* vm) {
    printf("Opcode94 %02x, %s\n",
        lsb8(vm->code, vm->pc, 1),
        getVar32Name(vm, 2));

    vm->pc += 6;
}

static void Opcode95(struct vm_t* vm) {
    printf("Opcode95 %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void Opcode96(struct vm_t* vm) {
    printf("Opcode96\n");

    vm->pc += 1;
}

static void Opcode97(struct vm_t* vm) {
    printf("Opcode97 %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));

    vm->pc += 5;
}

static void Opcode98(struct vm_t* vm) {
    printf("Opcode98\n");

    vm->pc += 1;
}

static void Opcode99(struct vm_t* vm) {
    printf("Opcode99 %s\n",
        getVar32Name(vm, 1));

    vm->pc += 5;
}

static void Opcode9A(struct vm_t* vm) {
    printf("Opcode9A\n");
    vm->pc += 1;
}

static void Opcode9B(struct vm_t* vm) {
    printf("Opcode9B\n");

    vm->pc += 1;
}

static void Opcode9C(struct vm_t* vm) {
    printf("Opcode9C %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void Opcode9D(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        // Array access
        printf("READARRAY %s, .L%04X[%s]\n",
            getVar16Name(vm, 4),
            getImm16(vm, 2),
            getVar16Name(vm, 6));

        vm->pc += 8;
    } else if (param == 1) {
        printf("COPYVECTOR %s, .L%04X[%s]\n",
            getVar16Name(vm, 4),
            getImm16(vm, 2),
            getVar16Name(vm, 6));

        vm->pc += 8;
    } else if (param == 2) {
        printf("Opcode9D %02x, .L%04X, %s\n",
            param,
            getImm16(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param == 3) {
        printf("Opcode9D %02x, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6));

        vm->pc += 8;
    } else if (param == 4) {
        printf("Opcode9D %02x, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6));

        vm->pc += 8;
    } else if (param == 5) {
        // Array write
        printf("WRITEARRAY .L%04X[%s], %s\n",
            getImm16(vm, 2),
            getVar16Name(vm, 6),
            getVar16Name(vm, 4));

        vm->pc += 8;
    } else if (param == 6) {
        printf("Opcode9D %02x, .L%04X, %s, %s\n",
            param,
            getImm16(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6));

        vm->pc += 8;
    } else if (param == 7) {
        // CALL by jump table
        printf("CALL .L%04X[%s]\n",
            getImm16(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param == 8) {
        const char* text = GetPrintableText(&vm->code[vm->pc + 5], 16);

        printf("STRCMP %s, %02x, %s, .L%04X\n",
            getVar16Name(vm, 2),
            getImm8(vm, 4),
            text,
            getImm16(vm, 21));

        vm->pc += 23;
    } else if (param == 9) {
        printf("STRCMP %s, %02x, %s, .L%04X\n",
            getVar16Name(vm, 2),
            getImm8(vm, 6),
            getVar16Name(vm, 4),
            getImm16(vm, 7));

        vm->pc += 9;
    } else if (param == 10) {
        // Bounded array access
        printf("READBOUNDEDARRAY %s, .L%04X[%s, %s]\n",
            getVar16Name(vm, 4),
            getImm16(vm, 2),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 11) {
        printf("COPYVECTORBOUNDED %s, .L%04X[%s], %s\n",
            getVar16Name(vm, 4),
            getImm16(vm, 2),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 12) {
        printf("Opcode9D %02x, .L%04X, %s, %s\n",
            param,
            getImm16(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6));

        vm->pc += 8;
    } else if (param == 13) {
        printf("Opcode9D %02x, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 14) {
        printf("Opcode9D %02x, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 15) {
        // Bounded array write
        printf("WRITEBOUNDEDARRAY .L%04X[%s, %s], %s\n",
            getImm16(vm, 2),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 4));

        vm->pc += 10;
    } else if (param == 16) {
        printf("Opcode9D %02x, .L%04X, %s, %s, %s\n",
            param,
            getImm16(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else {
        vm->running = 0;
    }
}

static void Opcode9E(struct vm_t* vm) {
    printf("Opcode9E %02x\n",
        lsb8(vm->code, vm->pc, 1));

    vm->pc += 2;
}

static void Opcode9F(struct vm_t* vm) {
    printf("Opcode9F %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11),
        getVar16Name(vm, 15));

    vm->pc += 17;
}

static void OpcodeA0(struct vm_t* vm) {
    printf("OpcodeA0 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeA1(struct vm_t* vm) {
    printf("OpcodeA1 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeA2(struct vm_t* vm) {
    printf("OpcodeA2 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeA3(struct vm_t* vm) {
    printf("OpcodeA3 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeA4(struct vm_t* vm) {
    printf("OpcodeA4 %02x\n",
        getImm8(vm, 1));

    vm->pc += 2;
}

static void OpcodeA5(struct vm_t* vm) {
    printf("OpcodeA5 %02x\n",
        getImm8(vm, 1));

    vm->pc += 2;
}

static void OpcodeA6(struct vm_t* vm) {
    uint32_t param = getImm8(vm, 1);

    // a.k.a. "Special Release"
    //
    // Only seen once as part of the Windurst 9-2 ending;
    // the script uses the stored value to activate
    // the corresponding subregion.
    // Exact use unknown atm.
    if (param == 0) {
        printf("WINDURST92\n");

        vm->pc += 2;
    } else if (param == 1) {
        printf("WINDURST92 WAIT\n");

        vm->pc += 2;
    } else if (param == 2) {
        printf("WINDURST92 STORE, %s\n",
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else {
        vm->running = 0;
    }
}

static void OpcodeA7(struct vm_t* vm) {
    uint32_t param = lsb8(vm->code, vm->pc, 1);

    // Sends an event update packet and waits for
    // the instance entry response.
    if (param == 0) {
        printf("ENTERINSTANCE\n");

        vm->pc += 2;
    } else if (param == 1) {
        printf("ENTERINSTANCE WAIT, %s\n",
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else {
        vm->running = 0;
    }
}

static void OpcodeA8(struct vm_t* vm) {
    printf("OpcodeA8 %02x, %s, %s\n",
        getImm8(vm, 1),
        getVar16Name(vm, 2),
        getVar16Name(vm, 4));

    vm->pc += 6;
}

static void OpcodeA9(struct vm_t* vm) {
    printf("OpcodeA9 %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void OpcodeAA(struct vm_t* vm) {
    // P1: See GETEARTHSECONDS for how to interpret this value
    // P2 <- VanaYears(P1)
    // P3 <- VanaMonths(P1)
    // P4 <- etc...
    // P5 <- Moon phase?
    printf("EXTRACTVANATIME %s, %s, %s, %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5),
        getVar16Name(vm, 7),
        getVar16Name(vm, 9),
        getVar16Name(vm, 11),
        getVar16Name(vm, 13),
        getVar16Name(vm, 15));

    vm->pc += 17;
}

static void OpcodeAB(struct vm_t* vm) {
    uint8_t param = lsb8(vm->code, vm->pc, 1);

    if (param == 0) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 1) {
         printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 2) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 3) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 4) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 5) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 6) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 7) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 8) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 9) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 10) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 11) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 12) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 13) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 14) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 15) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 16) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 17) {
        printf("OpcodeAB %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 18) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 19) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 20) {
        printf("OpcodeAB %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 21) {
        printf("OpcodeAB %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 22) {
        printf("OpcodeAB %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 23) {
        printf("OpcodeAB %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 24) {
        printf("OpcodeAB %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 25 || param == 26) {
        printf("OpcodeAB %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 27 || param == 28) {
        printf("OpcodeAB %02x, %s\n",
            param,
            getVar32Name(vm, 2));

        vm->pc += 6;
    } else {
        vm->running = 0;
    }
}

static void OpcodeAC(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("OpcodeAC %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 1) {
        printf("OpcodeAC %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 2 || param == 3) {
        printf("OpcodeAC %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 6;
    } else if (param == 4) {
        printf("OpcodeAC %02x, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar16Name(vm, 6));

        vm->pc += 8;
    } else {
        vm->running = 0;
    }
}

static void OpcodeAD(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    printf("OpcodeAD %02x, %s, %s, %s\n",
        param,
        getVar16Name(vm, 2),
        getVar32Name(vm, 4),
        getVar32Name(vm, 8));

    vm->pc += 12;
}

static void OpcodeAE(struct vm_t* vm) {
    uint32_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("SETWEATHER %s, %s\n",
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));
        vm->pc += 6;
    } else if (param == 1) {
        printf("SETNAMECOLOR %s, %s\n",
            getVar32Name(vm, 2),
            getVar16Name(vm, 6));
        vm->pc += 8;
    } else if (param == 2) {
        printf("RESETNAMECOLOR %s\n",
            getVar32Name(vm, 2));
        // last two bytes are ignored
        vm->pc += 8;
    } else if (param == 3) {
        printf("OpcodeAE %02x, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar16Name(vm, 6));
        vm->pc += 8;
    } else if (param == 4) {
        printf("OpcodeAE %02x, %s\n",
            param,
            getVar32Name(vm, 2));
        // last two bytes are ignored
        vm->pc += 8;
    } else if (param == 5) {
        printf("OpcodeAE %02x, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar32Name(vm, 6));
        vm->pc += 10;
    } else if (param == 6) {
        printf("OpcodeAE %02x, %s\n",
            param,
            getVar32Name(vm, 2));
        vm->pc += 6;
    } else if (param == 7) {
        printf("OpcodeAE %02x, %s, %s\n",
            param,
            getVar32Name(vm, 2),
            getVar32FourCC(vm, 6));
        vm->pc += 10;
    } else if (param == 8) {
        printf("OpcodeAE %02x, %s\n",
            param,
            getVar32Name(vm, 2));
        vm->pc += 6;
    } else {
        vm->running = 0;
    }
}

static void OpcodeAF(struct vm_t* vm) {
    printf("OpcodeAF %02x, %s, %s, %s\n",
        getImm8(vm, 1),
        getVar16Name(vm, 2),
        getVar16Name(vm, 4),
        getVar16Name(vm, 6));

    vm->pc += 8;
}

static void OpcodeB0(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        const char* message = getVar16Message(vm, 10);

        if (message != NULL) {
            printf("NPCSAY3 %s, %s, %s // %s\n",
                getVar32Name(vm, 2),
                getVar32Name(vm, 6),
                getVar16Name(vm, 10),
                message);
            FreePrintableText(message);
        } else {
            printf("NPCSAY3 %s, %s, %s\n",
                getVar32Name(vm, 2),
                getVar32Name(vm, 6),
                getVar16Name(vm, 10));
        }

        vm->pc += 12;
    } else {
        vm->running = 0;
    }
}

static void OpcodeB1(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("OpcodeB1 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 1) {
        printf("OpcodeB1 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 2) {
        printf("OpcodeB1 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 3) {
        // Guessing
        printf("OpcodeB1 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 4) {
        // Guessing
        printf("OpcodeB1 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else {
        vm->running = 0;
    }
}

static void OpcodeB2(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("OpcodeB2 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 1) {
        printf("OpcodeB2 %02x\n",
            param);

        vm->pc += 2;
    } else {
        vm->running = 0;
    }
}

static void OpcodeB3(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("OpcodeB3 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 1) {
        printf("OpcodeB3 %02x, %s, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10),
            getVar16Name(vm, 12));

        vm->pc += 14;
    } else if (param == 2) {
        printf("OpcodeB3 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 3) {
        printf("OpcodeB3 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 4) {
        printf("OpcodeB3 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 5) {
        printf("OpcodeB3 %02x, %s, %s, %s, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10),
            getVar16Name(vm, 12),
            getVar16Name(vm, 14),
            getVar16Name(vm, 16));

        vm->pc += 18;
    } else if (param == 6) {
        printf("OpcodeB3 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 7) {
        printf("OpcodeB3 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 8) {
        printf("OpcodeB3 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 9) {
        printf("OpcodeB3 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else {
        vm->running = 0;
    }
}

static void OpcodeB4(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        const char* text = GetPrintableText(&vm->code[vm->pc + 4], 16);

        printf("OpcodeB4 %02x, %s, \"%s\"\n",
            param,
            getVar16Name(vm, 2),
            text);

        FreePrintableText(text);
        vm->pc += 20;
    } else if (param == 1) {
        printf("OpcodeB4 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param == 2) {
        printf("OpcodeB4 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param == 3) {
        printf("OpcodeB4 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 4) {
        printf("OpcodeB4 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param == 5) {
        printf("OpcodeB4 %02x, %02x\n",
            param,
            getImm8(vm, 2));

        vm->pc += 3;
    } else if (param == 6) {
        printf("OpcodeB4 %02x, %02x\n",
            param,
            getImm8(vm, 2));

        vm->pc += 3;
    } else if (param == 7) {
        printf("OpcodeB4 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 8) {
        printf("OpcodeB4 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 9) {
        printf("OpcodeB4 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 10) {
        printf("OpcodeB4 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 11) {
        printf("OpcodeB4 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 12) {
        printf("OpcodeB4 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 13) {
        printf("OpcodeB4 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 14) {
        printf("OpcodeB4 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 15) {
        printf("OpcodeB4 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param == 16) {
        // ???
        printf("OpcodeB4 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param == 17) {
        // ???
        printf("OpcodeB4 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param == 18) {
        // ???
        printf("OpcodeB4 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param == 19) {
        const char* text = GetPrintableText(&vm->code[vm->pc + 4], 16);

        printf("OpcodeB4 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            text);

        FreePrintableText(text);
        vm->pc += 20;
    } else if (param == 20) {
        printf("OpcodeB4 %02x, %s, %s, %s, %s, %s\n",
            param,
            getVar16NameFloat100(vm, 2),
            getVar16NameFloat100(vm, 4),
            getVar16NameFloat100(vm, 6),
            getVar16NameFloat100(vm, 8),
            getVar16Name(vm, 10));

        vm->pc += 12;
    } else if (param == 21) {
        printf("OpcodeB4 %02x\n",
            param);

        vm->pc += 2;
    } else {
        vm->running = 0;
    }
}

static void OpcodeB5(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("SETENTITYNAME %s\n",
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else {
        vm->running = 0;
    }
}

static void OpcodeB6(struct vm_t* vm) {
    // Appearance-related opcodes.
    // Todo - figure out what the later opcodes are doing.
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("SETMODELID %s\n",
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 1) {
         printf("SETRACE %s\n",
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param >= 2 && param <= 10) {
        printf("SETAPPEARANCEBYINDEX %d, %s\n",
            param - 2,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 11) {
        // Race + Appearance
        printf("SETAPPERANCE %s, %s, %s, %s, %s, %s, %s, %s, %s\n",
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10),
            getVar16Name(vm, 12),
            getVar16Name(vm, 14),
            getVar16Name(vm, 16),
            getVar16Name(vm, 18));

        vm->pc += 20;
    } else if (param == 12) {
        // todo - verify name
        printf("SETCHOCOBO %s\n",
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 13) {
        // ???
        printf("OpcodeB6 %02x, %s, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10),
            getVar16Name(vm, 12));

        vm->pc += 14;
    } else if (param == 14) {
        // ???
        printf("OpcodeB6 %02x, %s, %s, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10),
            getVar16Name(vm, 12),
            getVar16Name(vm, 14));

        vm->pc += 16;
    } else if (param == 15) {
        // todo - float * 0.01
        printf("SETMODELSIZE %s\n",
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 16) {
        printf("OpcodeB6 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 17) {
        printf("OpcodeB6 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 18 || param == 19) {
        printf("OpcodeB6 %02x\n",
            param);

        vm->pc += 2;
    } else if (param == 20 || param == 21) {
        printf("OpcodeB6 %02x, %s\n",
            param,
            getVar32Name(vm, 2));

        vm->pc += 6;
    } else {
        vm->running = 0;
    }
}

static void OpcodeB7(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        // Cross-NPC MOV
        // MOV NPC[addr], val
        printf("MOV %s[%s], %s\n",
            getVar32Name(vm, 2),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 1) {
        printf("COPYNAME %s, %s\n",
            getVar16Name(vm, 2),
            getVar32Name(vm, 4));

        vm->pc += 8;
    } else if (param == 2) {
        // MOV addr, EntityId
        printf("MOVENTITYID %s, %s\n",
            getVar16Name(vm, 2),
            getVar32Name(vm, 4));

        vm->pc += 8;
    } else if (param == 3) {
        // MOV addr, EntityIndex
        printf("MOVENTITYINDEX %s, %s\n",
            getVar16Name(vm, 2),
            getVar32Name(vm, 4));

        vm->pc += 8;
    } else if (param == 4) {
        // setact_userindex_to_mesactornameindex
        // ???
        printf("OpcodeB7 04, %s, %s\n",
            getVar16Name(vm, 2),
            getVar32Name(vm, 4));

        vm->pc += 8;
    } else {
        vm->running = 0;
    }
}

static void OpcodeB8(struct vm_t* vm) {
    // !!!
    uint8_t buf[17];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &vm->code[vm->pc + 11], 16);

    const char* text = GetPrintableText(buf, 16);

    printf("OpcodeB8 %s, %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5),
        getVar16NameFloat100(vm, 7),
        getVar16NameFloat100(vm, 9),
        text);

    FreePrintableText(text);

    vm->pc += 27;
}

static void OpcodeB9(struct vm_t* vm) {
    printf("OpcodeB9 %02x, %s, %s, %s\n",
        getImm8(vm, 1),
        getVar16Name(vm, 2),
        getVar16Name(vm, 4),
        getVar16Name(vm, 6));

    vm->pc += 8;
}

static void OpcodeBA(struct vm_t* vm) {
    printf("OpcodeBA %s, %s, %s, %s, %s\n",
        getVar32Name(vm, 1),
        getVar16NameFloat100(vm, 5),
        getVar16NameFloat100(vm, 7),
        getVar16NameFloat100(vm, 9),
        getVar16NameRadians(vm, 11));

    vm->pc += 13;
}

static void OpcodeBB(struct vm_t* vm) {
    printf("OpcodeBB %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11),
        getVar16Name(vm, 15));

    vm->pc += 17;
}

static void OpcodeBC(struct vm_t* vm) {
    printf("OpcodeBC %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeBD(struct vm_t* vm) {
    printf("OpcodeBD %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeBE(struct vm_t* vm) {
    printf("OpcodeBE %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void OpcodeBF(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        printf("CHOCOBORACE GetRaceParam, %s, %s, %s\n",
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6));

        vm->pc += 8;
    } else if (param == 32) {
        printf("CHOCOBORACE GetChocoboParam, %s, %s, %s, %s\n",
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 64) {
        printf("CHOCOBORACE GetSectionParam, %s, %s, %s, %s\n",
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 96) {
        printf("CHOCOBORACE GetResultParam, %s, %s, %s\n",
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6));

        vm->pc += 8;
    } else {
        vm->running = 0;
    }
}

static void OpcodeC0(struct vm_t* vm) {
    printf("OpcodeC0 %s\n",
        getVar16Name(vm, 1));

    vm->pc += 3;
}

static void OpcodeC1(struct vm_t* vm) {
    printf("OpcodeC1 %s\n",
        getVar32Name(vm, 1));

    vm->pc += 5;
}

static void OpcodeC2(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 1) {
        printf("OpcodeC2 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 2) {
        printf("OpcodeC2 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else {
        vm->running = 0;
    }
}

static void OpcodeC3(struct vm_t* vm) {
    printf("OpcodeC3 %s, %s, %s\n",
        // array index (stride 20 bytes)
        getVar16Name(vm, 1),
        // ptr to work area, copies 16 bytes to element
        getVar16Name(vm, 3),
        // the remaining 4 bytes to copy into the element
        getVar16Name(vm, 5));

    vm->pc += 7;
}

static void OpcodeC4(struct vm_t* vm) {
    printf("OpcodeC4 %02x, %s, %s, %s\n",
        getImm8(vm, 1),
        getVar16Name(vm, 2),
        getVar32Name(vm, 4),
        getVar32Name(vm, 8));

    vm->pc += 12;
}

static void OpcodeC5(struct vm_t* vm) {
    printf("OpcodeC5 %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11),
        getVar16Name(vm, 15));

    vm->pc += 17;
}

static void OpcodeC6(struct vm_t* vm) {
    printf("OpcodeC6 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeC7(struct vm_t* vm) {
    printf("OpcodeC7 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeC8(struct vm_t* vm) {
    printf("OpcodeC8 %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5));

    vm->pc += 7;
}

static void OpcodeC9(struct vm_t* vm) {
    printf("OpcodeC9\n");

    vm->pc += 1;
}

static void OpcodeCC(struct vm_t* vm) {

    uint8_t param = getImm8(vm, 1);

    if (param == 0 || param == 1 || param == 3) {
        printf("OpcodeCC %02x, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8));

        vm->pc += 10;
    } else if (param == 2) {
        printf("OpcodeCC %02x, %s, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10),
            getVar16Name(vm, 12));

        vm->pc += 14;
    } else if (param == 16) {
        printf("OpcodeCC %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else if (param == 17) {
        printf("OpcodeCC %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else if (param == 32) {
        printf("OpcodeCC %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    } else {
        vm->running = 0;
    }
}

static void OpcodeCD(struct vm_t* vm) {
    printf("OpcodeCD %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11),
        getVar16Name(vm, 15));

    vm->pc += 17;
}

static void OpcodeCE(struct vm_t* vm) {
    printf("OpcodeCE %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeCF(struct vm_t* vm) {
    printf("OpcodeCF %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeD0(struct vm_t* vm) {
    printf("OpcodeD0 %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11),
        getVar16Name(vm, 15));

    vm->pc += 17;
}

static void OpcodeD1(struct vm_t* vm) {
    printf("OpcodeD1 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeD2(struct vm_t* vm) {
    printf("OpcodeD2 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeD3(struct vm_t* vm) {
    printf("OpcodeD3 %02x, %s\n",
        getImm8(vm, 1),
        getVar32Name(vm, 2));

    vm->pc += 6;
}

static void OpcodeD4(struct vm_t* vm) {
    uint32_t param = getImm8(vm, 1);

    if (param == 0) {
        const char* message = getVar16Message(vm, 2);

        if (message != NULL) {
            printf("PROMPT2 %s, %s, %s // %s\n",
                getVar16Name(vm, 2),
                getVar16Name(vm, 4),
                getVar16Name(vm, 6),
                message);
            FreePrintableText(message);
        } else {
            printf("PROMPT2 %s, %s, %s\n",
                getVar16Name(vm, 2),
                getVar16Name(vm, 4),
                getVar16Name(vm, 6));
        }
        vm->pc += 8;
    } else if (param == 1) {
        printf("OpcodeD4 %02x, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6));
        vm->pc += 8;
    } else if (param == 2) {
        const char* message = getVar16Message(vm, 2);

        if (message != NULL) {
            printf("PROMPT3 %s, %s, %s // %s\n",
                getVar16Name(vm, 2),
                getVar16Name(vm, 4),
                getVar16Name(vm, 6),
                message);
            FreePrintableText(message);
        } else {
            printf("PROMPT3 %s, %s, %s\n",
                getVar16Name(vm, 2),
                getVar16Name(vm, 4),
                getVar16Name(vm, 6));
        }
        vm->pc += 8;
    } else if (param == 3) {
        printf("OpcodeD4 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));
        vm->pc += 6;
    } else if (param == 4 || param == 5) {
        printf("OpcodeD4 %02x, %s, %s, %s, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4),
            getVar16Name(vm, 6),
            getVar16Name(vm, 8),
            getVar16Name(vm, 10));
        vm->pc += 12;
    } else {
        vm->running = 0;
    }
}

static void OpcodeD5(struct vm_t* vm) {
    printf("OpcodeD5 %s, %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11),
        getVar16Name(vm, 15));

    vm->pc += 17;
}

static void OpcodeD6(struct vm_t* vm) {
    printf("OpcodeD6 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeD7(struct vm_t* vm) {
    printf("OpcodeD7 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar32Name(vm, 3),
        getVar32Name(vm, 7),
        getVar32FourCC(vm, 11));

    vm->pc += 15;
}

static void OpcodeD8(struct vm_t* vm) {
    uint8_t param = getImm8(vm, 1);

    if (param == 0) {
        // Copies rotation to LocalWorkZone
        printf("COPYROTATION %s\n",
            getVar32Name(vm, 2));

        vm->pc += 6;
    } else if (param == 1) {
        // Sets LocalWorkZone.Rotation.X
        printf("SETROTATIONX %s, %s\n",
            getVar32Name(vm, 2),
            getVar16NameRadians(vm, 6));

        vm->pc += 8;
    } else if (param == 2) {
        // Sets LocalWorkZone.Rotation.Y
        printf("SETROTATIONY %s, %s\n",
            getVar32Name(vm, 2),
            getVar16NameRadians(vm, 6));

        vm->pc += 8;
    } else if (param == 3) {
        // Sets LocalWorkZone.Rotation.Z
        printf("SETROTATIONZ %s, %s\n",
            getVar32Name(vm, 2),
            getVar16NameRadians(vm, 6));

        vm->pc += 8;
    } else if (param == 4) {
        // Sets LocalWorkZone.Rotation.XYZ
        printf("SETROTATIONXYZ %s, %s, %s, %s\n",
            getVar32Name(vm, 2),
            getVar16NameRadians(vm, 6),
            getVar16NameRadians(vm, 8),
            getVar16NameRadians(vm, 10));

        vm->pc += 12;
    } else {
        vm->running = 0;
    }
}

static void OpcodeD9(struct vm_t* vm) {
    // A newer opcode;
    // Unknown what it does.
    uint8_t param = getImm8(vm, 1);

    printf("OpcodeD9 %02x\n", param);

    vm->pc += 2;
}

static const OpcodeFunc OpcodeTable[256] = {
    [0x00] = Opcode00,
    [0x01] = Opcode01,
    [0x02] = Opcode02,
    [0x03] = Opcode03,
    [0x04] = Opcode04,
    [0x05] = Opcode05,
    [0x06] = Opcode06,
    [0x07] = Opcode07,
    [0x08] = Opcode08,
    [0x09] = Opcode09,
    [0x0a] = Opcode0A,
    [0x0b] = Opcode0B,
    [0x0c] = Opcode0C,
    [0x0d] = Opcode0D,
    [0x0e] = Opcode0E,
    [0x0f] = Opcode0F,
    [0x10] = Opcode10,
    [0x11] = Opcode11,
    [0x12] = Opcode12,
    [0x13] = Opcode13,
    [0x14] = Opcode14,
    [0x15] = Opcode15,
    [0x16] = Opcode16,
    [0x17] = Opcode17,
    [0x18] = Opcode18,
    [0x19] = Opcode19,
    [0x1a] = Opcode1A,
    [0x1b] = Opcode1B,
    [0x1c] = Opcode1C,
    [0x1d] = Opcode1D,
    [0x1e] = Opcode1E,
    [0x1f] = Opcode1F,
    [0x20] = Opcode20,
    [0x21] = Opcode21,
    [0x22] = Opcode22,
    [0x23] = Opcode23,
    [0x24] = Opcode24,
    [0x25] = Opcode25,
    [0x26] = Opcode26,
    [0x27] = Opcode27,
    [0x28] = Opcode28,
    [0x29] = Opcode29,
    [0x2a] = Opcode2A,
    [0x2b] = Opcode2B,
    [0x2c] = Opcode2C,
    [0x2d] = Opcode2D,
    [0x2e] = Opcode2E,
    [0x2f] = Opcode2F,
    [0x30] = Opcode30,
    [0x31] = Opcode31,
    [0x32] = Opcode32,
    [0x33] = Opcode33,
    [0x34] = Opcode34,
    [0x35] = Opcode35,
    [0x36] = Opcode36,
    [0x37] = Opcode37,
    [0x38] = Opcode38,
    [0x39] = Opcode39,
    [0x3a] = Opcode3A,
    [0x3b] = Opcode3B,
    [0x3c] = Opcode3C,
    [0x3d] = Opcode3D,
    [0x3e] = Opcode3E,
    [0x3f] = Opcode3F,
    [0x40] = Opcode40,
    [0x41] = Opcode41,
    [0x42] = Opcode42,
    [0x43] = Opcode43,
    [0x44] = Opcode44,
    [0x45] = Opcode45,
    [0x46] = Opcode46,
    [0x47] = Opcode47,
    [0x48] = Opcode48,
    [0x49] = Opcode49,
    [0x4a] = Opcode4A,
    [0x4b] = Opcode4B,
    [0x4c] = Opcode4C,
    [0x4d] = Opcode4D,
    [0x4e] = Opcode4E,
    [0x4f] = Opcode4F,
    [0x50] = Opcode50,
    [0x51] = Opcode51,
    [0x52] = Opcode52,
    [0x53] = Opcode53,
    [0x54] = Opcode54,
    [0x55] = Opcode55,
    [0x56] = Opcode56,
    [0x57] = Opcode57,
    [0x58] = Opcode58,
    [0x59] = Opcode59,
    [0x5a] = Opcode5A,
    [0x5b] = Opcode5B,
    [0x5c] = Opcode5C,
    [0x5d] = Opcode5D,
    [0x5e] = Opcode5E,
    [0x5f] = Opcode5F,
    [0x60] = Opcode60,
    [0x61] = Opcode61,
    [0x62] = Opcode62,
    [0x63] = Opcode63,
    [0x64] = Opcode64,
    [0x65] = Opcode65,
    [0x66] = Opcode66,
    [0x67] = Opcode67,
    [0x68] = Opcode68,
    [0x69] = Opcode69,
    [0x6a] = Opcode6A,
    [0x6b] = Opcode6B,
    [0x6c] = Opcode6C,
    [0x6d] = Opcode6D,
    [0x6e] = Opcode6E,
    [0x6f] = Opcode6F,
    [0x70] = Opcode70,
    [0x71] = Opcode71,
    [0x72] = Opcode72,
    [0x73] = Opcode73,
    [0x74] = Opcode74,
    [0x75] = Opcode75,
    [0x76] = Opcode76,
    [0x77] = Opcode77,
    [0x78] = Opcode78,
    [0x79] = Opcode79,
    [0x7a] = Opcode7A,
    [0x7b] = Opcode7B,
    [0x7c] = Opcode7C,
    [0x7d] = Opcode7D,
    [0x7e] = Opcode7E,
    [0x7f] = Opcode7F,
    [0x80] = Opcode80,
    [0x81] = Opcode81,
    [0x82] = Opcode82,
    [0x83] = Opcode83,
    [0x84] = Opcode84,
    [0x85] = Opcode85,
    [0x86] = Opcode86,
    [0x87] = Opcode87,
    [0x88] = Opcode88,
    [0x89] = Opcode89,
    [0x8a] = Opcode8A,
    [0x8b] = Opcode8B,
    [0x8c] = Opcode8C,
    [0x8d] = Opcode8D,
    [0x8e] = Opcode8E,
    [0x8f] = Opcode8F,
    [0x90] = Opcode90,
    [0x91] = Opcode91,
    [0x92] = Opcode92,
    [0x93] = Opcode93,
    [0x94] = Opcode94,
    [0x95] = Opcode95,
    [0x96] = Opcode96,
    [0x97] = Opcode97,
    [0x98] = Opcode98,
    [0x99] = Opcode99,
    [0x9a] = Opcode9A,
    [0x9b] = Opcode9B,
    [0x9c] = Opcode9C,
    [0x9d] = Opcode9D,
    [0x9e] = Opcode9E,
    [0x9f] = Opcode9F,
    [0xa0] = OpcodeA0,
    [0xa1] = OpcodeA1,
    [0xa2] = OpcodeA2,
    [0xa3] = OpcodeA3,
    [0xa4] = OpcodeA4,
    [0xa5] = OpcodeA5,
    [0xa6] = OpcodeA6,
    [0xa7] = OpcodeA7,
    [0xa8] = OpcodeA8,
    [0xa9] = OpcodeA9,
    [0xaa] = OpcodeAA,
    [0xab] = OpcodeAB,
    [0xac] = OpcodeAC,
    [0xad] = OpcodeAD,
    [0xae] = OpcodeAE,
    [0xaf] = OpcodeAF,
    [0xb0] = OpcodeB0,
    [0xb1] = OpcodeB1,
    [0xb2] = OpcodeB2,
    [0xb3] = OpcodeB3,
    [0xb4] = OpcodeB4,
    [0xb5] = OpcodeB5,
    [0xb6] = OpcodeB6,
    [0xb7] = OpcodeB7,
    [0xb8] = OpcodeB8,
    [0xb9] = OpcodeB9,
    [0xba] = OpcodeBA,
    [0xbb] = OpcodeBB,
    [0xbc] = OpcodeBC,
    [0xbd] = OpcodeBD,
    [0xbe] = OpcodeBE,
    [0xbf] = OpcodeBF,
    [0xc0] = OpcodeC0,
    [0xc1] = OpcodeC1,
    [0xc2] = OpcodeC2,
    [0xc3] = OpcodeC3,
    [0xc4] = OpcodeC4,
    [0xc5] = OpcodeC5,
    [0xc6] = OpcodeC6,
    [0xc7] = OpcodeC7,
    [0xc8] = OpcodeC8,
    [0xc9] = OpcodeC9,
    [0xca] = OpcodeUNSUP,
    [0xcb] = OpcodeUNSUP,
    [0xcc] = OpcodeCC,
    [0xcd] = OpcodeCD,
    [0xce] = OpcodeCE,
    [0xcf] = OpcodeCF,
    [0xd0] = OpcodeD0,
    [0xd1] = OpcodeD1,
    [0xd2] = OpcodeD2,
    [0xd3] = OpcodeD3,
    [0xd4] = OpcodeD4,
    [0xd5] = OpcodeD5,
    [0xd6] = OpcodeD6,
    [0xd7] = OpcodeD7,
    [0xd8] = OpcodeD8,
    [0xd9] = OpcodeD9,
    [0xda] = OpcodeUNSUP,
    [0xdb] = OpcodeUNSUP,
    [0xdc] = OpcodeUNSUP,
    [0xdd] = OpcodeUNSUP,
    [0xde] = OpcodeUNSUP,
    [0xdf] = OpcodeUNSUP,
    [0xe0] = OpcodeUNSUP,
    [0xe1] = OpcodeUNSUP,
    [0xe2] = OpcodeUNSUP,
    [0xe3] = OpcodeUNSUP,
    [0xe4] = OpcodeUNSUP,
    [0xe5] = OpcodeUNSUP,
    [0xe6] = OpcodeUNSUP,
    [0xe7] = OpcodeUNSUP,
    [0xe8] = OpcodeUNSUP,
    [0xe9] = OpcodeUNSUP,
    [0xea] = OpcodeUNSUP,
    [0xeb] = OpcodeUNSUP,
    [0xec] = OpcodeUNSUP,
    [0xed] = OpcodeUNSUP,
    [0xee] = OpcodeUNSUP,
    [0xef] = OpcodeUNSUP,
    [0xf0] = OpcodeUNSUP,
    [0xf1] = OpcodeUNSUP,
    [0xf2] = OpcodeUNSUP,
    [0xf3] = OpcodeUNSUP,
    [0xf4] = OpcodeUNSUP,
    [0xf5] = OpcodeUNSUP,
    [0xf6] = OpcodeUNSUP,
    [0xf7] = OpcodeUNSUP,
    [0xf8] = OpcodeUNSUP,
    [0xf9] = OpcodeUNSUP,
    [0xfa] = OpcodeUNSUP,
    [0xfb] = OpcodeUNSUP,
    [0xfc] = OpcodeUNSUP,
    [0xfd] = OpcodeUNSUP,
    [0xfe] = OpcodeUNSUP,
    [0xff] = OpcodeUNSUP,
};

int ParseScript(const struct event_zone_t* eventZone, uint32_t index, const struct dialog_t* dialog, const struct npc_t* npc, int verbose) {

    const struct event_npc_t* eventNpc = &eventZone->events[index];

    struct vm_t vm;

    vm.code = eventNpc->bytecode;
    vm.pc = 0;
    vm.running = 1;
    vm.length = eventNpc->bytecodeSize;

    vm.constants = eventNpc->constants;
    vm.numConstants = eventNpc->numConstants;

    vm.eventZone = eventZone;
    vm.dialog = dialog;
    vm.npc = npc;

    vm.tracerState = (struct tracer_state_t*) calloc(vm.length + 1, sizeof(struct tracer_state_t));
    vm.tracerCount = 0;

    firstpass(&vm, eventNpc);
    vm.pc = 0;
    vm.running = 1;

    while (vm.running != 0 && vm.pc < vm.length) {
        int isJumpTarget = 0;
        int isData = 0;
        int isEnd = 0;

        struct tracer_state_t* state = FindTracerState(&vm, vm.pc);

        if (state != NULL) {
            isJumpTarget = IS_JUMP(state);
            isData = IS_DATA(state);
            isEnd = IS_END(state);
        }

        if (isEnd) {
            printf("\n");
        }

        if (isJumpTarget) {
            isData = 0;
            isEnd = 0;
        }

        if (isEnd || isData) {
            uint32_t addrJmp = vm.length;

            for (uint32_t i = 0; i < vm.tracerCount; i++) {
                struct tracer_state_t* state = &vm.tracerState[i];

                if (IS_JUMP(state) || IS_DATA(state)) {
                    if (state->address > vm.pc && state->address < addrJmp) {
                        addrJmp = state->address;
                    }
                }
            }

            if (!isData) {
                printf("        # Possibly unreachable code or garbage data:\n");
            }

            printf(".L%04X: .db $", vm.pc);
            while (vm.pc < addrJmp) {
                printf("%02x", vm.code[vm.pc]);
                vm.pc++;
            }
            printf("\n\n");

            continue;
        }

        for (uint32_t i = 0; i < eventNpc->numEvents; i++) {
            const struct event_t* event = &eventNpc->events[i];

            if (event->pc == vm.pc) {
                printf("EventEntryPoint_%u_%u.L%04X:\n", eventNpc->NPCId, event->id, vm.pc);
            }
        }

        if (isJumpTarget) {
            printf(".L%04X: ", vm.pc);
        } else {
            printf("        ");
        }

        OpcodeTable[vm.code[vm.pc]](&vm);

        if (vm.running == 0 && vm.pc < vm.length) {
            // Disassembly ended early, so dump the state for debugging purposes.
            printf("# PC@%04X: ", vm.pc);
            for (uint32_t i = vm.pc; i < vm.length && i < vm.pc + 64; i++) {
                printf("%02X", vm.code[i]);
            }
            printf("\n");

            // Look for a suitable point to resume disassembly.
            uint32_t addrJmp = vm.length;

            for (uint32_t i = 0; i < vm.tracerCount; i++) {
                struct tracer_state_t* state = &vm.tracerState[i];

                if (state->address > vm.pc && state->address < addrJmp) {
                    addrJmp = state->address;
                }
            }

            if (addrJmp < vm.length) {
                printf("# Halted. // %04x: %02x\n", vm.pc, vm.code[vm.pc]);
                printf("# Skipping ahead to L%04X...\n", addrJmp);

                vm.running = 1;
                vm.pc = addrJmp;
            }
        }
    }

    if (vm.length == 0) {
        printf("# Nothing to do.\n");
    } else if (vm.pc == vm.length) {
        if (verbose) {
            printf("# Finished!\n");
        }
    } else if (vm.pc >= vm.length) {
        printf("# Overrun.\n");
    } else if (vm.running == 0) {
        printf("# Halted. // %04x: %02x\n", vm.pc, vm.code[vm.pc]);
    } else {
        printf("# Underrun\n");
    }

    return 0;
}
