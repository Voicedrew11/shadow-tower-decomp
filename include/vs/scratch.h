/*
 * VS.EXE's use of the scratchpad by the renderers: ST.EXE's ScratchPad
 * (st/scratch.h) with the screen-point words and the vertex buffer pointer
 * four bytes higher. Only proven fields are spelled.
 */
#ifndef VS_SCRATCH_H
#define VS_SCRATCH_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/mesh.h>

typedef struct {
    u_long *ot;          /* 0x00 */
    u_long far_z;        /* 0x04: faces at or beyond this OT depth are culled */
    u8 unk8[0xc - 0x8];
    u8 *model;           /* 0x0c */
    u_long *prim;        /* 0x10: next free primitive word */
    u_long *prim_end;    /* 0x14: end of the primitive buffer */
    u8 unk18[0x1c - 0x18];
    u8 *faces;           /* 0x1c: face cursor the renderers hand back */
    u8 unk20[0x24 - 0x20];
    MeshHeader *header;  /* 0x24 (ST 0x20) */
    u8 *normals;         /* 0x28 (ST 0x24) */
    u8 unk2C[0x30 - 0x2c];
    u_long *unk30;       /* 0x30: mesh_nclip_triangle's three screen-point words (ST 0x2c) */
    u_long *unk34;
    u_long *unk38;
    u8 unk3C[8];
    u8 *vertices;        /* 0x44: transformed vertices, by byte offset (ST 0x40) */
    u8 *unk48;           /* 0x48: a second vertex cursor (effect_draw_model_part) */
    u8 *local_vertices;  /* 0x4c: SVECTOR model-space vertices (ST 0x48) */
    u8 unk50[0x64 - 0x50];
    CVECTOR rgb;         /* 0x64: the renderers' NCCS input colour; .cd carries the GPU code (ST 0x60) */
    s32 unk68;           /* 0x68: the depth-cue near depth (fighter_draw_skeletal) */
    s32 unk6C;           /* 0x6c: and far depth */
    long opz;            /* 0x70: NCLIP result */
    long nclip;          /* 0x74: the renderers' NCLIP result */
    u8 unk78[0x82 - 0x78];
    u_short near_z;      /* 0x82: faces at or before this OT depth are culled */
    u8 unk84[0x88 - 0x84];
    int z_ot;            /* 0x88: the model renderers' first per-frame counter */
    int face_count;      /* 0x8c: faces submitted */
    int prim_count;      /* 0x90: primitives emitted */
    s32 unk94;           /* 0x94: their fourth per-frame counter */
    u_short clut_base;   /* 0x98 (effects_draw) */
    u8 unk9A[0xa4 - 0x9a];
    struct StAnimList *anims; /* 0xa4: the node list anim_list_compose composes */
    MATRIX matrix;       /* 0xa8: the model matrix effect_draw_model_part draws with */
    u8 unkC8[0x13c - 0xc8];
    VECTOR unk13C;       /* 0x13c: an effect's position (effects_draw) */
} VsScratchPad;

/* Layout check: the offset the code proves. */
typedef char VsScratchPadOpzAt0x70[offsetof(VsScratchPad, opz) == 0x70 ? 1 : -1];
typedef char VsScratchPadAnimsAt0xA4[offsetof(VsScratchPad, anims) == 0xa4 ? 1 : -1];

#define VS_SCRATCHPAD ((VsScratchPad *)0x1f800000)

#endif
