/*
 * VS.EXE's controller state at 0x801603c0 (bss): one 20-byte record per
 * player, decoded each frame from the Psy-Q pad buffers at 0x801961e0
 * (bss), plus two frame counters. Only the words the code proves are
 * spelled.
 */
#ifndef VS_PAD_H
#define VS_PAD_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>

typedef struct {
    u32 unk0;            /* 0x00: buttons held (inverted pad bits) */
    u32 unk4;            /* 0x04: last frame's unk0 */
    u32 unk8;            /* 0x08: newly pressed buttons, cleared by readers */
    u32 unkC;            /* 0x0c: pressed or repeating, cleared by readers */
    s32 unk10;           /* 0x10: repeat delay: 30 frames, then every 5 */
} VsPad;

typedef struct {
    VsPad pads[2];       /* 0x00 */
    u8 unk28[8];
    volatile s32 unk30;  /* 0x30: frames, counted by the func_80013848 callback */
    u8 unk34[8];
    volatile s32 unk3C;  /* 0x3c: frames, counted by the func_80013868 callback */
} VsPadState;

typedef char VsPadSize[sizeof(VsPad) == 0x14 ? 1 : -1];
typedef char VsPadStateSize[sizeof(VsPadState) == 0x40 ? 1 : -1];

extern VsPadState DAT_801603c0;

/* One controller's 34-byte Psy-Q receive buffer, filled by the pad
 * interrupt (volatile: the decode rereads it each frame). */
typedef struct {
    volatile u8 status;  /* 0x00: 0 when a controller answered */
    volatile u8 type;
    volatile s16 buttons; /* 0x02: active-low button bits */
    u8 unk4[34 - 4];
} VsPadBuffer;

extern VsPadBuffer DAT_801961e0[2];

#endif
