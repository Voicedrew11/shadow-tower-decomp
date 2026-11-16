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
    s32 unk0;            /* 0x00: the kind of the request func_80049544 takes */
    VECTOR unk4;         /* 0x04: its position */
    SVECTOR unk14;       /* 0x14: its velocity */
    s16 unk1C;           /* 0x1c */
    u16 unk1E;           /* 0x1e: height added while resolving */
    s16 unk20;           /* 0x20: the velocity's heading */
    s16 unk22;           /* 0x22: its horizontal length */
    u8 unk24[0x30 - 0x24];
    s32 unk30;           /* 0x30: a height func_8003bdb0 tests */
    u8 unk34[0x4c - 0x34];
    s32 unk4C;           /* 0x4c: 0x80000 when unset (func_8003be10) */
    u8 unk50[0x58 - 0x50];
    s32 unk58;           /* 0x58: 0x80000 when unset */
    s32 unk5C;           /* 0x5c: an index into vis_table_a.entries */
    s32 unk60;           /* 0x60: func_8004be80's result */
    s32 unk64;           /* 0x64: an index into vis_table_b.entries */
    u8 unk68[0xc];
    s32 unk74;           /* 0x74: the kind bits func_80049544 resolved */
    u8 unk78[0x34];
    VECTOR unkAC;        /* 0xac: the resolved source's position */
    s32 unkBC;           /* 0xbc: its horizontal distance */
    u8 unkC0[0x44c0 - 0xc0];
    u16 unk44C0;         /* 0x44c0 */
    u16 unk44C2;         /* 0x44c2 */
    u8 unk44C4[6];
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
    u32 unk47E0;         /* 0x47e0: flag bits func_80044404 tests */
    u8 unk47E4[0xc];
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
/* 0x801856d0: per axis (x, y, z) and 4096-unit cell, the sources that cover it. */
extern VisSourceMask DAT_801856d0[3][128];

/* 0x801ba138: 64 entries of 212 bytes, then the collected index list. */
/* A flags word whose byte +2 is also updated on its own (func_800241d0). */
typedef union {
    u32 word;
    u8 bytes[4];
} VisFlags;

typedef struct {
    u8 unk00[2];
    u8 unk02;            /* 0x02: index of its 192-byte VisTableA record */
    u8 unk03;
    u8 unk04;
    u8 unk05;
    u8 unk06[2];
    u8 unk08;            /* 0x08 */
    u8 unk09[2];
    u8 unk0B;            /* 0x0b: model slot - 128 */
    u8 unk0C;            /* 0x0c: block tag */
    u8 unk0D[0xf];
    VisFlags unk1C;      /* 0x1c: byte 2 bits 6/7 near and far range */
    u8 unk20;
    s8 unk21;            /* 0x21: OT depth bias */
    s8 unk22;            /* 0x22: voice, -1 none */
    u8 unk23;
    s16 unk24;           /* 0x24 */
    u16 unk26;
    u16 unk28;           /* 0x28: record index (func_8004ac38) */
    u16 unk2A;           /* 0x2a */
    u8 unk2C[2];
    u16 unk2E;           /* 0x2e */
    u16 unk30;           /* 0x30: height */
    u16 unk32;           /* 0x32: shadow size */
    s16 unk34;           /* 0x34 */
    s16 unk36;           /* 0x36 */
    u8 unk38[4];
    u32 unk3C;           /* 0x3c: squared distance */
    u8 unk40[0x28];
    u16 unk68;           /* 0x68: facing, 0xffff random (func_8004ac9c) */
    s16 unk6A;           /* 0x6a..0x6e: offset in its cell (func_8004ac38) */
    s16 unk6C;
    s16 unk6E;
    u32 unk70;           /* 0x70: bit 11 set by effect_emit */
    VECTOR unk74;        /* 0x74: position */
    u8 unk84[8];
    u16 unk8C;           /* 0x8c: rotation */
    s16 unk8E;           /* 0x8e: facing */
    u16 unk90;
    u8 unk92[2];
    SVECTOR unk94;       /* 0x94: scale */
    u8 unk9C[8];
    u8 unkA4[0x30];      /* 0xa4 */
} Vis212;

/* 0x3500 of VisTableA: 192-byte records. */
typedef struct {
    u8 unk00[0x10];
    u16 unk10;           /* 0x10: map bit, 0xffff none */
    u8 unk12[0xae];
} Vis192;

typedef struct {
    Vis212 entries[64];
    Vis192 unk3500[58];
    u8 unk6080[0x80];
    u8 count;            /* 0x6100 */
    u8 unk6101[3];
    u16 list[12];        /* 0x6104 */
    u8 unk611C[0x6144 - 0x611c];
    s32 unk6144;         /* 0x6144: loop index func_800402ac keeps in memory */
    u8 unk6148[0x6160 - 0x6148];
    s16 unk6160;         /* 0x6160..0x6164: cleared by player_update_status */
    s16 unk6162;
    s16 unk6164;
    u8 unk6166[0x6169 - 0x6166];
    u8 unk6169;          /* 0x6169: cleared by func_8004567c */
    u8 unk616A[0x61d8 - 0x616a];
} VisTableA;

