#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "bytes.h"
#include "event.h"
#include "firstpass.h"
#include "scriptint.h"

typedef int (*OpcodeSpecial)(struct vm_t* vm);

typedef struct Opcode {
    OpcodeSpecial special;
    uint16_t opcode;
    int16_t length;
} Opcode;

static uint16_t getImm16(const struct vm_t* vm, uint32_t off) {
   return lsb16(vm->code, vm->pc, off);
}

static void TrackEnd(struct vm_t* vm, uint32_t addr) {
    for (uint32_t i = 0; i < vm->addrEndLen; i++) {
        if (vm->addrEnd[i].destination == addr) {
            return;
        }
    }
    struct reference_t ref = {
        .location = vm->pc,
        .destination = addr,
        .end = 0,
        .active = 1
    };
    vm->addrEnd[vm->addrEndLen++] = ref;
}

static void TrackJmp(struct vm_t* vm, uint32_t addr) {
    for (uint32_t i = 0; i < vm->addrJmpLen; i++) {
        if (vm->addrJmp[i].destination == addr) {
            return;
        }
    }
    struct reference_t ref = {
        .location = vm->pc,
        .destination = addr,
        .end = 0,
        .active = 1
    };
    vm->addrJmp[vm->addrJmpLen++] = ref;
}

static void TrackData(struct vm_t* vm, uint32_t addr) {
    for (uint32_t i = 0; i < vm->addrDataLen; i++) {
        if (vm->addrData[i].destination == addr) {
            return;
        }
    }
    struct reference_t ref = {
        .location = vm->pc,
        .destination = addr,
        .end = 0,
        .active = 1
    };
    vm->addrData[vm->addrDataLen++] = ref;
}

static int OpcodeEND(struct vm_t* vm) {
    TrackEnd(vm, vm->pc + 1);
    return 0;
}

static int OpcodeJMP(struct vm_t* vm) {
    TrackJmp(vm, getImm16(vm, 1));
    TrackEnd(vm, vm->pc + 3);
    return 0;
}

static int OpcodeCMP(struct vm_t* vm) {
    TrackJmp(vm, getImm16(vm, 6));
    return 1;
}

static int OpcodeCALL(struct vm_t* vm) {
    TrackJmp(vm, getImm16(vm, 1));
    return 1;
}

static int OpcodeRET(struct vm_t* vm) {
    TrackEnd(vm, vm->pc + 1);
    return 0;
}

static int OpcodeSTOP(struct vm_t* vm) {
    TrackEnd(vm, vm->pc + 1);
    return 0;
}

static int OpcodeHALT(struct vm_t* vm) {
    TrackEnd(vm, vm->pc + 1);
    return 0;
}

static int OpcodeBITARRAYTST(struct vm_t* vm) {
    TrackJmp(vm, getImm16(vm, 5));
    return 1;
}

static int OpcodeISENTITY(struct vm_t* vm) {
    TrackJmp(vm, getImm16(vm, 3));
    return 1;
}

static int OpcodeINRID(struct vm_t* vm) {
    TrackJmp(vm, getImm16(vm, 5));
    return 1;
}

