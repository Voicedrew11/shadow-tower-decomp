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
 * root counter and vsync words, with a 4-entry ring, then the load cursor
 * and the load buffer.
 */
typedef struct {
    u8 unk0[8];
    StCdRequest unk8[VS_CD_REQUESTS];
    StCdRequest *unkD8; /* 0xd8: the next free entry (ST: unk9D4) */
    StCdRequest *unkDC; /* 0xdc: the entry being read (ST: unk9D8) */
    VsArchive archives[ST_ARCHIVES]; /* 0xe0: count as in ST.EXE (WIP) */
    u8 unk1D0[0x4a0 - 0x1d0];
    u_long *unk4A0;      /* 0x4a0: load cursor in unk4A4 (func_8001789c); the menu file buffer (func_8001bb48) */
    u_long unk4A4[0x8000]; /* 0x4a4: the 0x20000-byte load buffer */
} VsCdState;

typedef char VsCdRequestRingStride[(sizeof(((VsCdState *)0)->unk8[0]) == 0x34) ? 1 : -1];
typedef char VsCdRequestRingCount[(sizeof(((VsCdState *)0)->unk8) == 4 * 0x34) ? 1 : -1];
typedef char VsCdRequestRingAt8[(offsetof(VsCdState, unk8) == 8) ? 1 : -1];
typedef char VsCdFreeAt0xd8[(offsetof(VsCdState, unkD8) == 0xd8) ? 1 : -1];
typedef char VsCdCurrentAt0xdc[(offsetof(VsCdState, unkDC) == 0xdc) ? 1 : -1];


/* Layout check: the offset the code proves. */
typedef char VsCdStateArchivesAt0xE0[offsetof(VsCdState, archives) == 0xe0 ? 1 : -1];
typedef char VsCdStateUnk4A4At0x4A4[offsetof(VsCdState, unk4A4) == 0x4a4 ? 1 : -1];

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
    u8 unk1C7F0[0x2c970 - 0x1c7f0];
    u8 unk2C970;         /* 0x2c970: frames drawn since the last rate update (func_80015e50) */
    u8 unk2C971;         /* 0x2c971: the frame rate func_80015e50 shows */
    u8 frame;            /* 0x2c972: index of the current DRAWENV and buffers (ST: 0x1ecc2) */
    u8 unk2C973;
    u8 unk2C974;         /* 0x2c974: set while the loading display runs (ST: unk1ECC4) */
    u8 unk2C975[0x2c998 - 0x2c975];
    MATRIX unk2C998;     /* 0x2c998: the camera matrix */
    u8 unk2C9B8[0x2ca78 - 0x2c9b8];
    VECTOR unk2CA78;     /* 0x2ca78: the camera position */
    SVECTOR unk2CA88;    /* 0x2ca88: the camera angles (func_80024b5c) */
} VsWorldState;

/* Layout checks: the offsets the code proves. */
typedef char VsWorldStateAnimSlotsAt0xA3FC[offsetof(VsWorldState, anim_slots) == 0xa3fc ? 1 : -1];
typedef char VsWorldStateFrameAt0x2C972[offsetof(VsWorldState, frame) == 0x2c972 ? 1 : -1];
typedef char VsWorldStateCameraAt0x2CA78[offsetof(VsWorldState, unk2CA78) == 0x2ca78 ? 1 : -1];

extern VsWorldState vs_gfx_state;

/* 2000 20-byte particle records at 0x801c4a58 (bss): func_8002d738
 * starts them, func_8002d710 clears their lives. */
typedef struct {
    SVECTOR unk0;            /* 0x00: position */
    SVECTOR unk8;            /* 0x08: velocity */
    u16 unk10;               /* 0x10: life, 0 when free */
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
    u16 unk0;                /* 0x00: effect kind (func_8002f334) */
    s16 unk2;                /* 0x02..0x06: the effect's offset from the fighter, turned by its facing */
    s16 unk4;
    s16 unk6;
    u8 unk8[2];
    s16 unkA;                /* 0x0a: the frame it strikes on (func_80031478) */
    u16 unkC;                /* 0x0c: its cost to the second gauge */
    u16 damage[9];           /* 0x0e */
} VsAttack;

/* A hit func_8002e114 applies: three base damage amounts and an amount of
 * one further kind (3 + kind). */
typedef struct {
    u16 damage[3];
    s16 kind;
    u16 amount;
} VsHit;

/* A fighter's 70-byte hit record for one attack animation
 * (func_80031478): up to three hits, each landing on its frame within its
 * reach and arc, and up to three lunges that move the fighter between two
 * frames. */
