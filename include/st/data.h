/*
 * ST.EXE globals that no reconstructed source owns yet. Names stay
 * address-derived; each declaration spells only the fields band A's code
 * proves, at the offsets its instructions use.
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
    s32 unk248;
    s32 unk24C;
    s32 unk250;
} Data80198f28;
extern Data80198f28 DAT_80198f28;

typedef struct {
    s16 unk0; /* band D: read signed (lh) */
    volatile u16 unk2; /* band D: func_80021654 reads it zero-extended, then as a short */
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;
    u8 unk9;
    u8 unkA[0x10 - 0xa]; /* band D */
    u8 unk10; /* band D: 0xff when unset */
    u8 unk11[0x18 - 0x11]; /* band D */
    s32 unk18; /* band D: tick count func_80021654 measures from, -1 when idle */
    u8 unk1C[0x1e - 0x1c]; /* band D */
    u8 unk1E; /* band D */
    u8 unk1F[0x28 - 0x1f]; /* band D: WIP extent up to DAT_801b5948 */
} Data801b5920;
extern Data801b5920 DAT_801b5920;

/* One count per id: a 7-bit count under a flag bit. */
typedef struct {
    u8 count : 7;
    u8 flag : 1;
} Count801c7590;
extern Count801c7590 DAT_801c7590[360];

typedef struct {
    u8 unk0;
    u8 unk1[43];
} Item801c7814;

typedef struct {
    u8 unk0[0x70];
    u32 unk70[10]; /* one bit per id */
    u8 unk98[0x11c - 0x98];
    Item801c7814 unk11C[314];
} Data801c76f8;
extern Data801c76f8 DAT_801c76f8;

extern u32 DAT_801cceb0[2][5];

typedef struct {
    u8 unk0[0x10];
    u8 unk10;
} Data801cd180;
extern Data801cd180 DAT_801cd180;

/*
 * band D: one sound bank slot, the loaded bank's data (its state halfword
 * at +4) and the VAB id SsVabClose takes. func_800237a8 uses slots 8..23 as
 * four rows of four.
 */
typedef struct {
    s16 *block;
    s16 vab;
    s16 unk6;
} StVabSlot;
extern StVabSlot DAT_801dd1e8[24];

#endif
