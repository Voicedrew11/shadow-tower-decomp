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

typedef struct {
    u8 unk0[0x30];
    u_long *unk30;       /* 0x30: func_8002588c's three screen-point words (ST 0x2c) */
    u_long *unk34;
    u_long *unk38;
    u8 unk3C[8];
    u8 *vertices;        /* 0x44: transformed vertices, by byte offset (ST 0x40) */
    u8 unk48[0x70 - 0x48];
    long opz;            /* 0x70: NCLIP result */
} VsScratchPad;

/* Layout check: the offset the code proves. */
typedef char VsScratchPadOpzAt0x70[offsetof(VsScratchPad, opz) == 0x70 ? 1 : -1];

#define VS_SCRATCHPAD ((VsScratchPad *)0x1f800000)

#endif
