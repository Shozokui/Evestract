#include <inttypes.h>

#include "datid.h"

static const int BaseFileIds[][110] = {
    {
        0x9A6C,0x9A6D,0xD8AD,0xD8AE,
        0xD8AB,0x2D,0xD8E5,0xD8E6,
        0x1E,0xD8E8,0x20,0xD8EA,
        0xD8EB,0xD8EC,0xD8ED,0xD8EE,
        0xD8EF,0xD8EF,0xD908,0xD8F0,
        0xD8F1,0xD8F2,0xD8F3,0xD8F4,
        0xD8F9,0xD8FA,0xD90D,0xD914,
        0xD910,0xD90E,0xD916,0xD902,
        0xD911,0xD915,0xD904,0xD90F,
        0xD912,0xD913,0x9,0x4,
        0xD8FF,0x5,0x6,0xD8FB,
        0xD8FE,0x5F,0xD900,0xD901,
        0xD905,0xD906,0xD8D0,0x7,
        0xD8FC,0x8,0xA,0xB,
        0xC,0x9A84,0xD8FD,0xD907,
        0xD920,0xD921,0xD922,0xD923,
        0xD924,0xD925,0xD926,0xD927,
        0xD928,0xD932,0xD929,0xD92B,
        0xD92C,0xD92D,0xD92E,0xD92F,
        0xD930,0xD931,0xD934,0xD933,
        0xD93F,0xD940,0xD941,0xD942,
        0xD945,0xD946,0xD943,0xD944,
        0x50,0x51,0xD93C,0xD91E,
        0xD93E,0xD91D,0xD93D,0xD909,
        0xD90A,0x1B6C,0x1B6E,0x1B70,
        0x1B72,0x1B74,0x1B76,0x1B7A,
        0x1B7E,0x17E8,0xE239,0x10A73,
        0x14D2B,0x0
    },
    {
        0x9A76,0x9A75,0xD8AD,0xD8AE,
        0xD8AB,0x3F,0xD95D,0xD95E,
        0x30,0xD960,0x32,0xD962,
        0xD963,0xD964,0xD965,0xD966,
        0xD8A9,0xD96D,0xD980,0xD8AB,
        0xD969,0xD96A,0xD96B,0xD96C,
        0xD971,0xD972,0xD985,0xD98C,
        0xD988,0xD986,0xD98E,0xD97A,
        0xD989,0xD98D,0xD97C,0xD987,
        0xD98A,0xD98B,0x5B,0x49,
        0xD977,0x4A,0x4B,0xD973,
        0xD976,0x5F,0xD978,0xD979,
        0xD97D,0xD97E,0xD8D0,0x4C,
        0xD974,0x4D,0x55,0x56,
        0x57,0x9A85,0xD975,0xD97F,
        0xD998,0xD999,0xD99A,0xD99B,
        0xD99C,0xD99D,0xD99E,0xD99F,
        0xD9A0,0xD9AA,0xD9A1,0xD9A3,
        0xD9A4,0xD9A5,0xD9A6,0xD9A7,
        0xD9A8,0xD9A9,0xD9AC,0xD9AB,
        0xD9B7,0xD9B8,0xD9B9,0xD9BA,
        0xD9BD,0xD9BE,0xD9BB,0xD9BC,
        0x52,0x51,0xD9B4,0xD996,
        0xD9B6,0xD995,0xD9B5,0xD981,
        0xD982,0x1B6D,0x1B6F,0x1B71,
        0x1B73,0x1B75,0x1B77,0x1B7B,
        0x1B7F,0x1914,0xE259,0x10B9F,
        0x14E57,0x0
    },
    {
        0xD9C2,0xD9C1,0xD9E7,0xD9E6,
        0xD9E0,0x3F,0xD9D5,0xD9D6,
        0xD9D7,0xD9D8,0xD9D9,0xD9DA,
        0xD9DB,0xD9DC,0xD9DD,0xD9DE,
        0xD9DF,0xD9E5,0xD9F8,0xD9E0,
        0xD9E1,0xD9E2,0xD9E3,0xD9E4,
        0xD9E9,0xD9EA,0xD9FD,0xDA04,
        0xDA00,0xD9FE,0xDA06,0xD97A,
        0xDA01,0xDA05,0xD9F4,0xD9FF,
        0xDA02,0xDA03,0xDA0C,0xDA07,
        0xD9EF,0xDA08,0xDA09,0xD9EB,
        0xD9EE,0x5F,0xD9F0,0xD9F1,
        0xD9F5,0xD9F6,0xD8D0,0xDA0A,
        0xD9EC,0xDA0B,0x55,0x56,
        0xDA0F,0x9A86,0xD9ED,0xD9F7,
        0xDA10,0xDA11,0xDA12,0xDA13,
        0xDA14,0xDA15,0xDA16,0xDA17,
        0xDA18,0xDA22,0xDA19,0xDA1B,
        0xDA1C,0xDA1D,0xDA1E,0xDA1F,
        0xDA20,0xDA21,0xDA24,0xDA23,
        0xDA2F,0xDA30,0xDA31,0xDA32,
        0xDA35,0xDA36,0xDA33,0xDA34,
        0x52,0x51,0xDA2C,0xDA0E,
        0xDA2E,0xDA0D,0xDA2D,0xD9F9,
        0xD9FA,0xDA25,0xDA26,0xDA27,
        0xDA28,0xDA29,0xDA2A,0xDA2B,
        0x0,0xDA39,0xE279,0x10CCB,0x14F83, // guessing
        0x0
    },
    {
        0xDB66,0xDB65,0xDB8B,0xDB8A,
        0xDB84,0x3F,0xDB79,0xDB7A,
        0xDB7B,0xDB7C,0xDB7D,0xDB7E,
        0xDB7F,0xDB80,0xDB81,0xDB82,
        0xDB83,0xDB89,0xDB9C,0xDB84,
        0xDB85,0xDB86,0xDB87,0xDB88,
        0xDB8D,0xDB8E,0xDBA1,0xDBA8,
        0xDBA4,0xDBA2,0xD97A,0xDBAA,
        0xDBA5,0xDBA9,0xDB98,0xDBA3,
        0xDBA6,0xDBA7,0xDBB0,0xDBAB,
        0xDB93,0xDBAC,0xDBAD,0xDB8F,
        0xDB92,0x5F,0xDB94,0xDB95,
        0xDB99,0xDB9A,0xD8D0,0xDBAE,
        0xDB90,0xDBAF,0x55,0x56,
        0xDBB3,0x9A87,0xDB91,0xDB9B,
        0xDBB4,0xDBB5,0xDBB6,0xDBB7,
        0xDBB8,0xDBB9,0xDBBA,0xDBBB,
        0xDBBC,0xDBC6,0xDBBD,0xDBBF,
        0xDBC0,0xDBC1,0xDBC2,0xDBC3,
        0xDBC4,0xDBC5,0xDBC8,0xDBC7,
        0xDBD3,0xDBD4,0xDBD5,0xDBD6,
        0xDBD9,0xDBDA,0xDBD7,0xDBD8,
        0x52,0x51,0xDBD0,0xDBB2,
        0xDBD2,0xDBB1,0xDBD1,0xDB9D,
        0xDB9E,0xDBC9,0xDBCA,0xDBCB,
        0xDBCC,0xDBCD,0xDBCE,0xDBCF,
        0x0,0xDBDD,0xE299,0x10DF7,0x150AF, // guessing
        0x0
    }
};