static int OpcodeREADARRAY(struct vm_t* vm) {
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

static int OpcodeCOPYVECTOR(struct vm_t* vm) {
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

static int Opcode9D02(struct vm_t* vm) {
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

static int OpcodeWRITEARRAY(struct vm_t* vm) {
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

static int Opcode9D06(struct vm_t* vm) {
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

static int OpcodeCALLTABLE(struct vm_t* vm) {
    // TODO - special tracking for the jump table
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

static int OpcodeSTRCMPIMM(struct vm_t* vm) {
    TrackJmp(vm, getImm16(vm, 21));
    return 1;
}

static int OpcodeSTRCMP(struct vm_t* vm) {
    TrackJmp(vm, getImm16(vm, 7));
    return 1;
}

static int OpcodeREADBOUNDEDARRAY(struct vm_t* vm) {
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

static int OpcodeCOPYVECTORBOUNDED(struct vm_t* vm) {
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

static int Opcode9D0C(struct vm_t* vm) {
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

static int OpcodeWRITEBOUNDEDARRAY(struct vm_t* vm) {
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

static int Opcode9D10(struct vm_t* vm) {
    TrackData(vm, getImm16(vm, 2));
    return 1;
}

// static int OpcodeB104(struct vm_t* vm) {
//    // TrackEnd(vm, vm->pc);
//    return 1;
// }

#define OPCODE(o, l)            { .opcode = o, .length = l, .special = NULL }
#define OPCODE_IS_TWO(o)        { .opcode = o, .length = -1, .special = NULL }
#define OPCODE_SPECIAL(o, l, s) { .opcode = o, .length = l, .special = s }

static const Opcode OpcodeTable[] = {
    OPCODE_SPECIAL(0x00  ,  1, OpcodeEND              ),                                         // END
    OPCODE_SPECIAL(0x01  ,  3, OpcodeJMP              ),                                         // JMP imm16
    OPCODE_SPECIAL(0x02  ,  8, OpcodeCMP              ),
    OPCODE        (0x03  ,  5                         ),
    OPCODE        (0x04  ,  3                         ),
    OPCODE        (0x05  ,  3                         ),
    OPCODE        (0x06  ,  3                         ),
    OPCODE        (0x07  ,  5                         ),
    OPCODE        (0x08  ,  5                         ),
    OPCODE        (0x09  ,  5                         ),
    OPCODE        (0x0A  ,  5                         ),
    OPCODE        (0x0B  ,  3                         ),
    OPCODE        (0x0C  ,  3                         ),
    OPCODE        (0x0D  ,  5                         ),
    OPCODE        (0x0E  ,  5                         ),
    OPCODE        (0x0F  ,  5                         ),
    OPCODE        (0x10  ,  5                         ),
    OPCODE        (0x11  ,  5                         ),
    OPCODE        (0x12  ,  3                         ),
    OPCODE        (0x13  ,  5                         ),
    OPCODE        (0x14  ,  5                         ),
    OPCODE        (0x15  ,  5                         ),
    OPCODE        (0x16  ,  7                         ),
    OPCODE        (0x17  ,  7                         ),
    OPCODE        (0x18  ,  7                         ),
    OPCODE        (0x19  ,  5                         ),
    OPCODE_SPECIAL(0x1A  ,  3, OpcodeCALL             ),
    OPCODE_SPECIAL(0x1B  ,  1, OpcodeRET              ),
    OPCODE        (0x1C  ,  3                         ),
    OPCODE        (0x1D  ,  3                         ),
    OPCODE        (0x1E  ,  5                         ),
    OPCODE_IS_TWO (0x1F                               ),
    OPCODE        (0x1F00,  8                         ),
    OPCODE        (0x1F01,  2                         ),
    OPCODE        (0x20  ,  2                         ),
    OPCODE_SPECIAL(0x21  ,  1, OpcodeSTOP             ),
    OPCODE        (0x22  ,  2                         ),
    OPCODE        (0x23  ,  1                         ),
    OPCODE        (0x24  ,  7                         ),
    OPCODE        (0x25  ,  1                         ),
    OPCODE_SPECIAL(0x26  ,  1, OpcodeHALT             ),
    OPCODE        (0x27  ,  7                         ),
    OPCODE        (0x28  ,  7                         ),
    OPCODE        (0x29  ,  7                         ),
    OPCODE        (0x2A  ,  6                         ),
    OPCODE        (0x2B  ,  7                         ),
    OPCODE        (0x2C  , 13                         ),
    OPCODE        (0x2D  , 13                         ),
    OPCODE        (0x2E  ,  1                         ),
    OPCODE        (0x2F  ,  6                         ),
    OPCODE        (0x30  ,  1                         ),
    OPCODE_IS_TWO (0x31                               ),
    OPCODE        (0x3100, 10                         ),
    OPCODE        (0x3101,  2                         ),
    OPCODE        (0x32  ,  3                         ),
    OPCODE        (0x33  ,  2                         ),
    OPCODE        (0x34  ,  3                         ),
    OPCODE        (0x35  ,  3                         ),
    OPCODE        (0x36  ,  7                         ),
    OPCODE        (0x37  ,  9                         ),
    OPCODE        (0x38  ,  3                         ),
    OPCODE        (0x39  ,  3                         ),
    OPCODE        (0x3A  ,  7                         ),
    OPCODE        (0x3B  , 11                         ),
    OPCODE        (0x3C  ,  7                         ),
    OPCODE        (0x3D  ,  7                         ),
    OPCODE_SPECIAL(0x3E  ,  7, OpcodeBITARRAYTST      ),
    OPCODE        (0x3F  ,  7                         ),
    OPCODE        (0x40  ,  9                         ),
    OPCODE        (0x41  ,  9                         ),
    OPCODE        (0x42  ,  1                         ),
    OPCODE_IS_TWO (0x43                               ),
    OPCODE        (0x4300,  2                         ),
    OPCODE        (0x4301,  2                         ),
    OPCODE_SPECIAL(0x44  ,  5, OpcodeISENTITY         ),
    OPCODE        (0x45  , 17                         ),
    OPCODE_IS_TWO (0x46                               ),
    OPCODE        (0x4600,  2                         ),
    OPCODE        (0x4601,  2                         ),
    OPCODE        (0x4602,  4                         ),
    OPCODE_IS_TWO (0x47                               ),
    OPCODE        (0x4700, 10                         ),
    OPCODE        (0x4701,  2                         ),
    OPCODE        (0x48  ,  3                         ),
    OPCODE        (0x49  ,  7                         ),
    OPCODE        (0x4A  ,  9                         ),
    OPCODE        (0x4B  ,  7                         ),
    OPCODE        (0x4C  ,  1                         ),
    OPCODE        (0x4D  ,  1                         ),
    OPCODE        (0x4E  ,  6                         ),
    OPCODE        (0x4F  ,  3                         ),
    OPCODE        (0x50  , 13                         ),
    OPCODE        (0x51  , 13                         ),
    OPCODE        (0x52  , 15                         ),
    OPCODE        (0x53  , 13                         ),
    OPCODE        (0x54  , 13                         ),
    OPCODE        (0x55  , 15                         ),
    OPCODE        (0x56  ,  5                         ),
    OPCODE        (0x57  ,  3                         ),
    OPCODE        (0x58  ,  1                         ),
    OPCODE_IS_TWO (0x59                               ),
    OPCODE        (0x5900,  4                         ),
    OPCODE        (0x5901,  8                         ),
    OPCODE        (0x5902,  4                         ),
    OPCODE        (0x5903,  8                         ),
    OPCODE        (0x5904,  8                         ),
    OPCODE        (0x5905,  7                         ),
    OPCODE        (0x5906,  6                         ),
    OPCODE        (0x5907,  4                         ),
    OPCODE        (0x5908,  8                         ),
    OPCODE_IS_TWO (0x5A                               ),
    OPCODE        (0x5A00,  8                         ),
    OPCODE        (0x5A01,  2                         ),
    OPCODE        (0x5B  , 15                         ),
    OPCODE_IS_TWO (0x5C                               ),
    OPCODE        (0x5C00,  4                         ),
    OPCODE        (0x5C01,  4                         ),
    OPCODE        (0x5C02,  4                         ),
    OPCODE        (0x5C03,  4                         ),
    OPCODE        (0x5C04,  4                         ),
    OPCODE        (0x5C05,  4                         ),
    OPCODE        (0x5C06,  4                         ),
    OPCODE        (0x5C07,  4                         ),
    OPCODE        (0x5C80,  6                         ),
    OPCODE        (0x5C81,  6                         ),
    OPCODE        (0x5C82,  6                         ),
    OPCODE        (0x5C83,  6                         ),
    OPCODE        (0x5C84,  6                         ),
    OPCODE        (0x5C85,  6                         ),
    OPCODE        (0x5C86,  6                         ),
    OPCODE        (0x5C87,  6                         ),
    OPCODE        (0x5CA0,  6                         ),
    OPCODE        (0x5CA1,  6                         ),
    OPCODE        (0x5D  ,  5                         ),
    OPCODE        (0x5E  ,  5                         ),
    OPCODE_IS_TWO (0x5F                               ),
    OPCODE        (0x5F00,  2                         ),
    OPCODE        (0x5F01,  2                         ),
    OPCODE        (0x5F02,  6                         ),
    OPCODE        (0x5F03, 16                         ),
    OPCODE        (0x5F04, 16                         ),
    OPCODE        (0x5F05, 18                         ),
    OPCODE        (0x5F06, 18                         ),
    OPCODE        (0x5F07, 14                         ),
    OPCODE_IS_TWO (0x60                               ),
    OPCODE        (0x6000,  4                         ),
    OPCODE        (0x6001,  4                         ),
    OPCODE        (0x6002,  6                         ),
    OPCODE        (0x61  ,  2                         ),
    OPCODE        (0x62  , 17                         ),
    OPCODE        (0x63  ,  3                         ),
    OPCODE        (0x64  , 11                         ),
    OPCODE        (0x65  , 11                         ),
    OPCODE        (0x66  , 15                         ),
    OPCODE        (0x67  ,  5                         ),
    OPCODE        (0x68  ,  1                         ),
    OPCODE        (0x69  ,  4                         ),
    OPCODE        (0x6A  ,  7                         ),
    OPCODE        (0x6B  ,  9                         ),
    OPCODE        (0x6C  ,  9                         ),
    OPCODE        (0x6D  ,  7                         ),
    OPCODE        (0x6E  ,  7                         ),
    OPCODE        (0x6F  ,  1                         ),
    OPCODE        (0x70  ,  1                         ),
    OPCODE_IS_TWO (0x71                               ),
    OPCODE        (0x7100,  2                         ),
    OPCODE        (0x7101,  2                         ),
    OPCODE        (0x7102,  2                         ),
    OPCODE        (0x7103,  4                         ),
    OPCODE        (0x7110,  4                         ),
    OPCODE        (0x7111,  4                         ),
    OPCODE        (0x7112,  6                         ),
    OPCODE        (0x7113,  4                         ),
    OPCODE        (0x7120, 16                         ),
    OPCODE        (0x7121,  2                         ),
    OPCODE        (0x7130,  4                         ),
    OPCODE        (0x7131,  4                         ),
    OPCODE        (0x7132,  6                         ),
    OPCODE        (0x7140,  4                         ),
    OPCODE        (0x7141,  8                         ),
    OPCODE        (0x7150,  4                         ),
    OPCODE        (0x7151,  2                         ),
    OPCODE        (0x7152,  4                         ),
    OPCODE        (0x7153,  2                         ),
    OPCODE        (0x7154, 10                         ),
    OPCODE        (0x7155,  4                         ),
    OPCODE_IS_TWO (0x72                               ),
    OPCODE        (0x7200,  4                         ),
    OPCODE        (0x7201,  6                         ),
    OPCODE        (0x73  , 11                         ),
    OPCODE        (0x74  ,  2                         ),
    OPCODE_IS_TWO (0x75                               ),
    OPCODE        (0x7500,  4                         ),
    OPCODE        (0x7501,  2                         ),
    OPCODE        (0x7502,  2                         ),
    OPCODE        (0x76  ,  5                         ),
    OPCODE        (0x77  ,  5                         ),
    OPCODE        (0x78  ,  1                         ),
    OPCODE_IS_TWO (0x79                               ),
    OPCODE        (0x7900, 10                         ),
    OPCODE        (0x7901, 12                         ),
    OPCODE        (0x7902, 10                         ),
    OPCODE_IS_TWO (0x7A                               ),
    OPCODE        (0x7A00,  6                         ),
    OPCODE        (0x7A01,  7                         ),
    OPCODE        (0x7A02,  6                         ),
    OPCODE        (0x7A03,  2                         ),
    OPCODE        (0x7A04,  8                         ),
    OPCODE        (0x7A05,  6                         ),
    OPCODE        (0x7B  ,  5                         ),
    OPCODE        (0x7C  ,  6                         ),
    OPCODE        (0x7D  ,  3                         ),
    OPCODE_IS_TWO (0x7E                               ),
    OPCODE        (0x7E00,  6                         ),
    OPCODE        (0x7E01,  6                         ),
    OPCODE        (0x7E02,  6                         ),
    OPCODE        (0x7E03, 16                         ),
    OPCODE        (0x7E04,  6                         ),
    OPCODE        (0x7E05,  6                         ),
    OPCODE        (0x7E06, 18                         ),
    OPCODE        (0x7E07,  8                         ),
    OPCODE        (0x7E08,  6                         ),
    OPCODE        (0x7F  ,  1                         ),
    OPCODE        (0x80  ,  5                         ),
    OPCODE        (0x81  ,  6                         ),
    OPCODE_SPECIAL(0x82  ,  7, OpcodeINRID            ),
    OPCODE        (0x83  ,  3                         ),
    OPCODE        (0x84  ,  1                         ),
    OPCODE        (0x85  ,  1                         ),
    OPCODE        (0x86  ,  6                         ),
    OPCODE        (0x87  ,  2                         ),
    OPCODE        (0x88  ,  2                         ),
    OPCODE        (0x89  ,  3                         ),
    OPCODE        (0x8A  ,  1                         ),
    OPCODE        (0x8B  , 25                         ),
    OPCODE_IS_TWO (0x8C                               ),
    OPCODE        (0x8C00,  8                         ),
    OPCODE        (0x8C01,  2                         ),
    OPCODE        (0x8C02, 12                         ),
    OPCODE        (0x8C03, 10                         ),
    OPCODE        (0x8C04, 10                         ),
    OPCODE        (0x8C05, 14                         ),
    OPCODE        (0x8D  ,  5                         ),
    OPCODE        (0x8E  ,  1                         ),
    OPCODE        (0x8F  ,  1                         ),
    OPCODE        (0x90  ,  1                         ),
    OPCODE        (0x91  ,  3                         ),
    OPCODE        (0x92  ,  6                         ),
    OPCODE        (0x93  ,  3                         ),
    OPCODE        (0x94  ,  6                         ),
    OPCODE        (0x95  ,  3                         ),
    OPCODE        (0x96  ,  1                         ),
    OPCODE        (0x97  ,  5                         ),
    OPCODE        (0x98  ,  1                         ),
    OPCODE        (0x99  ,  5                         ),
    OPCODE        (0x9A  ,  1                         ),
    OPCODE        (0x9B  ,  1                         ),
    OPCODE        (0x9C  ,  3                         ),
    OPCODE_IS_TWO (0x9D                               ),
    OPCODE_SPECIAL(0x9D00,  8, OpcodeREADARRAY        ),
    OPCODE_SPECIAL(0x9D01,  8, OpcodeCOPYVECTOR       ),
    OPCODE_SPECIAL(0x9D02,  6, Opcode9D02             ),
    OPCODE        (0x9D03,  8                         ),
    OPCODE        (0x9D04,  8                         ),
    OPCODE_SPECIAL(0x9D05,  8, OpcodeWRITEARRAY       ),
    OPCODE_SPECIAL(0x9D06,  8, Opcode9D06             ),
    OPCODE_SPECIAL(0x9D07,  6, OpcodeCALLTABLE        ),
    OPCODE_SPECIAL(0x9D08, 23, OpcodeSTRCMPIMM        ),
    OPCODE_SPECIAL(0x9D09,  9, OpcodeSTRCMP           ),
    OPCODE_SPECIAL(0x9D0A, 10, OpcodeREADBOUNDEDARRAY ),
    OPCODE_SPECIAL(0x9D0B, 10, OpcodeCOPYVECTORBOUNDED),
    OPCODE_SPECIAL(0x9D0C,  8, Opcode9D0C             ),
    OPCODE        (0x9D0D, 10                         ),
    OPCODE        (0x9D0E, 10                         ),
    OPCODE_SPECIAL(0x9D0F, 10, OpcodeWRITEBOUNDEDARRAY),
    OPCODE_SPECIAL(0x9D10, 10, Opcode9D10             ),
    OPCODE        (0x9E  ,  2                         ),
    OPCODE        (0x9F  , 17                         ),
    OPCODE        (0xA0  , 15                         ),
    OPCODE        (0xA1  , 15                         ),
    OPCODE        (0xA2  , 15                         ),
    OPCODE        (0xA3  , 15                         ),
    OPCODE        (0xA4  ,  2                         ),
    OPCODE        (0xA5  ,  2                         ),
    OPCODE_IS_TWO (0xA6                               ),
    OPCODE        (0xA600,  2                         ),
    OPCODE        (0xA601,  2                         ),
    OPCODE        (0xA602,  4                         ),
    OPCODE_IS_TWO (0xA7                               ),
    OPCODE        (0xA700,  2                         ),
    OPCODE        (0xA701,  4                         ),
    OPCODE        (0xA8  ,  6                         ),
    OPCODE        (0xA9  ,  3                         ),
    OPCODE        (0xAA  , 17                         ),
    OPCODE_IS_TWO (0xAB                               ),
    OPCODE        (0xAB00,  2                         ),
    OPCODE        (0xAB01,  2                         ),
    OPCODE        (0xAB02,  2                         ),
    OPCODE        (0xAB03,  2                         ),
    OPCODE        (0xAB04,  2                         ),
    OPCODE        (0xAB05,  2                         ),
    OPCODE        (0xAB06,  2                         ),
    OPCODE        (0xAB07,  2                         ),
    OPCODE        (0xAB08,  2                         ),
    OPCODE        (0xAB09,  2                         ),
    OPCODE        (0xAB0A,  2                         ),
    OPCODE        (0xAB0B,  2                         ),
    OPCODE        (0xAB0C,  2                         ),
    OPCODE        (0xAB0D,  2                         ),
    OPCODE        (0xAB0E,  2                         ),
    OPCODE        (0xAB0F,  2                         ),
    OPCODE        (0xAB10,  2                         ),
    OPCODE        (0xAB11,  4                         ),
    OPCODE        (0xAB12,  2                         ),
    OPCODE        (0xAB13,  2                         ),
    OPCODE        (0xAB14,  4                         ),
    OPCODE        (0xAB15,  4                         ),
    OPCODE        (0xAB16,  4                         ),
    OPCODE        (0xAB17,  4                         ),
    OPCODE        (0xAB18,  4                         ),
    OPCODE        (0xAB19,  2                         ),
    OPCODE        (0xAB1A,  2                         ),
    OPCODE        (0xAB1B,  6                         ),
    OPCODE        (0xAB1C,  6                         ),
    OPCODE_IS_TWO (0xAC                               ),
    OPCODE        (0xAC00,  4                         ),
    OPCODE        (0xAC01,  4                         ),
    OPCODE        (0xAC02,  6                         ),
    OPCODE        (0xAC03,  6                         ),
    OPCODE        (0xAC04,  8                         ),
    OPCODE        (0xAD  , 12                         ),
    OPCODE_IS_TWO (0xAE                               ),
    OPCODE        (0xAE00,  6                         ),
    OPCODE        (0xAE01,  8                         ),
    OPCODE        (0xAE02,  8                         ),
    OPCODE        (0xAE03,  8                         ),
    OPCODE        (0xAE04,  8                         ),
    OPCODE        (0xAE05, 10                         ),
    OPCODE        (0xAE06,  6                         ),
    OPCODE        (0xAE07, 10                         ),
    OPCODE        (0xAE08,  6                         ),
    OPCODE        (0xAF  ,  8                         ),
    OPCODE_IS_TWO (0xB0                               ),
    OPCODE        (0xB000, 12                         ),
    OPCODE_IS_TWO (0xB1                               ),
    OPCODE        (0xB100,  4                         ),
    OPCODE        (0xB101,  4                         ),
    OPCODE        (0xB102,  4                         ),
    OPCODE        (0xB103,  4                         ),
    OPCODE        (0xB104,  6                         ),
    OPCODE_IS_TWO (0xB2                               ),
    OPCODE        (0xB200,  4                         ),
    OPCODE        (0xB201,  2                         ),
    OPCODE_IS_TWO (0xB3                               ),
    OPCODE        (0xB300,  4                         ),
    OPCODE        (0xB301, 14                         ),
    OPCODE        (0xB302,  2                         ),
    OPCODE        (0xB303,  4                         ),
    OPCODE        (0xB304,  4                         ),
    OPCODE        (0xB305, 18                         ),
    OPCODE        (0xB306,  4                         ),
    OPCODE        (0xB307,  4                         ),
    OPCODE        (0xB308,  2                         ),
    OPCODE        (0xB309,  4                         ),
    OPCODE_IS_TWO (0xB4                               ),
    OPCODE        (0xB400, 20                         ),
    OPCODE        (0xB401,  6                         ),
    OPCODE        (0xB402,  6                         ),
    OPCODE        (0xB403,  2                         ),
    OPCODE        (0xB404,  6                         ),
    OPCODE        (0xB405,  3                         ),
    OPCODE        (0xB406,  3                         ),
    OPCODE        (0xB407,  4                         ),
    OPCODE        (0xB408,  2                         ),
    OPCODE        (0xB409,  4                         ),
    OPCODE        (0xB40A,  4                         ),
    OPCODE        (0xB40B,  2                         ),
    OPCODE        (0xB40C,  4                         ),
    OPCODE        (0xB40D,  2                         ),
    OPCODE        (0xB40E,  2                         ),
    OPCODE        (0xB40F,  6                         ),
    OPCODE        (0xB410,  6                         ),
    OPCODE        (0xB411,  6                         ),
    OPCODE        (0xB412,  6                         ),
    OPCODE        (0xB413, 20                         ),
    OPCODE        (0xB414, 12                         ),
    OPCODE        (0xB415,  2                         ),
    OPCODE_IS_TWO (0xB5                               ),
    OPCODE        (0xB500,  4                         ),
    OPCODE_IS_TWO (0xB6                               ),
    OPCODE        (0xB600,  4                         ),
    OPCODE        (0xB601,  4                         ),
    OPCODE        (0xB602,  4                         ),
    OPCODE        (0xB603,  4                         ),
    OPCODE        (0xB604,  4                         ),
    OPCODE        (0xB605,  4                         ),
    OPCODE        (0xB606,  4                         ),
    OPCODE        (0xB607,  4                         ),
    OPCODE        (0xB608,  4                         ),
    OPCODE        (0xB609,  4                         ),
    OPCODE        (0xB60A,  4                         ),
    OPCODE        (0xB60B, 20                         ),
    OPCODE        (0xB60C,  4                         ),
    OPCODE        (0xB60D, 14                         ),
    OPCODE        (0xB60E, 16                         ),
    OPCODE        (0xB60F,  4                         ),
    OPCODE        (0xB610,  2                         ),
    OPCODE        (0xB611,  4                         ),
    OPCODE        (0xB612,  2                         ),
    OPCODE        (0xB613,  2                         ),
    OPCODE        (0xB614,  6                         ),
    OPCODE        (0xB615,  6                         ),
    OPCODE_IS_TWO (0xB7                               ),
    OPCODE        (0xB700, 10                         ),
    OPCODE        (0xB701,  8                         ),
    OPCODE        (0xB702,  8                         ),
    OPCODE        (0xB703,  8                         ),
    OPCODE        (0xB704,  8                         ),
    OPCODE        (0xB8  , 27                         ),
    OPCODE        (0xB9  ,  8                         ),
    OPCODE        (0xBA  , 13                         ),
    OPCODE        (0xBB  , 17                         ),
    OPCODE        (0xBC  , 15                         ),
    OPCODE        (0xBD  , 15                         ),
    OPCODE        (0xBE  ,  3                         ),
    OPCODE_IS_TWO (0xBF                               ),
    OPCODE        (0xBF00,  8                         ),
    OPCODE        (0xBF20, 10                         ),
    OPCODE        (0xBF40, 10                         ),
    OPCODE        (0xBF60,  8                         ),
    OPCODE        (0xC0  ,  3                         ),
    OPCODE        (0xC1  ,  5                         ),
    OPCODE_IS_TWO (0xC2                               ),
    OPCODE        (0xC201,  4                         ),
    OPCODE        (0xC202,  6                         ),
    OPCODE        (0xC3  ,  7                         ),
    OPCODE        (0xC4  , 12                         ),
    OPCODE        (0xC5  , 17                         ),
    OPCODE        (0xC6  , 15                         ),
    OPCODE        (0xC7  , 15                         ),
    OPCODE        (0xC8  ,  7                         ),
    OPCODE        (0xC9  ,  1                         ),
    // invalid CA
    // invalid CB
    OPCODE_IS_TWO (0xCC                               ),
    OPCODE        (0xCC00, 10                         ),
    OPCODE        (0xCC01, 10                         ),
    OPCODE        (0xCC02, 14                         ),
    OPCODE        (0xCC03, 10                         ),
    OPCODE        (0xCC10,  6                         ),
    OPCODE        (0xCC11,  4                         ),
    OPCODE        (0xCC20,  4                         ),
    OPCODE        (0xCD  , 17                         ),
    OPCODE        (0xCE  , 15                         ),
    OPCODE        (0xCF  , 15                         ),
    OPCODE        (0xD0  , 17                         ),
    OPCODE        (0xD1  , 15                         ),
    OPCODE        (0xD2  , 15                         ),
    OPCODE        (0xD3  ,  6                         ),
    OPCODE_IS_TWO (0xD4                               ),
    OPCODE        (0xD400,  8                         ),
    OPCODE        (0xD401,  8                         ),
    OPCODE        (0xD402,  8                         ),
    OPCODE        (0xD403,  6                         ),
    OPCODE        (0xD404, 12                         ),
    OPCODE        (0xD405, 12                         ),
    OPCODE        (0xD5  , 17                         ),
    OPCODE        (0xD6  , 15                         ),
    OPCODE        (0xD7  , 15                         ),
    OPCODE_IS_TWO (0xD8                               ),
    OPCODE        (0xD800,  6                         ),
    OPCODE        (0xD801,  8                         ),
    OPCODE        (0xD802,  8                         ),
    OPCODE        (0xD803,  8                         ),
    OPCODE        (0xD804, 12                         ),
    OPCODE        (0xD9  ,  2                         )
};

static const Opcode* FindOpcode(uint16_t op) {
    for (uint32_t i = 0; i < sizeof(OpcodeTable)/sizeof(OpcodeTable[0]); i++) {
        const Opcode* pOp = &OpcodeTable[i];

        if (op == pOp->opcode) {
            return pOp;
        }
    }

    return NULL;
}

void firstpass(struct vm_t* vm, const struct event_npc_t* eventNpc) {
    TrackEnd(vm, vm->length);

    for (uint32_t i = 0; i < eventNpc->numEvents; i++) {
        const struct event_t* event = &eventNpc->events[i];
        TrackJmp(vm, event->pc);
    }

    for (;;) {
        uint32_t addrJmp = 0xffffffff;

        for (uint32_t i = 0; i < vm->addrJmpLen; i++) {
            if (vm->addrJmp[i].active) {
                addrJmp = vm->addrJmp[i].destination;
                vm->addrJmp[i].active = 0;
                break;
            }
        }

        if (addrJmp == 0xffffffff) {
            // Ran out of things to do.
            return;
        }

        vm->pc = addrJmp;

        while (vm->pc < vm->length) {
            uint16_t op = vm->code[vm->pc];

            const Opcode* pOp = FindOpcode(op);
            if (pOp == NULL) {
                printf("# FIRSTPASS ERROR - FindOpcode: %04X -> %02X\n", vm->pc, op);
                break;
            }

            if (pOp->length < 0) {
                if (vm->pc + 1 >= vm->length) {
                    printf("# FIRSTPASS ERROR - Ran out of space: %04X -> %02X\n", vm->pc, op);
                    break;
                }

                op = (op << 8) | vm->code[vm->pc + 1];

                pOp = FindOpcode(op);

                if (pOp == NULL) {
                    printf("# FIRSTPASS ERROR - FindOpcode: %04X -> %04X\n", vm->pc, op);
                    break;
                }
            }

            if (vm->pc + pOp->length > vm->length) {
                printf("# FIRSTPASS ERROR - Ran out of space: %04X -> %02X (%d bytes)\n", vm->pc, op, pOp->length);
                break;
            }

            if (pOp->special) {
                int isCont = pOp->special(vm);

                if (!isCont) {
                    break;
                }
            }

            vm->pc += pOp->length;
        }
    }
}
