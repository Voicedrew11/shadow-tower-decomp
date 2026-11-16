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
#include <LIBCD.H>
#include <st/memory.h>
#include <st/gfx.h>

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
    u16 unk8;            /* 0x8: shown by the status menu */
    u8 unkA[4];
    u16 unkE;            /* 0xe: shown by the stats menu */
    u16 unk10;           /* 0x10 */
    u16 unk12;           /* 0x12..0x18: player_hit's defence terms */
    u16 unk14;
    u16 unk16;
    u16 unk18;
    u8 unk1A[0x1e - 0x1a];
    u16 unk1E;           /* 0x1e: divides player_take_damage's shake time */
    u16 unk20;           /* 0x20: scales player_hit's status-time limits */
    u8 unk22[0x26 - 0x22];
    u8 unk26;
    u8 unk27;            /* 0x27: shown by the map menu */
    u16 unk28;
    u8 unk2A[0x2c - 0x2a];
    u32 unk2C;           /* 0x2c: flag bits the HUD shows (func_80026508); status flags (src/st/player.c) */
    u16 unk30;           /* 0x30 */
    u8 unk32;
    u8 unk33; /* incremented every vsync, wraps at 60 */
    s32 unk34; /* incremented when unk33 wraps */
    u16 unk38[21]; /* set from FN. ids 970..990 */
    /* Three parallel 14-entry figure arrays (func_8003cae4 sets unk62 and
     * unk9A to 50 and clears unk7E). func_8002e94c lists unk62; effect_play
     * reads six u16s from 0x78 (unk62[11]..unk7E[2]) by its level. */
    s16 unk62[14];       /* 0x62 */
    s16 unk7E[14];       /* 0x7e */
    u16 unk9A[14];       /* 0x9a: figures func_8004db0c adds bonuses to */
    u16 unkB6;           /* 0xb6: player_hit's resistance terms */
    u16 unkB8;           /* 0xb8 */
    u16 unkBA[3];        /* 0xba: shown by the stats menu; effect_play reads them as a row */
    u8 unkC0[0xd8 - 0xc0];
    u16 unkD8[6];        /* 0xd8: player_hit's per-kind defence */
    u8 unkE4[0xe8 - 0xe4];
    ViewerModel *unkE8[4]; /* 0xe8: func_80021ac4's slot positions and rotations */
    void *unkF8[4];      /* 0xf8: its slot model blocks (ModelFile, st/mesh.h) */
    void *unk108[4];     /* 0x108: its slot animation state: a StAnimSlot (anim_prepare) or
                          * a node set (anim_prepare_any), by the model file's halfword +6 */
    void *unk118[2];     /* 0x118: func_8003ee38's effects.defs record per slot */
    u8 *unk120[2];       /* 0x120: its load buffers (DAT_80190f28) */
    s32 unk128[2];
    s16 unk130[4];       /* 0x130: -1: slot empty */
    s16 unk138[2];       /* 0x138 */
    s16 unk13C[2];       /* 0x13c: per hand, -1 when idle */
    u8 unk140[0x148 - 0x140];
    u8 unk148[4];        /* 0x148: func_8003f12c's item model per slot (255: none); per hand: held item kind */
    u8 unk14C[2];        /* 0x14c: func_8003ee38's item per slot (255: none); per hand: magic id */
    u8 unk14E;
    u8 unk14F;
    u8 unk150[4];        /* 0x150 */
    u8 unk154;
    u8 unk155[2];        /* 0x155: func_8003f910's kind per slot (255: none): the magic cast */
    u8 unk157[2];
    u8 unk159[2];        /* 0x159: per hand: reload for unk15B */
    u8 unk15B[2];        /* 0x15b: per hand: countdown to the next func_8003f910 */
    u8 unk15D;
    s8 unk15E;           /* 0x15e: HUD frame offset (func_80026afc) */
    u8 unk15F;
    SVECTOR unk160[2];   /* 0x160: func_8003f704's offsets per slot */
    u8 unk170[0x178 - 0x170];
    s16 unk178;
    s16 unk17A;
    s16 unk17C;
    u16 unk17E;          /* 0x17e: HUD bar 0 value */
    u16 unk180;
    u16 unk182;          /* 0x182: HUD bar 1 value */
    u16 unk184;
    u8 unk186;           /* 0x186: 2 when the HUD numbers need redrawing */
    u8 unk187;
    u8 unk188;
    u8 unk189;
    u16 unk18A;          /* 0x18a: unk2 as last shown */
    u16 unk18C;          /* 0x18c: unk6 as last shown */
    u8 unk18E[2];
    s16 unk190[13][2];   /* 0x190: status timers: level, peak (func_8003c17c);
                          * the HUD shows an icon while timer 5.. as a word is nonzero */
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
    u8 unk1D4[4];        /* 0x1d4: per slot/hand: sound id */
    CdlLOC unk1D8;       /* 0x1d8: XA stream start position (src/st/xa.c) */
    CdlLOC unk1DC;       /* 0x1dc: its current position */
    s32 unk1E0;          /* 0x1e0: its length in sectors */
    u8 unk1E4;
    u8 unk1E5[3];
    u32 unk1E8;          /* 0x1e8: cd_state.unk4 when the stream started */
    s32 unk1EC[5];       /* 0x1ec: frames left to show unk20C[i] */
    u8 unk200[0x20c - 0x200];
    s16 unk20C[5];       /* 0x20c: numbers func_80026a88 queues */
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
    u8 unk254[0xc];
    SVECTOR unk260;      /* 0x260: motion (func_8004cc70 leads targets by it) */
    short unk268;        /* 0x268: pitch in func_80016554 */
    short unk26A;        /* 0x26a: yaw in func_80016554 */
    short unk26C;        /* 0x26c: roll; model_draw_list turns kind-2 parts by all three */
    short unk26E;
    SVECTOR unk270;      /* 0x270 */
    SVECTOR unk278;      /* 0x278: func_8003c054 copies it to 0x268; view pitch (vx) and yaw (vy) func_80041d4c turns
                          * (band L reads the yaw, unk278.vy) */
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
    s16 unk2CA;          /* 0x2ca: yaw speed */
    s16 unk2CC;          /* 0x2cc: pitch speed */
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
    s32 unk2E8;          /* 0x2e8: top yaw speed */
    s32 unk2EC;
    s32 unk2F0;
    s32 unk2F4;          /* 0x2f4: player_update_shake's countdown, -1 when idle */
} Viewer;