typedef struct {
    u8 count;
    u8 unk1;
    struct {
        s16 frame;
        s16 range;           /* added to the fighter's hit radius */
        s16 arc;
        VsHit hit;
    } hits[3];               /* 0x02 */
    u8 count2;               /* 0x32 */
    u8 unk33;
    struct {
        s16 from;
        s16 to;
        s16 speed;
    } moves[3];              /* 0x34 */
} VsHitRecord;

typedef char VsHitRecordSize[sizeof(VsHitRecord) == 70 ? 1 : -1];

/* A fighter's animation sound effects (func_80013c5c), a view of the first
 * 0x64 bytes of its VsFighterData block: keys[i].frame is the animation id
 * of VsFighterData.unk0[2 + i] (compared with VsFighter.unk4DEE8), and per
 * entry a tone and a trigger word (kind in the top nibble, a frame or
 * period below). */
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

/* An animation a fighter plays: its id and its frame step (negated to
 * play it backwards). */
typedef struct {
    u8 unk0;                 /* animation id */
    u8 unk1;
    u16 unk2;                /* frame step */
} VsAnimRef;

/*
 * A fighter's data block at +0x4df18 to the record's end: the movement
 * code addresses its animations and walking speed through one pointer to
 * it, so it is one object. Offsets in the field names are block-relative.
 */
typedef struct {
    VsAnimRef unk0[13];      /* 0x000: animations (count WIP) */
    u8 unk34[0x78 - 0x34];
    VsHitRecord unk78[3];    /* 0x078: hit records of the three combo animations (func_80031478) */
    VsAttack unk14A[3];      /* 0x14a: attacks (count WIP) */
    s16 unk1AA;              /* 0x1aa: movement kind */
    s16 unk1AC;              /* 0x1ac: walking speed */
    s16 unk1AE;              /* 0x1ae: turning speed */
    u8 unk1B0[0x1b2 - 0x1b0];
    s16 unk1B2;              /* 0x1b2: the start height, negated (func_800140d8) */
    s16 unk1B4;              /* 0x1b4: hit radius */
    s16 unk1B6;              /* 0x1b6: height */
    s16 unk1B8;              /* 0x1b8: full hit points */
    s16 unk1BA;              /* 0x1ba: full second gauge (0: none) */
    u16 unk1BC[9];           /* 0x1bc: resistance per damage kind */
} VsFighterData;

/* func_800140d8 copies a whole record (0x1ce bytes, halfword aligned). */
typedef char VsFighterDataSize[sizeof(VsFighterData) == 0x1ce ? 1 : -1];

/* The fighters' data blocks by character, copied into VsFighter.unk4DF18
 * when a round loads (.data; WIP extent: one record). */
extern VsFighterData DAT_8004a224[];

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
    StAnimSlot *unk4DEE4;    /* 0x4dee4: the fighter's vertex-animation slot (func_80028914) */
    u8 unk4DEE8;             /* 0x4dee8: animation playing (VsAnimRef.unk0); func_80013c5c matches it against the sound keys */
    u8 unk4DEE9;
    s16 unk4DEEA;            /* 0x4deea: animation frame */
    s16 unk4DEEC;            /* 0x4deec */
    u8 unk4DEEE[0x4def0 - 0x4deee];
    VECTOR unk4DEF0;         /* 0x4def0: position */
    s16 unk4DF00;            /* 0x4df00: rotation (an SVECTOR over 0x4df00..0x4df06): x */
    s16 unk4DF02;            /* 0x4df02: facing angle */
    s16 unk4DF04;            /* 0x4df04: z */
    u8 unk4DF06[0x4df08 - 0x4df06];
    s16 unk4DF08;            /* 0x4df08: hit points */
    s16 unk4DF0A;            /* 0x4df0a: second gauge */
    s16 unk4DF0C;            /* 0x4df0c: hit points left by the last hit */
    s16 unk4DF0E;            /* 0x4df0e: state; 3 hit, 4 down; 3 and 4 cannot be hit */
    s16 unk4DF10;            /* 0x4df10: model kind: 1 several parts, 2 one part */
    s16 unk4DF12;            /* 0x4df12: model brightness; 128 and up: the default colour (func_80025204) */
    s16 unk4DF14;            /* 0x4df14: state when an attack was blocked */
    s16 unk4DF16;            /* 0x4df16: hits left in the current attack (func_80031478) */
    VsFighterData unk4DF18;  /* 0x4df18: the fighter's data block (its first 0x64 bytes: VsFighterSfx) */
} VsFighter;

