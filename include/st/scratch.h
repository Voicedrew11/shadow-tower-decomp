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
    u_long *unk38;       /* 0x38: world_project_parts' output start for the current part */
    u8 *part_vertices;   /* 0x3c: the world renderers' model-space vertices (SVECTOR) */
    u8 *vertices;        /* 0x40: transformed vertices (MeshVertex), by byte offset */
    CVECTOR part_rgb;    /* 0x44: the world renderers' NCCS input colour; .cd carries the GPU code */
    u8 *local_vertices;  /* 0x48: SVECTOR model-space vertices, same indices */
    u8 unk4C[0xc];       /* 0x4c */
    long flag;           /* 0x58: GTE FLAG after world_transform_parts' RTV0TR */
    u8 unk5C[4];         /* 0x5c */
    CVECTOR rgb;         /* 0x60: NCDS input colour; .cd carries the GPU code */
    s32 unk64;           /* 0x64: copied from the lighting's +0x68 */
    s32 unk68;           /* 0x68: copied from the lighting's +0x6a */
    u8 unk6C[4];         /* 0x6c */
    long opz;            /* 0x70: NCLIP result */
    long opz2;           /* 0x74: NCLIP of a quad's second triangle */
    u8 unk78[8];         /* 0x78 */
    int near_z;          /* 0x80: faces at or before this OT depth are culled */
    int z_max;           /* 0x84: the world renderers' farthest vertex depth of a face */
    int z_ot;            /* 0x88: its OT depth */
    int face_count;      /* 0x8c: faces submitted */
    int prim_count;      /* 0x90: primitives emitted */
    u8 unk94[4];         /* 0x94 */
    u_short clut_base;   /* 0x98: added to subdivided faces' CLUT */
    u8 unk9A[0xe];       /* 0x9a */
    VECTOR part_pos;     /* 0xa8: a record's camera-space position (RTV0TR result) */
    u8 unkB8[0x10];      /* 0xb8 */
    u8 unkC8[16];        /* 0xc8: block tag per slot (low four bits) */
    SVECTOR unkD8;       /* 0xd8: rotation func_800241d0 hands func_80022678 */
    s32 unkE0;           /* 0xe0: a position func_80016554 tests (x, y, z) */
    s32 unkE4;           /* 0xe4 */
    s32 unkE8;           /* 0xe8 */
    u8 unkEC[4];         /* 0xec */
    u8 *unkF0;           /* 0xf0: cursor into unkF8 */
    u32 *unkF4;          /* 0xf4: &unk244 */
    u8 unkF8[0x28];      /* 0xf8: slot flag bytes cleared per frame (up to 332, over the
                          * following fields: the object passes run in another phase) */
    SVECTOR part_offset; /* 0x120: a record's viewer-relative position, scaled to 16 bits */
    long x;              /* 0x128: the position vis_measure_position measures */
    long y;              /* 0x12c */
    long z;              /* 0x130 */
    u8 unk134[0x50];     /* 0x134 */
    s32 faces_left;      /* 0x184: the Gouraud world renderers' face countdown */
    u8 unk188[0x28];     /* 0x188 */
    s32 part_vertex_count; /* 0x1b0..0x1e0: per-frame counters world_draw saves back to world_state; this one counts vertices projected */
    s32 part_faces;      /* 0x1b4: faces the world renderers submitted */
    s32 unk1B8;          /* 0x1b8 */
    s32 part_prims;      /* 0x1bc: primitives they emitted undivided */
    s32 part_div_prims;  /* 0x1c0: primitives RCpoly* emitted (4^ndiv each) */
    s32 unk1C4;          /* 0x1c4 */
    s32 unk1C8;          /* 0x1c8 */
    s32 unk1CC;          /* 0x1cc */
    s32 unk1D0;          /* 0x1d0 */
    s32 unk1D4;          /* 0x1d4 */
    s32 unk1D8;          /* 0x1d8 */
    s32 unk1DC;          /* 0x1dc */
    s32 unk1E0;          /* 0x1e0 */
    s32 light_index;     /* 0x1e4: the current part's light set */
    u8 no_plane_test;    /* 0x1e8 */
    u8 unk1E9;           /* 0x1e9 */
    short offset_x;      /* 0x1ea */
    short unk1EC;        /* 0x1ec */
    short offset_z;      /* 0x1ee */
    long plane[6];       /* 0x1f0 */
    long unk208;         /* 0x208 */
    s32 part_shift;      /* 0x20c: halvings part_offset needed to fit 16 bits */
    u8 unk210[0x34];     /* 0x210 */
    u32 unk244[0x26];    /* 0x244: object passes clear 4..75 words (another phase) */
    u8 unk2DC[0xb8];     /* 0x2dc: the menu screens' fields in their phase (MenuScratch, st/menu.h) */
    StLight *unk394;     /* 0x394: lighting environment func_80017d28 blends toward */
    u8 unk398[0x2c];     /* 0x398 */
    StLight *light;      /* 0x3c4: the world renderers' lighting environment */
} ScratchPad;

#define SCRATCHPAD ((ScratchPad *)0x1f800000)

#endif
