#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "bytes.h"
#include "dialog.h"
#include "npc.h"
#include "script.h"

struct vm_t {
    const uint8_t* code;

    const uint32_t* constants;

    uint32_t pc;
    uint32_t running;
    uint32_t length;
    uint32_t numConstants;

    const struct dialog_t* dialog;
    const struct npc_t* npc;
};

typedef void (*OpcodeFunc)(struct vm_t*);

static char tmpBuf[32][256];

static const char* getVar16Name(const struct vm_t* vm, uint32_t off) {
    uint16_t addr = lsb16(vm->code, vm->pc, off);

    char* buf = &tmpBuf[off][0];
    if (addr == 0x1000) {
        return "SelectedOption";
    } else if (addr == 0x1001) {
        return "EventResult";
    } else if (addr >= 0x1002 && addr <= 0x1009) {
        sprintf(buf, "Param%d", addr - 0x1002);
    } else if (addr >= 0x8000 && addr < (0x8000 + vm->numConstants)) {
        uint32_t constant = vm->constants[addr - 0x8000];
        if (constant == 0x40000000) {
            return "OptionCancel";
        } else {
            sprintf(buf, "%u", constant);
        }
    } else {
        sprintf(buf, "[%04x]", addr);
    }

    return buf;
}

static const char* getVar32Name(const struct vm_t* vm, uint32_t off) {
    uint16_t addr = lsb32(vm->code, vm->pc, off);

    char* buf = &tmpBuf[off][0];
    if (addr >= 0x8000 && addr < (0x8000 + vm->numConstants)) {
        uint32_t constant = vm->constants[addr - 0x8000];
        if (constant == 0x40000000) {
            return "OptionCancel";
        } else {
            sprintf(buf, "%u", constant);
        }
    } else {
        sprintf(buf, "[%04x]", addr);
    }

    return buf;
}

static char* getVar16Message(const struct vm_t* vm, uint32_t off) {
    if (vm->dialog != NULL) {
        uint16_t addr = lsb16(vm->code, vm->pc, off);

        if (addr >= 0x8000) {
            uint32_t constant = vm->constants[addr - 0x8000];

            if (constant < vm->dialog->numEntries) {
                const struct dialog_entry_t* entry = &vm->dialog->entries[constant];

                const uint8_t* rawMessage = entry->text;
                uint32_t rawMessageSize = entry->length;

                char* message = (char*) calloc(16, rawMessageSize + 1);
                char* pMsg = message;

                // This is going to need to live in dialog, and have it handle
                // Shift-JIS to UTF-8 or whatever.
                for (uint32_t i = 0; i < rawMessageSize; i++) {
                    uint8_t b = rawMessage[i];

                    if (isprint(b)) {
                        *pMsg = b;
                        pMsg++;
                    } else if (b == 0x92 && (i+1) < rawMessageSize && rawMessage[i+1] >= 0 && rawMessage[i+1] <= 9) {
                        char tmp[16];
                        sprintf(tmp, "[S_P][Param%d]", rawMessage[i+1]);
                        strcpy(pMsg, tmp);
                        pMsg += strlen(tmp);
                        i++;
                    } else if (b == 0xa && (i+1) < rawMessageSize && rawMessage[i+1] >= 0 && rawMessage[i+1] <= 9) {
                        char tmp[16];
                        sprintf(tmp, "[Num][Param%d]", rawMessage[i+1]);
                        strcpy(pMsg, tmp);
                        pMsg += strlen(tmp);
                        i++;
                    } else if (b == 0xc && (i+1) < rawMessageSize && rawMessage[i+1] >= 0 && rawMessage[i+1] <= 9) {
                        char tmp[16];
                        sprintf(tmp, "[Idx][Param%d]", rawMessage[i+1]);
                        strcpy(pMsg, tmp);
                        pMsg += strlen(tmp);
                        i++;
                    } else {
                        char tmp[16];
                        sprintf(tmp, "<%02x>", b);
                        strcpy(pMsg, tmp);
                        pMsg += strlen(tmp);
                    }
                }
                return message;
            }
        }
    }

    return NULL;
}

static void OpcodeUNSUP(struct vm_t* vm) {
     printf("UNSUP %02x\n",
        lsb8(vm->code, vm->pc));
    vm->running = 0;
}

static void Opcode00(struct vm_t* vm) {
    // vm->running = 0;

    printf("End\n");

    // Just skip over it for disassembly purposes.
    vm->pc += 1;
}

static void Opcode01(struct vm_t* vm) {
    // This is what the code appears to do but tends to be off a few bytes at times?
    printf("JMP %04x\n", lsb16(vm->code, vm->pc, 1));
    vm->pc += 3;
}