/* Layout check: the record size the index arithmetic proves. */
typedef char VsFighterSize[sizeof(VsFighter) == 0x4e0e8 ? 1 : -1];
typedef char VsFighterDataAt0x4DF18[offsetof(VsFighter, unk4DF18) == 0x4df18 ? 1 : -1];

/* The fighters live in DAT_800633b0 (vs/buffer.h VsBuffer.fighter). */

/* A movement scale the fighters' walking steps multiply by (.sdata,
 * unowned; read absolutely by the -G8 moves). */
extern s32 DAT_80063220;

/* Nonzero (1) while the fighters may hit each other (bss, unowned). */
extern s32 DAT_80063324;

/* Renderer switches (unowned): a fixed OT depth for every face (.sdata),
 * semi-transparent faces and mirrored winding (bss). */
extern s32 DAT_80063224;
extern s32 DAT_80063350;
extern s32 DAT_80063360;

/* DAT_80063220 (declared above) is also the animation step the sound
 * triggers scale a fighter's frame step by. */


/* VS's copy of ST.EXE's vis_table_c (bss, 0x80191d00): the 44-byte item
 * records from +0x118; only those are used so far. */
extern VisTableC DAT_80191d00;

/* A switch func_800222d8 sets around its count list and func_8001d9cc
 * reads (bss). */
extern u16 DAT_8006337a;

/* A light matrix, a light colour matrix, a back colour and the depth-cue
 * near and far depths, as the model renderers load them (func_80025204,
 * func_800353d0). Halfword-aligned: retail copies its initializers with
 * an alignment test. */
typedef struct {
    short l[3][3];
    short pad0;
    short lc[3][3];
    short pad1;
    CVECTOR bk;
    short dq[2];
} VsLights;

/* The fighter being drawn (bss, unowned). */
extern VsFighter *DAT_80063300;

/* Rotation offsets added to the drawn fighter's x and y angles (.sdata,
 * unowned; read absolutely). */
extern s32 DAT_80063218;
extern s32 DAT_8006321c;

/* The model renderers' scale (4096, 4096, 4096; .data, unowned). */
extern VECTOR DAT_8005b2a8;

/* The column of DAT_801624b8 func_8002ad40 shows (bss, unowned). */
extern s32 DAT_80063338;

/* Each fighter's three ids func_8002ad40 names are DAT_801624b8, both
 * players' teams (vs/menu.h VsTeam.slot). */

/* A 5000-byte file buffer (archive 5 member 3, func_80016488): the guard
 * flash's mesh model, a centre vertex and three rings of 16 (bss). */
extern u8 DAT_8015ec38[5000];

/* Rounds won per fighter (bss, unowned): src/vs/main.c proves the first
 * word its own symbol (the second, DAT_80063374, is read absolutely where
 * the first is $gp-relative); func_80029f28 still walks both from it. */
extern s32 DAT_80063370[1];

/* VS band 6: the text scroll switch and the two scroll offsets
 * func_8001d3f0 picks between (ST.EXE's DAT_800881ac/a0/a2; bss, unowned). */
extern u16 DAT_80063378;
extern u16 DAT_8006333e;
extern u16 DAT_80063340;

/* VS band 6: the two 0x20000-byte GPU packet buffers func_80014480 hands
 * the graphics state (bss; ST.EXE's DAT_8011c488). */
extern u_long DAT_8011c528[];

/* VS band 6: seven words and seven halfwords func_8001f9a0 clears, and the
 * two players' picked fighters, -1 for none (bss, unowned). */
extern s32 DAT_801c4a38[7];
extern u16 DAT_80117598[7];
/* The two players' picked fighters, -1 for none. Retail's translation units
 * see two sizes: the team, pick and setup screens read both words
 * $gp-relative through an 8-byte array (define VS_PICKED_PAIR before
 * including this header), while the main module and the card screen see
 * only the first word here and the second as DAT_80063334 (vs/main.h). */
#ifdef VS_PICKED_PAIR
extern s32 DAT_80063330[2];
#else
extern s32 DAT_80063330[1];
#endif

/* Read-only words the first-pass round sequence and effect code read
 * directly (WIP; 0x80011934 is the start of vs.effect_80032404's
 * JUMPTABLES claim, 0x8001150c of vs.fade_80028fcc's). */
extern s32 DAT_80011934;
extern s32 DAT_8001147c;
extern s32 DAT_80011480;
extern s32 DAT_80011484;
extern s32 DAT_80011488;
extern s32 DAT_8001148c;
extern s32 DAT_800114cc;
extern s32 DAT_8001150c;

/* A halfword the team screen and the setup page read (bss, unowned). */
extern u16 DAT_8006335c;
#endif
