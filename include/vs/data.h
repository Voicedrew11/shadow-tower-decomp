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

#endif
