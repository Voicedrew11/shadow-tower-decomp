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
    u8 unk0[6];
    u16 unk6;            /* 0x06: func_8003f4ec copies it to viewer.unk134 */
    u16 unk8;            /* 0x08: and this one to viewer.unk138 */
    u8 unkA[2];
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
    u16 unk8;
    u8 unkA[0x12 - 0xa];
    u16 unk12;           /* 0x12..0x18: player_hit's defence terms */
    u16 unk14;
    u16 unk16;
    u16 unk18;
    u8 unk1A[0x1e - 0x1a];
    u16 unk1E;           /* 0x1e: divides player_take_damage's shake time */
    u16 unk20;           /* 0x20: scales player_hit's status-time limits */
    u8 unk22[0x26 - 0x22];
    u8 unk26;
    u8 unk27;
    u16 unk28;
    u8 unk2A[0x2c - 0x2a];
    u32 unk2C;           /* 0x2c: status flag bits (src/st/player.c) */
    u16 unk30;
    u8 unk32;
    u8 unk33; /* incremented every vsync, wraps at 60 */
    s32 unk34; /* incremented when unk33 wraps */
    u16 unk38[21]; /* set from FN. ids 970..990 */
    s16 unk62[14];       /* 0x62: func_8003cae4 sets 50 */
    s16 unk7E[14];       /* 0x7e: func_8003cae4 clears */
    u16 unk9A[14];       /* 0x9a: func_8003cae4 sets 50 */
    u16 unkB6;           /* 0xb6: player_hit's resistance terms */
    u8 unkB8[0xd8 - 0xb8];
    u16 unkD8[6];        /* 0xd8: player_hit's per-kind defence */
    u8 unkE4[0xe8 - 0xe4];
    ViewerModel *unkE8[4]; /* 0xe8: func_80021ac4's slot positions and rotations */
    void *unkF8[4];      /* 0xf8: its slot model blocks (ModelFile, st/mesh.h) */
    u32 unk108[4];       /* 0x108: its slot animation states */
    void *unk118[2];     /* 0x118: func_8003ee38's DAT_8018d298 record per slot */
    u8 *unk120[2];       /* 0x120: its load buffers (DAT_80190f28) */
    s32 unk128[2];
    s16 unk130[4];       /* 0x130: -1: slot empty */
    s16 unk138[2];       /* 0x138 */
    s16 unk13C[2];       /* 0x13c: -1 when idle */
    u8 unk140[0x148 - 0x140];
    u8 unk148[4];        /* 0x148: func_8003f12c's item model per slot (255: none) */
    u8 unk14C[2];        /* 0x14c: func_8003ee38's item per slot (255: none) */
    u8 unk14E;
    u8 unk14F;
    u8 unk150[4];        /* 0x150 */
    u8 unk154;
    u8 unk155[2];        /* 0x155: func_8003f910's kind per slot (255: none) */
    u8 unk157[2];
    u8 unk159[0x15e - 0x159];
    u8 unk15E;
    u8 unk15F;
    SVECTOR unk160[2];   /* 0x160: func_8003f704's offsets per slot */
    u8 unk170[0x178 - 0x170];
    s16 unk178;
    s16 unk17A;
    s16 unk17C;
    u16 unk17E;
    u16 unk180;
    u16 unk182;
    u16 unk184;
    u8 unk186;
    u8 unk187;
    u8 unk188;
    u8 unk189;
    u16 unk18A;
    u16 unk18C;
    u8 unk18E[2];
    s16 unk190[13][2];   /* 0x190: status timers: level, peak (func_8003c17c) */
    u8 unk1C4;
    u8 unk1C5;
    u8 unk1C6;
    s8 unk1C7;           /* 0x1c7: lighting_update picks lighting rows by its sign */
    u8 unk1C8;
    u8 unk1C9;
    u8 unk1CA;
    u8 unk1CB;
    s16 unk1CC;
    s16 unk1CE;
    s16 unk1D0;
    u8 unk1D2[2];
    u8 unk1D4[4];
    u8 unk1D8[0x1e4 - 0x1d8];
    u8 unk1E4;
    u8 unk1E5[0x1ec - 0x1e5];
    s32 unk1EC[5];
    u8 unk200[0x20c - 0x200];
    s16 unk20C[5];
    u8 unk216[2];
    u8 unk218;           /* 0x218 */
    u8 unk219;
    u8 unk21A;
    u8 unk21B;
    u8 unk21C;
    u8 unk21D;
    u8 unk21E;
    u8 unk21F[0x22c - 0x21f];
    u8 unk22C;
    u8 unk22D;
    u8 unk22E;
    u8 unk22F;
    s16 unk230;
    s16 unk232;
    s16 unk234;
    s16 unk236;
    u8 unk238;
    u8 unk239;
    u8 unk23A[2];
    u32 unk23C;
    u8 unk240[4];
    s32 unk244;
    long x;              /* 0x248 */
    long y;              /* 0x24c */
    long z;              /* 0x250 */
    u8 unk254[0x14];
    short unk268;        /* 0x268: pitch in func_80016554 */
    short unk26A;        /* 0x26a: yaw in func_80016554 */
    short unk26C;        /* 0x26c: roll; model_draw_list turns kind-2 parts by all three */
    short unk26E;
    SVECTOR unk270;      /* 0x270 */
    SVECTOR unk278;      /* 0x278: func_8003c054 copies it to 0x268 */
    s16 unk280;
    s16 unk282;
    s16 unk284;
    s16 unk286;
    u16 unk288;          /* 0x288, 0x28c: player_update_shake adds its offsets */
    u16 unk28A;
    u16 unk28C;
    u8 unk28E[2];
    s16 unk290;
    s16 unk292;
    s16 unk294;
    u8 unk296[2];
    MATRIX unk298;       /* 0x298: func_8003f704's rotation */
    u8 unk2B8[0x2c8 - 0x2b8];
    s16 unk2C8;
    s16 unk2CA;
    s16 unk2CC;
    short unk2CE;        /* 0x2ce: added to y by world_transform_parts */
    short unk2D0;
    short unk2D2;        /* 0x2d2: added to y by world_transform_parts */
    s16 unk2D4;
    s16 unk2D6;          /* 0x2d6: red fade level, falls by unk2D8 */
    s16 unk2D8;
    s16 unk2DA;          /* 0x2da: yellow fade level, falls by unk2DC */
    s16 unk2DC;
    short unk2DE;        /* 0x2de..0x2e2: added to func_80021ac4's slot rotations */
    short unk2E0;
    short unk2E2;
    s32 unk2E4;
    s32 unk2E8;
    s32 unk2EC;
    s32 unk2F0;
    s32 unk2F4;          /* 0x2f4: player_update_shake's countdown, -1 when idle */
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
    u8 unk0[4];
    u16 unk4;            /* 0x04: request bits func_8003f4ec clears */
    u8 unk6[0x10 - 0x6];
    u8 unk10;
    u8 unk11[0x32 - 0x11];
    u16 unk32;           /* 0x32: slot 0's request bit */
    u16 unk34;           /* 0x34: the other slots' request bit */
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

/* The CD file buffer func_80016030 loads TIMs and models into (WIP extent:
 * up to DAT_8009ac88). */
extern u_long DAT_80088c88[];

/* 176 records of 24 bytes; func_8003ee38 keeps one per item slot. */
typedef struct {
    u8 unk0[0x18];
} Data8018d298;
extern Data8018d298 DAT_8018d298[176];

/* 128 records of 88 bytes: unk0 is 255 when free, its bit 5 the item slot;
 * func_8003ee38 frees the ones whose unk1 is below 44 or in 170..179. */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2[0x56];
} Data8018e318;
extern Data8018e318 DAT_8018e318[128];

/* Two 0x4000-byte load buffers (viewer.unk120). */
extern u8 DAT_80190f28[2][0x4000];
/* Four 0x7000-byte model blocks (viewer.unkF8). */
extern u8 DAT_80199220[4][0x7000];
/* The 64 equipment model placements viewer.unkE8 points into. */
extern ViewerModel DAT_801b5220[64];

/* Settings bytes func_8003ea20 initializes (WIP extent). */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;
} Data801cd120;
extern Data801cd120 DAT_801cd120;

#endif