int GetBaseFileId(int languageId, int index) {
    return BaseFileIds[languageId][index];
}

int GetEntityListFileId(int zoneId) {
    if (zoneId < 256) {
        return 0x1A40 + zoneId;
    }
    if (zoneId < 1000) {
        return 0x150DB + zoneId;
    }
    if (zoneId < 2000) {
        // todo - revisit this
        return 0;
    }
    // todo - revisit this
    return 0x1055F + zoneId - 1000;
}

int GetEventMessageFileId(int languageId, int zoneId) {
    if (zoneId < 2000) {
        if (zoneId < 1000) {
            if (zoneId < 256) {
                int base = GetBaseFileId(languageId, 0x69);
                return base + zoneId;
            } else {
                int base = GetBaseFileId(languageId, 0x6c);
                return base - 256 + zoneId;
            }
        } else {
            int base = GetBaseFileId(languageId, 0x6a);
            return base - 1000 + zoneId;
        }
    } else {
        int base = GetBaseFileId(languageId, 0x6b);
        return base - 2000 + zoneId;
    }
}

int GetEventFileId(int zoneId) {
    if (zoneId < 2000) {
        if (zoneId < 1000) {
            if (zoneId < 0x100) {
                return zoneId + 0x16BC;
            } else {
                return zoneId + 0x14AFF;
            }
        } else {
            return zoneId + 0xDE31;
        }
    } else {
        return zoneId + 0x1004B;
    }
}
