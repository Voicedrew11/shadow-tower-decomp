/*
 * ST.EXE globals that no reconstructed source owns yet and that several
 * modules share. Each declaration spells only the fields the reconstructed
 * code proves, at the offsets its instructions use.
 */
#ifndef ST_DATA_H
#define ST_DATA_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
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
/* A viewer model slot's position (+0xc) and rotation (+0x14). */
typedef struct {
    u8 unk0[0xc];
    short unkC;
    short unkE;
    short unk10;
    short unk12;
    SVECTOR unk14;
} ViewerModel;

typedef struct {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
    u16 unk8;            /* 0x8: shown by the status menu */
    u8 unkA[4];
    u16 unkE;            /* 0xe: shown by the stats menu */
    u16 unk10;           /* 0x10 */
    u8 unk12[0x15];
    u8 unk27;            /* 0x27: shown by the map menu */
    u16 unk28;
    u8 unk2A[2];
    u32 unk2C;           /* 0x2c: flag bits the HUD shows (func_80026508) */
    u16 unk30;           /* 0x30 */
    u8 unk32;
    u8 unk33; /* incremented every vsync, wraps at 60 */
    s32 unk34; /* incremented when unk33 wraps */
    u16 unk38[21]; /* set from FN. ids 970..990 */
    s16 unk62[14];       /* 0x62: figures func_8002e94c lists */
    u8 unk7E[0xba - 0x7e];
    u16 unkBA;           /* 0xba: shown by the stats menu */
    u16 unkBC;           /* 0xbc */
    u16 unkBE;           /* 0xbe */
    u8 unkC0[0xe8 - 0xc0];
    ViewerModel *unkE8[4]; /* 0xe8: func_80021ac4's slot positions and rotations */
    void *unkF8[4];      /* 0xf8: its slot model blocks (ModelFile, st/mesh.h) */
    u32 unk108[4];       /* 0x108: its slot animation states */
    u8 unk118[0x130 - 0x118];
    s16 unk130[4];       /* 0x130: -1: slot empty */
    u8 unk138[0x150 - 0x138];
    u8 unk150[4];        /* 0x150 */
    u8 unk154[0x15e - 0x154];
    s8 unk15E;           /* 0x15e */
    u8 unk15F[0x17e - 0x15f];
    u16 unk17E;          /* 0x17e: HUD bar 0 value */
    u8 unk180[2];
    u16 unk182;          /* 0x182: HUD bar 1 value */
    u8 unk184[2];
    u8 unk186;           /* 0x186: 2 when the HUD numbers need redrawing */
    u8 unk187[3];
    u16 unk18A;          /* 0x18a: unk2 as last shown */
    u16 unk18C;          /* 0x18c: unk6 as last shown */
    u8 unk18E[0x1a4 - 0x18e];
    s32 unk1A4[8];       /* 0x1a4: nonzero shows a HUD icon */
    u8 unk1C4[0x1ec - 0x1c4];
    s32 unk1EC[5];       /* 0x1ec: frames left to show unk20C[i] */
    u8 unk200[0xc];
    s16 unk20C[5];       /* 0x20c: numbers func_80026a88 queues */
    u8 unk216[2];
    u8 unk218;           /* 0x218 */
    u8 unk219[3];
    u8 unk21C;
    u8 unk21D[0x248 - 0x21d];
    long x;              /* 0x248 */
    long y;              /* 0x24c */
    long z;              /* 0x250 */
    u8 unk254[0x14];
    short unk268;        /* 0x268: pitch in func_80016554 */
    short unk26A;        /* 0x26a: yaw in func_80016554 */
    short unk26C;        /* 0x26c: roll; model_draw_list turns kind-2 parts by all three */
    u8 unk26E[0x2ce - 0x26e];
    short unk2CE;        /* 0x2ce: added to y by world_transform_parts */
    u8 unk2D0[2];
    short unk2D2;        /* 0x2d2: added to y by world_transform_parts */
    u8 unk2D4[0x2de - 0x2d4];
    short unk2DE;        /* 0x2de..0x2e2: added to func_80021ac4's slot rotations */
    short unk2E0;
    short unk2E2;
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
    u8 unk1F;            /* 0x1f: nonzero skips the block refresh (band E object passes) */
    u8 unk20[8];
} AreaState;

extern AreaState area_state;

/* One count per id: a 7-bit count under a flag bit. */
typedef struct {
    u8 count : 7;
    u8 flag : 1;
} Count801c7590;

/* The counts of the 314 table C ids, then a halfword id table. */
typedef struct {
    Count801c7590 counts[314];
    u16 unk13A[23];      /* 0x13a: ids the item list screens show */
} Data801c7590;
extern Data801c7590 DAT_801c7590;

extern u32 DAT_801cceb0[2][5];

typedef struct {
    u8 unk0[4];
    u16 unk4;            /* 0x04: page bits func_8002fc4c consumes */
    u8 unk6[0xa];
    u8 unk10;
} Data801cd180;
extern Data801cd180 DAT_801cd180;

/*
 * One sound bank slot: the loaded bank's data (its state halfword at +4)
 * and the VAB id SsVabClose takes. func_800237a8 uses slots 8..23 as four
 * rows of four; func_80023c6c addresses slots 8.. and func_80024004 slots
 * 24.. (src/st/slots.c). WIP extent.
 */
typedef struct {
    s16 *block;
    s16 vab;
    s16 unk6;
} StVabSlot;
extern StVabSlot DAT_801dd1e8[32];

#endif