extern Viewer viewer;

/* What area_state.unkC points at: a function table (WIP extent). */
typedef struct {
    void (*unk0)();      /* 0x00: called by func_80055374 with (position, facing) */
    void (*unk4)(void);  /* 0x04: per frame (func_8005735c) */
    u8 unk08[0x18 - 0x08];
    void (*unk18)(void); /* 0x18: an item count rose (func_80055300) */
    u8 unk1C[0x30 - 0x1c];
    void (*unk30)();     /* 0x30: item use hook */
    u8 unk34[0x40 - 0x34];
    void (*unk40)(void); /* 0x40 */
} AreaProcs;

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
    u8 unkA[2];
    AreaProcs *unkC;     /* 0x0c */
    u8 unk10;            /* 0x10: 0xff when unset */
    u8 unk11;
    u8 unk12;            /* 0x12 */
    u8 unk13;            /* 0x13 */
    u8 unk14;            /* 0x14 */
    u8 unk15[3];
    s32 unk18;           /* 0x18: tick func_80021654 times its fade from, -1 when idle */
    u8 unk1C;
    u8 unk1D;
    u8 unk1E;
    u8 unk1F;            /* 0x1f: nonzero skips the block refresh (band E object passes) */
    u8 unk20;            /* 0x20 */
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u16 unk26;           /* 0x26 */
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
    u16 unk13A[14];      /* 0x13a: ids the item list screens show */
    u8 unk156[14];       /* 0x156: their stock (src/st/save.c) */
    u8 unk164[4];
} Data801c7590;
extern Data801c7590 DAT_801c7590;

extern u32 DAT_801cceb0[2][5];

typedef struct {
    u16 unk0;            /* 0x00: unk2 of the previous frame (src/st/pad.c) */
    u16 unk2;            /* 0x02: held bits */
    u16 unk4;            /* 0x04: page bits func_8002fc4c consumes; request bits func_8003f4ec clears */
    u16 unk6;            /* 0x06..0x0c: src/st/pad.c's repeat and combination words */
    u16 unk8;
    u16 unkA;
    u16 unkC;
    u8 unkE;             /* 0x0e: 1 while the controller is disconnected */
    u8 unkF;
    u8 unk10;
    u8 unk11;
    u8 unk12;            /* 0x12 */
    u8 unk13;
    u8 unk14;            /* 0x14: request func_80042900 consumes */
    /* 0x15..0x17: written by the vsync-time pad update; every access reloads them */
    volatile u8 unk15;   /* 0x15: last PadGetState(0) */
    volatile u8 unk16;   /* 0x16: disconnected since the last stable state */
    volatile u8 unk17;   /* 0x17: PadStateFindPad seen; actuator alignment pending */
    u8 unk18;            /* 0x18: PadInfoAct(0, -1, 0) */
    u8 unk19;            /* 0x19: frames of vibration left */
    u8 unk1A[6];         /* 0x1a: PadSetActAlign data */
    u8 unk20[2];
    u16 unk22;           /* 0x22..0x38: button masks func_800573bc sets */
    u16 unk24;
    u16 unk26;           /* 0x26 */
    u16 unk28;           /* 0x28 */
    u16 unk2A;
    u16 unk2C;
    u16 unk2E;           /* 0x2e */
    u16 unk30;           /* 0x30 */
    u16 unk32;           /* 0x32: slot 0's request bit; bit masks tested against unk2/unk4 */
    u16 unk34;           /* 0x34: the other slots' request bit */
    u16 unk36;           /* 0x36 */
    u16 unk38;
    u16 unk3A;           /* 0x3a: unk32 | unk34 | unk36 | unk38 */
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

/* The CD file buffer func_80016030 loads TIMs and models into (WIP extent:
 * up to DAT_8009ac88). */
extern u_long DAT_80088c88[];

/* Two 0x4000-byte load buffers (viewer.unk120). */
extern u8 DAT_80190f28[2][0x4000];
/* Four 0x7000-byte model blocks (viewer.unkF8). */
extern u8 DAT_80199220[4][0x7000];
/* The 64 equipment model placements viewer.unkE8 points into. */
extern ViewerModel DAT_801b5220[64];

/* Settings bytes func_8003ea20 initializes (WIP extent). */
typedef struct {
    u8 unk0;
    u8 unk1;             /* option page level, shown as a slider and a number (menu.c) */
    u8 unk2;             /* 0x02..0x05: option page on/off bytes */
    u8 unk3;             /* nonzero while the HUD is drawn (func_80026a88, func_80026afc) */
    u8 unk4;             /* 1: the title-card image is drawn whatever the card mode (text.c) */
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;
} Data801cd120;
extern Data801cd120 DAT_801cd120;

#endif
