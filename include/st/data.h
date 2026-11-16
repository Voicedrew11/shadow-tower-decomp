/*
 * ST.EXE globals that no reconstructed source owns yet and that several
 * modules share. Each declaration spells only the fields the reconstructed
 * code proves, at the offsets its instructions use.
 */
#ifndef ST_DATA_H
#define ST_DATA_H

#include <kf/lib/types.h>
#include <st/memory.h>

/* PSX.EXE's next-program byte and the two mailbox bytes after it. */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800103f0)
#define MAILBOX_BYTE_1 ((u8 *)0x800103f1)
#define MAILBOX_BYTE_2 ((u8 *)0x800103f2)

/*
 * Words at the end of the game .rdata, before LIBCD's: 0x80013974 holds the
 * bss start, 0x80013978..0x80013980 the overlay area 0x801eb560, 0x80013988
 * its end. main stores the heap base at 0x8001398c. Their owner is unknown;
 * main addresses each with its own lui pair, as separate scalars.
 */
extern u8 *DAT_80013980;
extern u8 *DAT_80013988;
extern u8 *DAT_8001398c;

/* src/st/loop.c: the game loop runs while it is 0; 2 leaves for TITLE.EXE,
 * 3 for END.EXE. */
extern s32 DAT_800881a4;

/* Block heap arena (0x56400 bytes). */
extern StBlock DAT_8009c888[];

/*
 * The viewer block at 0x80198f28: the vsync seconds clock at +0x33/+0x34,
 * the FN. slots at +0x38, the position at +0x248 and two angles (pitch,
 * yaw) at +0x268.
 */
typedef struct {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
    u8 unk8[0x28 - 0x8];
    u16 unk28;
    u8 unk2A[0x33 - 0x2a];
    u8 unk33; /* incremented every vsync, wraps at 60 */
    s32 unk34; /* incremented when unk33 wraps */
    u16 unk38[21]; /* set from FN. ids 970..990 */
    u8 unk62[0x21c - 0x62];
    u8 unk21C;
    u8 unk21D[0x248 - 0x21d];
    long x;              /* 0x248 */
    long y;              /* 0x24c */
    long z;              /* 0x250 */
    u8 unk254[0x14];
    short unk268;        /* 0x268: pitch in func_80016554 */
    short unk26A;        /* 0x26a: yaw in func_80016554 */
} Viewer;

extern Viewer viewer;

/*
 * The state block at 0x801b5920: func_80014544 sets bytes 4..9 from FN.;
 * func_80021654 times the screen fade from the tick word at +0x18;
 * byte 9 is the current area the texture slots and table C entries are
 * compared with.
 */
typedef struct {
    s16 unk0;
    volatile u16 unk2; /* func_80021654 reads it zero-extended, then as a short */
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;
    u8 area;             /* 0x09 */
    u8 unkA[0x10 - 0xa];
    u8 unk10;            /* 0x10: 0xff when unset */
    u8 unk11[0x18 - 0x11];
    s32 unk18;           /* 0x18: tick func_80021654 times its fade from, -1 when idle */
    u8 unk1C;
    u8 unk1D;
    u8 unk1E;
    u8 unk1F[0x28 - 0x1f];
} AreaState;

extern AreaState area_state;

/* One count per id: a 7-bit count under a flag bit. */
typedef struct {
    u8 count : 7;
    u8 flag : 1;
} Count801c7590;
extern Count801c7590 DAT_801c7590[360];

extern u32 DAT_801cceb0[2][5];

typedef struct {
    u8 unk0[0x10];
    u8 unk10;
} Data801cd180;
extern Data801cd180 DAT_801cd180;

/*
 * One sound bank slot: the loaded bank's data (its state halfword at +4)
 * and the VAB id SsVabClose takes. func_800237a8 uses slots 8..23 as four
 * rows of four.
 */
typedef struct {
    s16 *block;
    s16 vab;
    s16 unk6;
} StVabSlot;
extern StVabSlot DAT_801dd1e8[24];

#endif
