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
#include <st/pad_buttons.h>

typedef struct {
    u32 held;            /* 0x00: buttons held (inverted pad bits) */
    u32 prev_held;       /* 0x04: last frame's held */
    u32 pressed;         /* 0x08: newly pressed buttons, cleared by readers */
    u32 pressed_repeat;  /* 0x0c: pressed or repeating, cleared by readers */
    s32 repeat_timer;    /* 0x10: repeat delay: 30 frames, then every 5 */
} VsPad;

typedef struct {
    VsPad pads[2];       /* 0x00 */
    s32 show_frame_stats; /* 0x28: frame statistics shown when 1 (round_play toggles it) */
    s32 pan_hold_frames; /* 0x2c: frames button 1 has been held (round_play) */
    volatile s32 rcnt2_count; /* 0x30: frames, counted by the rcnt2_handler callback */
    u8 unk34[4];
    s32 rcnt2_event;     /* 0x38: main's root counter 2 event */
    volatile s32 vsync_count; /* 0x3c: frames, counted by the vsync_handler callback */
    u32 rate_vsync;      /* 0x40: a frame count the periodic sound triggers divide */
    s32 vsync_event;     /* 0x44: main's root counter 3 event */
} VsPadState;

typedef char VsPadSize[sizeof(VsPad) == 0x14 ? 1 : -1];
typedef char VsPadStateSize[sizeof(VsPadState) == 0x48 ? 1 : -1];

extern VsPadState vs_pad_state;

/* One controller's 34-byte Psy-Q receive buffer, filled by the pad
 * interrupt (volatile: the decode rereads it each frame). */
typedef struct {
    volatile u8 status;  /* 0x00: 0 when a controller answered */
    volatile u8 type;
    volatile s16 buttons; /* 0x02: active-low button bits */
    u8 unk4[34 - 4];
} VsPadBuffer;

extern VsPadBuffer pad_buffers[2];

#endif
