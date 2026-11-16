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
#include <st/mesh.h>

typedef struct {
    u8 unk0[0xc];
    u8 *model;           /* 0x0c */
    u8 unk10[0x24 - 0x10];
    MeshHeader *header;  /* 0x24 (ST 0x20) */
    u8 unk28[0x30 - 0x28];
    u_long *unk30;       /* 0x30: func_8002588c's three screen-point words (ST 0x2c) */
    u_long *unk34;
    u_long *unk38;
    u8 unk3C[8];
    u8 *vertices;        /* 0x44: transformed vertices, by byte offset (ST 0x40) */
    u8 unk48[0x4c - 0x48];
    u8 *local_vertices;  /* 0x4c: SVECTOR model-space vertices (ST 0x48) */
    u8 unk50[0x70 - 0x50];
    long opz;            /* 0x70: NCLIP result */
    u8 unk74[0xa4 - 0x74];
    struct StAnimList *anims; /* 0xa4: the node list anim_list_compose composes */
} VsScratchPad;

/* Layout check: the offset the code proves. */
typedef char VsScratchPadOpzAt0x70[offsetof(VsScratchPad, opz) == 0x70 ? 1 : -1];
typedef char VsScratchPadAnimsAt0xA4[offsetof(VsScratchPad, anims) == 0xa4 ? 1 : -1];

#define VS_SCRATCHPAD ((VsScratchPad *)0x1f800000)

#endif
