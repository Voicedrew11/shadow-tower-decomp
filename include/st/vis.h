#ifndef ST_VIS_H
#define ST_VIS_H

/*
 * ST.EXE visible-set lists (band 0x8001c100..0x8001cb88, WIP). All objects
 * here are unresolved .bss globals; only the members these functions touch
 * are named, as unkXXX until their meaning is proven. The viewer and the
 * area state are shared with other modules (st/data.h); the scratchpad is
 * st/scratch.h.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/data.h>
#include <st/scratch.h>

/* 0x801b5948: the source id list vis_collect_sources fills and the visible id
 * list vis_build_visible builds from it; vis_record_is_visible/vis_id_is_visible search the
 * visible list. */
typedef struct {
    u8 unk0000[0x30];
    s32 unk30;           /* 0x30: a height func_8003bdb0 tests */
    u8 unk34[0x4c - 0x34];
    s32 unk4C;           /* 0x4c: 0x80000 when unset (func_8003be10) */
    u8 unk50[0x58 - 0x50];
    s32 unk58;           /* 0x58: 0x80000 when unset */
    u8 unk5C[0x44ca - 0x5c];
    u16 n_sources;       /* 0x44ca */
    u16 n_visible;       /* 0x44cc */
    u16 sources[0x80];   /* 0x44ce */
    u16 visible[0x80];   /* 0x45ce */
    u8 unk46CE[0x47ce - 0x46ce];
    s16 unk47CE;         /* 0x47ce..0x47d2: cleared by player_update_status */
    s16 unk47D0;
    s16 unk47D2;
    u8 unk47D4[0x47dc - 0x47d4];
    s32 unk47DC;         /* 0x47dc: cleared by func_8003c054 */
} VisLists;

/* 0x8017bed0: 12-byte records indexed by visible id. */
typedef struct {
    u8 part;             /* 0x00: the model part drawn for this record */
    u8 unk01[2];
    u8 unk03;            /* 0x03: bits 0-1 select the light set */
    u8 unk04;            /* bit 1: flagged by vis_cull_by_range */
    u8 unk05[3];
    u8 unk08;
    u8 unk09;
    u8 unk0a;
    u8 unk0b;
} VisRecord;

/* 0x8017d6d0: 64-byte records, a 512-bit mask per source id. */
typedef struct {
    u_long bits[16];
} VisSourceMask;

extern VisLists vis_lists;
extern VisRecord vis_records[];
extern VisSourceMask vis_source_masks[];

/* 0x801ba138: 64 entries of 212 bytes, then the collected index list. */
typedef struct {
    u8 unk00[3];
    u8 unk03;
    u8 unk04;
    u8 unk05;
    u8 unk06[0x20];
    u16 unk26;
    u8 unk28[0xac];
} Vis212;

typedef struct {
    Vis212 entries[64];
    u8 unk3500[0x2c00];
    u8 count;            /* 0x6100 */
    u8 unk6101[3];
    u16 list[12];        /* 0x6104 */
    u8 unk611C[0x6160 - 0x611c];
    s16 unk6160;         /* 0x6160..0x6164: cleared by player_update_status */
    s16 unk6162;
    s16 unk6164;
} VisTableA;

/* 0x801c0310: 240 entries of 88 bytes at +0x1f20, then the index list. */
typedef struct {
    u8 unk00[4];
    u8 unk04;
    u8 unk05;
    u8 unk06;
    u8 unk07;
    u16 unk08;
    u16 unk0a;
    u8 unk0c[0x4c];
} Vis88;

typedef struct {
    u8 unk0000[0x1f20];
    Vis88 entries[240];
    u8 unk71a0[4];
    s16 unk71A4;         /* 0x71a4..0x71a8: cleared by player_update_status */
    s16 unk71A6;
    s16 unk71A8;
    u8 unk71AA[0xa];
    u16 list[96];        /* 0x71b4 */
    u8 count;            /* 0x7274 */
} VisTableB;

/*
 * 0x801c76f8: the index list, a bit per id at +0x70, 314 records of 44
 * bytes from +0x118 (func_80014544 reads byte +4 of each), then 64 entries
 * of 48 bytes at +0x3710.
 */
typedef struct {
    u8 unk0[4];
    u8 unk4;             /* 0xff: none (func_80014544); func_8003f12c's model */
    u8 unk5[0x20 - 0x5];
    /* 0x20: three bonuses (kind, amount) player_equipment_bonuses sums */
    u32 kind0 : 4;
    u32 amount0 : 4;
    u32 kind1 : 4;
    u32 amount1 : 4;
    u32 kind2 : 8;
    u32 amount2 : 8;
    u8 unk24[4];
    u8 unk28;            /* 0x28: durability, 0 when broken (func_8003d104) */
    u8 unk29[3];
} Vis44;

typedef struct {
    u16 unk00;
    u16 unk02;
    u8 unk04[0x28];
    u8 unk2c;
    u8 unk2d[3];
} Vis48;

typedef struct {
    u8 unk0000[6];
    u8 count;            /* 0x0006 */
    u8 list[64];         /* 0x0007 */
    u8 unk0047[0x70 - 0x47];
    u32 unk0070[10];     /* 0x0070: one bit per id */
    u8 unk0098[0x118 - 0x98];
    Vis44 unk0118[314];  /* 0x0118 */
    Vis48 entries[64];   /* 0x3710 */
} VisTableC;

extern VisTableA vis_table_a;
extern VisTableB vis_table_b;
extern VisTableC vis_table_c;

void vis_collect_sources(u8 *arg, int a1, int a2);
void vis_build_visible(u8 *arg);
void vis_build_planes(void);
int vis_measure_position(void);
void vis_cull_by_range(void);
int vis_record_is_visible(int a, int b, int c);
int vis_id_is_visible(u16 id);
void vis_collect_table_a(void);
void vis_collect_table_b(void);
void vis_collect_table_c(void);

#endif
