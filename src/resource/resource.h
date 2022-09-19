#ifndef RESOURCE__RESOURCE_H
#define RESOURCE__RESOURCE_H

#include <inttypes.h>
#include <stdbool.h>

typedef enum CHUNK_TYPE {
    CHUNK_TYPE_Terminate,
    CHUNK_TYPE_Rmp,
    CHUNK_TYPE_Rmw,
    CHUNK_TYPE_Directory,
    CHUNK_TYPE_Bin,
    CHUNK_TYPE_Generator,
    CHUNK_TYPE_Camera,
    CHUNK_TYPE_Scheduler,
    CHUNK_TYPE_Mtx,
    CHUNK_TYPE_Tim,
    CHUNK_TYPE_TexInfo,
    CHUNK_TYPE_Vum,
    CHUNK_TYPE_Oml,
    CHUNK_TYPE_FileInfo,
    CHUNK_TYPE_Anm,
    CHUNK_TYPE_Rsd,
    CHUNK_TYPE_UnKnown,
    CHUNK_TYPE_Osm,
    CHUNK_TYPE_Skd,
    CHUNK_TYPE_Mtd,
    CHUNK_TYPE_Mld,
    CHUNK_TYPE_Mlt,
    CHUNK_TYPE_Mws,
    CHUNK_TYPE_Mod,
    CHUNK_TYPE_Tim2,
    CHUNK_TYPE_KeyFrame,
    CHUNK_TYPE_Bmp,
    CHUNK_TYPE_Bmp2,
    CHUNK_TYPE_Mzb,
    CHUNK_TYPE_Mmd,
    CHUNK_TYPE_Mep,
    CHUNK_TYPE_D3m,
    CHUNK_TYPE_D3s,
    CHUNK_TYPE_D3a,
    CHUNK_TYPE_DistProg,
    CHUNK_TYPE_VuLineProg,
    CHUNK_TYPE_RingProg,
    CHUNK_TYPE_D3b,
    CHUNK_TYPE_Asn,
    CHUNK_TYPE_Mot,
    CHUNK_TYPE_Skl,
    CHUNK_TYPE_Sk2,
    CHUNK_TYPE_Os2,
    CHUNK_TYPE_Mo2,
    CHUNK_TYPE_Ps2,
    CHUNK_TYPE_Wsd,
    CHUNK_TYPE_Mmb,
    CHUNK_TYPE_Weather,
    CHUNK_TYPE_Meb,
    CHUNK_TYPE_Msb,
    CHUNK_TYPE_Med,
    CHUNK_TYPE_Msh,
    CHUNK_TYPE_Ysh,
    CHUNK_TYPE_Mpb,
    CHUNK_TYPE_Rid,
    CHUNK_TYPE_Wd,
    CHUNK_TYPE_Bgm,
    CHUNK_TYPE_Lfd,
    CHUNK_TYPE_Lfe,
    CHUNK_TYPE_Esh,
    CHUNK_TYPE_Sch,
    CHUNK_TYPE_Sep,
    CHUNK_TYPE_Vtx,
    CHUNK_TYPE_Lwo,
    CHUNK_TYPE_Rme,
    CHUNK_TYPE_Elt,
    CHUNK_TYPE_Rab,
    CHUNK_TYPE_Mtt,
    CHUNK_TYPE_Mtb,
    CHUNK_TYPE_Cib,
    CHUNK_TYPE_Tlt,
    CHUNK_TYPE_PointLightProg,
    CHUNK_TYPE_Mgd,
    CHUNK_TYPE_Mgb,
    CHUNK_TYPE_Sph,
    CHUNK_TYPE_Bmd,
    CHUNK_TYPE_Qif,
    CHUNK_TYPE_Qdt,
    CHUNK_TYPE_Mif,
    CHUNK_TYPE_Mdt,
    CHUNK_TYPE_Sif,
    CHUNK_TYPE_Sdt,
    CHUNK_TYPE_Acd,
    CHUNK_TYPE_Acb,
    CHUNK_TYPE_Afb,
    CHUNK_TYPE_Aft,
    CHUNK_TYPE_Wwd,
    CHUNK_TYPE_NullProg,
    CHUNK_TYPE_Spw,
    CHUNK_TYPE_Fud,
    CHUNK_TYPE_DisgregaterProg,
    CHUNK_TYPE_Smt,
    CHUNK_TYPE_DamValueProg,
    CHUNK_TYPE_Bp,
    CHUNK_TYPE_Unknown1,
    CHUNK_TYPE_Unknown2,
    CHUNK_TYPE_Max,
} CHUNK_TYPE;

typedef struct chunk_t {
    CHUNK_TYPE type;
    uint32_t length;
    uint32_t flags;

    const uint8_t* buf;

    char FourCC[5];

} chunk_t;

const char* GetChunkTypeName(CHUNK_TYPE type);

int LoadChunkedResource(const uint8_t* buf, uint32_t bufLen, int (*callback)(const chunk_t*, void*), void* userData);

#endif
