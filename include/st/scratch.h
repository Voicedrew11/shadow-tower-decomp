/*
 * The scratchpad (fixed data cache at 0x1f800000) as ST.EXE's game code
 * uses it. Every access is an absolute `lui 0x1f80` base with no
 * relocation, so the sources spell the fixed address. The mesh renderers,
 * the visible-set culling and the src/st/prim.c GTE helpers use disjoint words;
 * only those are named, the rest stay opaque.
 */
#ifndef ST_SCRATCH_H
#define ST_SCRATCH_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/gfx.h>
#include <st/mesh.h>

typedef struct {
    u_long *ot;          /* 0x00 */
    int far_z;           /* 0x04: faces at or beyond this OT depth are culled */
    u8 unk8[4];          /* 0x08 */
    u8 *model;           /* 0x0c */
    u_long *prim;        /* 0x10: next free primitive word */
    u_long *prim_end;    /* 0x14: end of the primitive buffer */
    u8 unk18[4];         /* 0x18 */
    u8 *faces;           /* 0x1c: face cursor the renderers hand back */
    MeshHeader *header;  /* 0x20 */
    u8 *normals;         /* 0x24 */
    u8 unk28[4];         /* 0x28 */
    u_long *unk2C;       /* 0x2c: func_80017cc4's three screen-point words */
    u_long *unk30;       /* 0x30 */
    u_long *unk34;       /* 0x34 */
    u8 unk38[8];         /* 0x38 */
    u8 *vertices;        /* 0x40: transformed vertices (MeshVertex), by byte offset */
    u8 unk44[4];         /* 0x44 */
    u8 *local_vertices;  /* 0x48: SVECTOR model-space vertices, same indices */
    u8 unk4C[0x14];      /* 0x4c */
    CVECTOR rgb;         /* 0x60: NCDS input colour; .cd carries the GPU code */
    s32 unk64;           /* 0x64: copied from the lighting's +0x68 */
    s32 unk68;           /* 0x68: copied from the lighting's +0x6a */
    u8 unk6C[4];         /* 0x6c */
    long opz;            /* 0x70: NCLIP result */
    long opz2;           /* 0x74: NCLIP of a quad's second triangle */
    u8 unk78[8];         /* 0x78 */
    int near_z;          /* 0x80: faces at or before this OT depth are culled */
    u8 unk84[8];         /* 0x84 */
    int face_count;      /* 0x8c: faces submitted */
    int prim_count;      /* 0x90: primitives emitted */
    u8 unk94[4];         /* 0x94 */
    u_short clut_base;   /* 0x98: added to subdivided faces' CLUT */
    u8 unk9A[0x2e];      /* 0x9a */
    u8 unkC8[16];        /* 0xc8: block tag per slot (low four bits) */
    SVECTOR unkD8;       /* 0xd8: rotation func_800241d0 hands func_80022678 */
    s32 unkE0;           /* 0xe0: a position func_80016554 tests (x, y, z) */
    s32 unkE4;           /* 0xe4 */
    s32 unkE8;           /* 0xe8 */
    u8 unkEC[4];         /* 0xec */
    u8 *unkF0;           /* 0xf0: cursor into unkF8 */
    u32 *unkF4;          /* 0xf4: &unk244 */
    u8 unkF8[0x30];      /* 0xf8: 128 flag bytes cleared per frame; overlaps x..z, used in another phase */
    long x;              /* 0x128: the position vis_measure_position measures */
    long y;              /* 0x12c */
    long z;              /* 0x130 */
    u8 unk134[0xb4];     /* 0x134 */
    u8 no_plane_test;    /* 0x1e8 */
    u8 unk1E9;           /* 0x1e9 */
    short offset_x;      /* 0x1ea */
    short unk1EC;        /* 0x1ec */
    short offset_z;      /* 0x1ee */
    long plane[6];       /* 0x1f0 */
    long unk208;         /* 0x208 */
    u8 unk20C[0x188];    /* 0x20c */
    StLight *unk394;     /* 0x394: lighting environment func_80017d28 blends toward */
} ScratchPad;

#define SCRATCHPAD ((ScratchPad *)0x1f800000)

#endif
