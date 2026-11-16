#ifndef ST_VIS_H
#define ST_VIS_H

/*
 * ST.EXE visible-set lists (band 0x8001c100..0x8001cb88, WIP). All objects
 * here are unresolved .bss globals; only the members these functions touch
 * are named, as unkXXX until their meaning is proven.
 */
#include <kf/lib/types.h>
#include <sys/types.h>

/* 0x801b5920: a state block; func_8001c100 reads a halfword at +0 and a
 * byte at +0x1d, func_8001cab8 a byte at +9. */
typedef struct {
    s16 unk00;
    u8 unk02[7];
    u8 unk09;
    u8 unk0a[0x13];
    u8 unk1d;
    u8 unk1e[0xa];
} Dat801b5920;

/* 0x801b5948: the source id list func_80046270 fills and the visible id
 * list func_8001c100 builds from it; func_8001c800/func_8001c88c search the
 * visible list. */
typedef struct {
    u8 unk0000[0x44ca];
    u16 n_sources;       /* 0x44ca */
    u16 n_visible;       /* 0x44cc */
    u16 sources[0x80];   /* 0x44ce */
    u16 visible[0x80];   /* 0x45ce */
} Dat801b5948;

/* 0x8017bed0: 12-byte records indexed by visible id. */
typedef struct {
    u8 unk00[4];
    u8 unk04;            /* bit 1: flagged by func_8001c60c */
    u8 unk05[3];
    u8 unk08;
    u8 unk09;
    u8 unk0a;
    u8 unk0b;
} Dat8017bed0;

/* 0x8017d6d0: 64-byte records, a 512-bit mask per source id. */
typedef struct {
    u_long bits[16];
} Dat8017d6d0;

extern Dat801b5920 DAT_801b5920;
extern Dat801b5948 DAT_801b5948;
extern Dat8017bed0 DAT_8017bed0[];
extern Dat8017d6d0 DAT_8017d6d0[];

/* The scratchpad (0x1f800000) as the culling code uses it; the mesh
 * renderers' view of the same memory is MeshScratch (st/mesh.h). */
typedef struct {
    u8 unk000[0x128];
    long x;              /* 0x128 */
    long y;              /* 0x12c */
    long z;              /* 0x130 */
    u8 unk134[0xb4];
    u8 no_plane_test;    /* 0x1e8 */
    u8 unk1e9;
    short offset_x;      /* 0x1ea */
    short unk1ec;        /* 0x1ec */
    short offset_z;      /* 0x1ee */
    long plane[6];       /* 0x1f0 */
    long unk208;         /* 0x208 */
} VisScratch;

#define VIS_SCRATCH ((VisScratch *)0x1f800000)

/* 0x80198f28: the viewer block: position at +0x248, angles at +0x268. */
typedef struct {
    u8 unk000[0x248];
    long x;              /* 0x248 */
    long y;              /* 0x24c */
    long z;              /* 0x250 */
    u8 unk254[0x14];
    short unk268;        /* 0x268 */
    short unk26a;        /* 0x26a */
} Dat80198f28;

extern Dat80198f28 DAT_80198f28;

#endif
