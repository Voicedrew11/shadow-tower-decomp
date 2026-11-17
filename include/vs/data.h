/*
 * VS.EXE (versus mode) globals that no reconstructed source owns yet. VS
 * links its own copies of many ST.EXE functions; their record types are
 * ST's (st/cd.h, st/gfx.h, ...), but the blocks holding them have VS sizes
 * and addresses, so each is declared here with a VS type and its
 * address-derived VS identity. Only the fields the reconstructed code
 * proves are spelled, at the offsets its instructions use.
 */
#ifndef VS_DATA_H
#define VS_DATA_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBCD.H>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/vis.h>

/* Ordering-table lengths, as in ST.EXE (src/st/gfx.c). */
#define VS_OT_LENGTH 4096
#define VS_OT2_LENGTH 64

/* The CD request ring holds 4 entries in VS (ST.EXE: 48). */
#define VS_CD_REQUESTS 4

/* An archive table: its start-sector table and file extent like ST.EXE's
 * StArchive, in an 80-byte record (the rest is unmodeled). */
typedef struct {
    u16 *sectors;
    StFileExtent file;
    u8 unkC[0x50 - 0xc];
} VsArchive;

/*
 * The CD state block at 0x801a4590 (bss): ST.EXE's StCdState without the
 * root counter and vsync words, with a 4-entry ring. WIP extent ends with
 * the archive tables.
 */
typedef struct {
    u8 unk0[8];
    StCdRequest unk8[VS_CD_REQUESTS];
    StCdRequest *unkD8; /* 0xd8: the next free entry (ST: unk9D4) */
    StCdRequest *unkDC; /* 0xdc: the entry being read (ST: unk9D8) */
    VsArchive archives[ST_ARCHIVES]; /* 0xe0: count as in ST.EXE (WIP) */
} VsCdState;

/* Layout check: the offset the code proves. */
typedef char VsCdStateArchivesAt0xE0[offsetof(VsCdState, archives) == 0xe0 ? 1 : -1];

extern VsCdState DAT_801a4590;

/*
 * The graphics state at 0x801624d0 (bss): laid out as ST.EXE's WorldState
 * through the per-frame counters at 0x8338; the frame index and the loading
 * flag sit at other offsets. WIP extent ends with the last field the code reads.
 */
typedef struct {
    DRAWENV draw[2];
    DISPENV disp[2];
    u_long *unkE0;
    u_long ot[2][VS_OT_LENGTH];
    u_long *ot_cur;      /* 0x80e4 */
    u_long ot2[2][VS_OT2_LENGTH];
    u_long *ot2_cur;
    StPacketBuffer packets[2];
    StPacketBuffer *packet; /* 0x8304 */
    void *unk8308[4];
    void *unk8318;
    s32 unk831C[2];
    s32 unk8324;
    s32 unk8328;
    s32 unk832C;
    s32 unk8330;
    s32 unk8334;
    s32 unk8338[5];
    void *unk834C[144];  /* 0x834c: model files by id, as in ST.EXE */
    u8 unk858C[0xa3fc - 0x858c];
    StAnimSlot anim_slots[12]; /* 0xa3fc: 12 vertex-animation slots (ST: 16 at 0xa5fc) */
    StAnimSet anim_sets[12]; /* 0xa51c: 12 skeletal-animation slots (ST: 16 at 0xa77c) */
    u8 unkA7EC[0xc7f0 - 0xa7ec];
    SVECTOR unkC7F0[0x2000]; /* 0xc7f0: the vertices func_800281d4 interpolates (ST: unkEB40) */
    u8 unk1C7F0[0x2c972 - 0x1c7f0];
    u8 frame;            /* 0x2c972: index of the current DRAWENV and buffers (ST: 0x1ecc2) */
    u8 unk2C973;
    u8 unk2C974;         /* 0x2c974: set while the loading display runs (ST: unk1ECC4) */
} VsWorldState;

/* Layout checks: the offsets the code proves. */
typedef char VsWorldStateAnimSlotsAt0xA3FC[offsetof(VsWorldState, anim_slots) == 0xa3fc ? 1 : -1];
typedef char VsWorldStateFrameAt0x2C972[offsetof(VsWorldState, frame) == 0x2c972 ? 1 : -1];

extern VsWorldState DAT_801624d0;

/* 2000 20-byte records at 0x801c4a58 (bss) whose halfword +0x10
 * func_8002d710 clears; only that field is proven. */
typedef struct {
    u8 unk0[0x10];
    u16 unk10;
    u8 unk12[2];
} VsRecord20;

extern VsRecord20 DAT_801c4a58[2000];

/* A flag the two-choice dialog func_80022f14 compares with its page
 * (0x80063228, .sdata, initially 1). */
extern s32 DAT_80063228;

/* The VRAM position func_8001cac0 saves for its read's completion
 * callback func_8001ca74 (bss). */
extern s32 DAT_80063304;
extern s32 DAT_80063308;

/* A fighter's attack record: its damage per kind (32 bytes, the stride
 * func_8002e314 indexes). */