/* 0x801c0310: 240 entries of 88 bytes at +0x1f20, then the index list. */
typedef struct {
    u8 unk00;            /* 0x00: 2 skips the entry (func_800246e4) */
    u8 unk01;
    u8 unk02;            /* 0x02 */
    u8 unk03;            /* 0x03: bits 6/7 near and far range */
    u8 unk04;
    u8 unk05;
    u8 unk06;
    u8 unk07;
    u16 unk08;
    u16 unk0a;           /* 0x0a: index of its 24-byte VisTableB record */
    u8 unk0c[4];
    u16 unk10;           /* 0x10 */
    u8 unk12[2];
    s16 unk14;           /* 0x14: OT depth bias */
    s16 unk16;           /* 0x16 */
    u32 unk18;           /* 0x18: squared distance */
    s32 unk1C;           /* 0x1c: distance (sound type 236) */
    u8 unk20[4];
    VECTOR unk24;        /* 0x24: position */
    u16 unk34;           /* 0x34: rotation */
    u16 unk36;
    u16 unk38;
    u8 unk3A[2];
    SVECTOR unk3C;       /* 0x3c: scale */
    u8 unk44[4];         /* 0x44 */
    u8 unk48;            /* 0x48: sound area width (2048 units) */
    u8 unk49;            /* 0x49: sound area depth */
    u8 unk4A;            /* 0x4a: sound id */
    u8 unk4B;            /* 0x4b: volume */
    u8 unk4C;            /* 0x4c: fade range */
    u8 unk4D;            /* 0x4d: sound flags */
    u16 unk4E;           /* 0x4e: repeat period */
    s32 unk50;           /* 0x50: next tick to play */
    s16 unk54;           /* 0x54: playing voice, -1 none */
    u8 unk56[2];
} Vis88;

/* VisTableB+0: 24-byte records. */
typedef struct {
    u8 unk00[3];
    u8 unk03;            /* 0x03 */
    u16 unk04;           /* 0x04 */
    u8 unk06[2];
    u16 unk08;           /* 0x08 */
    u8 unk0A[2];
    u16 unk0C;           /* 0x0c */
    u16 unk0E;           /* 0x0e */
    u8 unk10[8];
} Vis24;

typedef struct {
    Vis24 unk0000[330];
    u8 unk1EF0[0x30];
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
    u8 unk0[2];
    u8 unk2;             /* 0x02: carry limit or price base (src/st/menu_shop.c) */
    u8 unk3;             /* 0x03 */
    u8 unk4;             /* 0xff: none (func_80014544); func_8003f12c's model */
    u8 unk5[0xb];
    u8 unk10[14];        /* 0x10: counts func_80031674 lists */
    u16 unk1E;           /* 0x1e */
    /* 0x20: three bonuses (kind, amount) player_equipment_bonuses sums */
    u32 kind0 : 4;
    u32 amount0 : 4;
    u32 kind1 : 4;
    u32 amount1 : 4;
    u32 kind2 : 8;
    u32 amount2 : 8;
    u16 unk24;           /* 0x24: func_80031378 shows it / 4.54 */
    u8 unk26;
    u8 unk27;            /* 0x27 */
    u8 unk28;            /* 0x28: durability, 0 when broken (player_wear_item) */
    u8 unk29[3];
} Vis44;

typedef struct {
    u16 unk00;
    u16 unk02;
    u8 unk04[4];         /* 0x04 */
    VECTOR unk08;        /* 0x08: position */
    s16 unk18;           /* 0x18: rotation */
    s16 unk1A;
    s16 unk1C;
    u8 unk1E[4];
    u8 unk22;            /* 0x22: kind; 0 and 4 are not drawn */
    u8 unk23;
    u8 unk24;            /* 0x24 */
    u8 unk25[3];
    s32 unk28;           /* 0x28: distance */
    u8 unk2c;
    s8 unk2D;            /* 0x2d: OT depth bias */
    u8 unk2E[2];
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

void vis_collect_sources(VECTOR *a, VECTOR *b, VECTOR *c);
void func_8004656c(VECTOR *out);
void vis_build_visible(u8 *arg);
void vis_build_planes(void);
int vis_measure_position(void);
void vis_cull_by_range(void);
int vis_record_is_visible(int a, int b, int c);
int vis_id_is_visible(u16 id);
void vis_collect_table_a(void);
void vis_collect_table_b(void);
void vis_collect_table_c(void);

/* src/st/vis_request.c */
void func_800492f4(VECTOR *pos);
int func_80049544(VisLists *req);
int func_80048ad0(VisLists *req);
int func_8004bb88(VisLists *req, int arg1);
int func_8004be80(VisLists *req);
int func_8004f53c(VisLists *req);

/* src/st/vis_entry.c */
void func_8004ac38(Vis212 *e);
void func_8004ac9c(Vis212 *e);

#endif