static void Opcode02(struct vm_t* vm) {
    // CMP A, B, Op, Dest

    const char* Op = "<#>";

    // These may be backwards!
    // Need to redo in a more-intuitive syntax.
    switch (lsb8(vm->code, vm->pc, 5) & 0x0f) {
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
            Op = ">";
            break;
        case 5:
            Op = "<";
            break;
        case 6:
        case 9:
            Op = "!&";
            break;
        case 8:
            Op = "!|";
            break;
        case 10:
            Op = "~!&";
            break;
    }

    printf("CMP %s %s %s, %04x\n",
        getVar16Name(vm, 1),
        Op,
        getVar16Name(vm, 3),
        lsb16(vm->code, vm->pc, 6));
    vm->pc += 8;
}

static void Opcode03(struct vm_t* vm) {
    printf("MOV %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
}

static void Opcode04(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode05(struct vm_t* vm) {
    printf("MOV %s, #1\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
};

static void Opcode06(struct vm_t* vm) {
    printf("MOV %s, #0\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
};

static void Opcode07(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode08(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode09(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode0A(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode0B(struct vm_t* vm) {
    printf("ADD %s, #1\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
};

static void Opcode0C(struct vm_t* vm) {
    printf("SUB %s, #-1\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
};

static void Opcode0D(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode0E(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode0F(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode10(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode11(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode12(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode13(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode14(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode15(struct vm_t* vm) {
    // DIV Num <- Num, Div
    printf("DIV %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3));
    vm->pc += 5;
};

static void Opcode16(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode17(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode18(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode19(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode1A(struct vm_t* vm) {
    printf("Opcode1A %s\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
};

static void Opcode1B(struct vm_t* vm) {
    printf("Opcode1A\n");
    vm->pc += 1;
}

static void Opcode1C(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode1D(struct vm_t* vm) {
    // Thought: Prints message to user.
    char* message = getVar16Message(vm, 1);

    if (message != NULL) {
        printf("Opcode1D %s // %s\n",
            getVar16Name(vm, 1),
            message);
        free(message);
    } else {
        printf("Opcode1D %s\n",
            getVar16Name(vm, 1));
    }

    vm->pc += 3;
};

static void Opcode1E(struct vm_t* vm) {
    printf("Opcode1E %s\n",
        getVar32Name(vm, 1));
    vm->pc += 5;
};

static void Opcode1F(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode20(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode21(struct vm_t* vm) {
    printf("Opcode21\n");
    vm->pc += 1;
};

static void Opcode22(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode23(struct vm_t* vm) {
    // Thought: Causes NPC to turn towards you.
    printf("Opcode23\n");
    vm->pc += 1;
};

static void Opcode24(struct vm_t* vm) {
    char* message = getVar16Message(vm, 1);

    if (message != NULL) {
        printf("Opcode24 %s, %s, %s // %s\n",
            getVar16Name(vm, 1),
            getVar16Name(vm, 3),
            getVar16Name(vm, 5),
            message);
        free(message);
    } else {
        printf("Opcode24 %s, %s, %s\n",
            getVar16Name(vm, 1),
            getVar16Name(vm, 3),
            getVar16Name(vm, 5));
    }

    vm->pc += 7;
};

static void Opcode25(struct vm_t* vm) {
    printf("Opcode25\n");
    vm->pc += 1;
};

static void Opcode26(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode27(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode28(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode29(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode2A(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode2B(struct vm_t* vm) {
    printf("Opcode2B %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 5));
    vm->pc += 7;
}

static void Opcode2C(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode2D(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode2E(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode2F(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode30(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode31(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode32(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode33(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode34(struct vm_t* vm) {
    printf("Opcode34 %s\n",
        getVar16Name(vm, 1));
    vm->pc += 3;
};

static void Opcode35(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode36(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode37(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode38(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode39(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode3A(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode3B(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode3C(struct vm_t* vm) {
    printf("Opcode3C %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5));

    vm->pc += 7;
};

static void Opcode3D(struct vm_t* vm) {
    printf("Opcode3D %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5));

    vm->pc += 7;
};

static void Opcode3E(struct vm_t* vm) {
    printf("Opcode3E %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5));

    vm->pc += 7;
};

static void Opcode3F(struct vm_t* vm) {
    printf("Opcode3F %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5));

    vm->pc += 7;
};

static void Opcode40(struct vm_t* vm) {
    printf("Opcode40 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5),
        getVar16Name(vm, 7));
    vm->pc += 9;
};

static void Opcode41(struct vm_t* vm) {
    printf("Opcode41 %s, %s, %s, %s\n",
        getVar16Name(vm, 1),
        getVar16Name(vm, 3),
        getVar16Name(vm, 5),
        getVar16Name(vm, 7));
    vm->pc += 9;
};

static void Opcode42(struct vm_t* vm) {
    printf("Opcode42\n");
    vm->pc += 1;
};

static void Opcode43(struct vm_t* vm) {
    printf("Opcode43 %02x\n",
        lsb8(vm->code, vm->pc, 1));
    vm->pc += 2;
}

static void Opcode44(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
}

static void Opcode45(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
}

static void Opcode46(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
}

static void Opcode47(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
}

static void Opcode48(struct vm_t* vm) {
    // Thought: prints message to user
    char* message = getVar16Message(vm, 1);

    if (message != NULL) {
        printf("Opcode48 %s // %s\n",
            getVar16Name(vm, 1),
            message);
        free(message);
    } else {
        printf("Opcode48 %s\n",
            getVar16Name(vm, 1));
    }

    vm->pc += 3;
}

static void Opcode49(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
}

static void Opcode4A(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
}

static void Opcode4B(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
}

static void Opcode4C(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode4D(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode4E(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode4F(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode50(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode51(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode52(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode53(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode54(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode55(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode56(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode57(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode58(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode59(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode5A(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode5B(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode5C(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode5D(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode5E(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode5F(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode60(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode61(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode62(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode63(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode64(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode65(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode66(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode67(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode68(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode69(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode6A(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode6B(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode6C(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode6D(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode6E(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode6F(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode70(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode71(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
}

static void Opcode72(struct vm_t* vm) {

    uint32_t param = lsb8(vm->code, vm->pc, 1);

    if (param != 0) {
        printf("Opcode41 %02x, %s, %s\n",
            param,
            getVar16Name(vm, 2),
            getVar16Name(vm, 4));

        vm->pc += 6;
    } else {
        printf("Opcode41 %02x, %s\n",
            param,
            getVar16Name(vm, 2));

        vm->pc += 4;
    }
}

static void Opcode73(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode74(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode75(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode76(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode77(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode78(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode79(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode7A(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode7B(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode7C(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode7D(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode7E(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode7F(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode80(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode81(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode82(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode83(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode84(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode85(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode86(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode87(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode88(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode89(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode8A(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode8B(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode8C(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode8D(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode8E(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode8F(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode90(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode91(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode92(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode93(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode94(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode95(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode96(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode97(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode98(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode99(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode9A(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode9B(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode9C(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode9D(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode9E(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void Opcode9F(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeA0(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeA1(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeA2(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeA3(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeA4(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeA5(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeA6(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeA7(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeA8(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeA9(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeAA(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeAB(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeAC(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeAD(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeAE(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeAF(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeB0(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeB1(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeB2(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeB3(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeB4(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeB5(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeB6(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeB7(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeB8(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeB9(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeBA(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeBB(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeBC(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeBD(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeBE(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeBF(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeC0(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeC1(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeC2(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeC3(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeC4(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeC5(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeC6(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeC7(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeC8(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeC9(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeCA(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeCB(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeCC(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeCD(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeCE(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeCF(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeD0(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeD1(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeD2(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeD3(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeD4(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeD5(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeD6(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeD7(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

static void OpcodeD8(struct vm_t* vm) {
    // todo - Some other stuffs.
    vm->running = 0;
};

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
    [0xca] = OpcodeCA,
    [0xcb] = OpcodeCB,
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
    [0xd9] = OpcodeUNSUP,
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

int ParseScript(const uint8_t* script, uint32_t length, const uint32_t* eventOffsets, const uint32_t* eventIds, uint32_t numEvents, const uint32_t* constants, uint32_t numConstants, const struct dialog_t* dialog, const struct npc_t* npc) {

    struct vm_t vm;

    vm.code = script;
    vm.pc = 0;
    vm.running = 1;
    vm.length = length;

    vm.constants = constants;
    vm.numConstants = numConstants;

    vm.dialog = dialog;
    vm.npc = npc;

    while (vm.running != 0 && vm.pc < vm.length) {
        for (uint32_t i = 0; i < numEvents; i++) {
            if (eventOffsets[i] == vm.pc) {
                printf("EventEntryPoint_%u:\n", eventIds[i]);
            }
        }

        printf("%04x: ", vm.pc);
        OpcodeTable[vm.code[vm.pc]](&vm);
    }

    if (vm.length == 0) {
        printf("# Nothing to do.\n");
    } else if (vm.pc == vm.length/* && vm.code[vm.pc-1] == 0x00*/) {
        printf("# Finished!\n");
    } else if (vm.pc >= vm.length) {
        printf("# Overrun.\n");
    } else if (vm.running == 0) {
        printf("# Halted. // %04x: %02x\n", vm.pc, vm.code[vm.pc]);
    } else {
        printf("# Underrun\n");
    }

    return 0;
}