typedef struct {
    u8 unk0[14];
    u16 damage[9];           /* 0x0e */
} VsAttack;

/* A hit func_8002e114 applies: three base damage amounts and an amount of
 * one further kind (3 + kind). */
typedef struct {
    u16 damage[3];
    s16 kind;
    u16 amount;
} VsHit;

/* A fighter's animation sound effects (func_80013c5c): the frames that
 * pick them in states 2 and 6, and per entry a tone and a trigger word
 * (kind in the top nibble, a frame or period below). */
typedef struct {
    u8 unk0[8];
    struct {
        u8 frame;
        u8 unk1[3];
    } keys[6];               /* 0x08 */
    u8 unk20[0x3c - 0x20];
    struct {
        u16 tone;
        u16 trigger;
    } sounds[10];            /* 0x3c */
} VsFighterSfx;

typedef char VsFighterSfxSize[sizeof(VsFighterSfx) == 0x64 ? 1 : -1];

/*
 * The two fighters at 0x800633b0 (bss), 0x4e0e8 bytes each (the code
 * indexes them by fighter number and takes the opponent as 1 - number).
 * A fighter's animated model file (st/anim.h StAnimFile) is loaded at its
 * start; only the state fields the code proves are spelled.
 */
typedef struct {
    u8 file[0x4b000];        /* 0x00000: the model file, then unmodeled */
    u_long unk4B000[(0x4dee0 - 0x4b000) / 4]; /* 0x4b000: a second mesh model, absent when its first word is 0 */
    StAnimSet *unk4DEE0;     /* 0x4dee0: the fighter's skeletal-animation slot */
    u8 unk4DEE4[0x4dee8 - 0x4dee4];
    u8 unk4DEE8;             /* 0x4dee8: the frame func_80013c5c matches against the sound keys */
    u8 unk4DEE9;
    s16 unk4DEEA;            /* 0x4deea: animation frame */
    s16 unk4DEEC;            /* 0x4deec */
    u8 unk4DEEE[0x4def0 - 0x4deee];
    VECTOR unk4DEF0;         /* 0x4def0: position */
    u8 unk4DF00[0x4df08 - 0x4df00];
    s16 unk4DF08;            /* 0x4df08: hit points */
    u8 unk4DF0A[2];
    s16 unk4DF0C;            /* 0x4df0c: hit points left by the last hit */
    s16 unk4DF0E;            /* 0x4df0e: state; 3 hit, 4 down; 3 and 4 cannot be hit */
    s16 unk4DF10;            /* 0x4df10: model kind: 1 several parts, 2 one part */
    u8 unk4DF12[0x4df18 - 0x4df12];
    VsFighterSfx unk4DF18;   /* 0x4df18: animation sound effects */
    u8 unk4DF7C[0x4e062 - 0x4df7c];
    VsAttack unk4E062[3];    /* 0x4e062: attacks (count WIP) */
    u8 unk4E0C2[0x4e0cc - 0x4e0c2];
    s16 unk4E0CC;            /* 0x4e0cc: hit radius */
    u8 unk4E0CE[0x4e0d4 - 0x4e0ce];
    u16 unk4E0D4[9];         /* 0x4e0d4: resistance per damage kind */
    u8 unk4E0E6[0x4e0e8 - 0x4e0e6];
} VsFighter;

/* Layout check: the record size the index arithmetic proves. */
typedef char VsFighterSize[sizeof(VsFighter) == 0x4e0e8 ? 1 : -1];

/* The fighters live in DAT_800633b0 (vs/buffer.h VsBuffer.fighter). */

/* Renderer switches (unowned): a fixed OT depth for every face (.sdata),
 * semi-transparent faces and mirrored winding (bss). */
extern s32 DAT_80063224;
extern s32 DAT_80063350;
extern s32 DAT_80063360;

/* The animation step the sound triggers scale a fighter's frame step by
 * (.sdata, 0 in the image; unowned). */
extern s32 DAT_80063220;

/* A frame count the periodic sound triggers divide (bss, unowned). */
extern u32 DAT_80160400;

/* VS's copy of ST.EXE's vis_table_c (bss, 0x80191d00): the 44-byte item
 * records from +0x118; only those are used so far. */
extern VisTableC DAT_80191d00;

/* A switch func_800222d8 sets around its count list and func_8001d9cc
 * reads (bss). */
extern u16 DAT_8006337a;

/* Rounds won per fighter (bss, unowned). */
extern s32 DAT_80063370[2];

/* VS band 6: the text scroll switch and the two scroll offsets
 * func_8001d3f0 picks between (ST.EXE's DAT_800881ac/a0/a2; bss, unowned). */
extern u16 DAT_80063378;
extern u16 DAT_8006333e;
extern u16 DAT_80063340;

/* VS band 6: the two 0x20000-byte GPU packet buffers func_80014480 hands
 * the graphics state (bss; ST.EXE's DAT_8011c488). */
extern u_long DAT_8011c528[];

#endif
