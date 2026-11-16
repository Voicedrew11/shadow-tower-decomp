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
    u16 unk1C;           /* 0x1c: reach */
    u16 unk1E;           /* 0x1e: height added while resolving */
    s16 unk20;           /* 0x20: the velocity's heading */
    s16 unk22;           /* 0x22: its horizontal length */
    u8 unk24[8];
    s32 unk2C;           /* 0x2c: floor height found */
    u8 unk30[0xc];
    s32 unk3C;
    u8 unk40[4];
    s32 unk44;
    u8 unk48[0x14];
    s32 unk5C;           /* 0x5c: an index into vis_table_a.entries */
    s32 unk60;           /* 0x60: func_8004be80's result */
    s32 unk64;           /* 0x64: an index into vis_table_b.entries */
    u8 unk68[0xc];
    s32 unk74;           /* 0x74: the kind bits func_80049544 resolved */
    s32 unk78;           /* 0x78: heading of the wall slid along */
    u8 unk7C[0x10];
    s16 unk8C;           /* 0x8c..0x90: wall normal (bit 1 hits) */
    u8 unk8E[2];
    s16 unk90;
    u8 unk92[0xa];
    s16 unk9C;           /* 0x9c..0xa0: wall normal (bit 3 hits) */
    u8 unk9E[2];
    s16 unkA0;
    u8 unkA2[0xa];
    VECTOR unkAC;        /* 0xac: the resolved source's position */
    s32 unkBC;           /* 0xbc: its horizontal distance */
    u8 unkC0[0x44c0 - 0xc0];
    u16 unk44C0;         /* 0x44c0 */
    u16 unk44C2;         /* 0x44c2 */
    u8 unk44C4[2];
    s16 unk44C6;         /* 0x44c6: push along the wall */
    u8 unk44C8[2];
    u16 n_sources;       /* 0x44ca */
    u16 n_visible;       /* 0x44cc */
    u16 sources[0x80];   /* 0x44ce */
    u16 visible[0x80];   /* 0x45ce */
    u8 unk46CE[0x47d8 - 0x46ce];
    u16 unk47D8;         /* 0x47d8: record index of the floor found */
    u8 unk47DA[0xa];
    s32 unk47E4;         /* 0x47e4: floor height (func_80048ad0) */
    s32 unk47E8;         /* 0x47e8: ceiling height */
    u8 unk47EC[4];
} VisLists;

/* 0x8017bed0: 12-byte records indexed by visible id. */
typedef struct {
    u8 part;             /* 0x00: the model part drawn for this record */
    u8 unk01;
    u8 unk02;
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
/* A flags word whose byte +2 is also updated on its own (func_800241d0). */
typedef union {
    u32 word;
    u8 bytes[4];
} VisFlags;

typedef struct {
    u8 unk00;            /* 0x00: 0xff when free (func_8004afdc) */
    u8 unk01;
    u8 unk02;            /* 0x02: index of its 192-byte VisTableA record */
    u8 unk03;
    u8 unk04;
    u8 unk05;            /* 0x05: state (func_8004b2dc) */
    u8 unk06;
    u8 unk07;
    u8 unk08;            /* 0x08 */
    u8 unk09[2];
    u8 unk0B;            /* 0x0b: model slot - 128 */
    u8 unk0C;            /* 0x0c: block tag */
    u8 unk0D;            /* 0x0d: added to vis_table_a.unk6128 while live */
    u8 unk0E;
    u8 unk0F;
    u16 unk10;           /* 0x10..0x14: the three items it drops */
    u16 unk12;
    u16 unk14;
    s16 unk16;           /* 0x16: vertical speed */
    u8 unk18;
    u8 unk19;
    u8 unk1A;
    u8 unk1B;
    VisFlags unk1C;      /* 0x1c: byte 2 bits 6/7 near and far range */
    u8 unk20;
    s8 unk21;            /* 0x21: OT depth bias */
    s8 unk22;            /* 0x22: voice, -1 none */
    u8 unk23;
    s16 unk24;           /* 0x24 */
    u16 unk26;
    u16 unk28;           /* 0x28: record index (func_8004ac38) */
    u16 unk2A;           /* 0x2a */
    u16 unk2C;
    u16 unk2E;           /* 0x2e */
    u16 unk30;           /* 0x30: height */
    u16 unk32;           /* 0x32: shadow size */
    s16 unk34;           /* 0x34 */
    s16 unk36;           /* 0x36 */
    u8 unk38;
    u8 unk39;
    u8 unk3A[2];
    u32 unk3C;           /* 0x3c: squared distance */
    s32 unk40;
    s32 unk44;
    u8 unk48[4];
    s32 unk4C;           /* 0x4c: height above the floor */
    s32 unk50;           /* 0x50: request kind bits */
    u16 unk54;
    u16 unk56;           /* 0x56: facing the viewer is compared with */
    u8 unk58[6];
    u16 unk5E;           /* 0x5e: heading of the wall slid along */
    u16 unk60;
    u8 unk62[3];
    u8 unk65;            /* 0x65..0x67: percent chances of unk10..unk14 */
    u8 unk66;
    u8 unk67;
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
    SVECTOR unk9C;       /* 0x9c: velocity */
    u32 unkA4;           /* 0xa4 */
    u8 *unkA8;           /* 0xa8: an entry of its record's unk60 list */
    u8 unkAC[4];
    s16 unkB0;           /* 0xb0: magnitude of the last unk2A step */
    u16 unkB2;
    u8 unkB4[0x20];
} Vis212;

/* 0x3500 of VisTableA: 192-byte records. */
typedef struct {
    u8 unk00;
    u8 unk01[4];
    u8 unk05;
    u8 unk06[8];
    s16 unk0E;           /* 0x0e: gravity */
    u16 unk10;           /* 0x10: map bit, 0xffff none */
    u16 unk12;           /* 0x12: added to vis_table_a.unk612C while live */
    u8 unk14[2];
    u16 unk16;
    u16 unk18;
    u16 unk1A;
    u32 unk1C;
    u32 unk20;
    s8 unk24[14];        /* 0x24: bonuses to the viewer's figures */
    u16 unk32;
    u8 unk34[0x16];
    u16 unk4A[9];        /* 0x4a: level weights (func_8004de24) */
    u8 unk5C[4];
    u8 *unk60[24];       /* 0x60: null-terminated (func_8004b1e8) */
} Vis192;

typedef struct {
    Vis212 entries[64];
    Vis192 unk3500[58];
    u8 unk6080[0x80];
    u8 count;            /* 0x6100 */
    u8 unk6101;
    u8 unk6102;
    u8 unk6103;
    u16 list[12];        /* 0x6104 */
    u8 unk611C[0xc];
    s32 unk6128;         /* 0x6128: sum of the live entries' unk0D */
    s32 unk612C;         /* 0x612c: sum of their records' unk12 */
    s32 unk6130;
    Vis192 *unk6134;     /* 0x6134: the current record */
    Vis212 *unk6138;     /* 0x6138: the current entry */
    Vis192 *unk613C;     /* 0x613c: the record of unk6140 */
    Vis212 *unk6140;     /* 0x6140: the entry the current one follows */
    s32 unk6144;
    s32 unk6148;         /* 0x6148: the current entry's record index */
    u8 unk614C[4];
    s32 unk6150;
    u8 unk6154[4];
    u16 unk6158[4];
    u8 unk6160[8];
    u8 unk6168;
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
    u8 unk71a0[0x14];
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
    u8 unk4;             /* 0xff: none (func_80014544) */
    u8 unk5[0xb];
    u8 unk10[14];        /* 0x10: counts func_80031674 lists */
    u16 unk1E;           /* 0x1e */
    u8 unk20[4];
    u16 unk24;           /* 0x24: func_80031378 shows it / 4.54 */
    u8 unk26;
    u8 unk27;            /* 0x27 */
    u8 unk28;            /* 0x28 */
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
    u8 unk1E[2];
    u16 unk20;
    u8 unk22;            /* 0x22: kind; 0 and 4 are not drawn */
    u8 unk23;
    u8 unk24;            /* 0x24 */
    u8 unk25;
    u16 unk26;
    s32 unk28;           /* 0x28: distance */
    u8 unk2c;
    s8 unk2D;            /* 0x2d: OT depth bias */
    u8 unk2E[2];
} Vis48;

typedef struct {
    u8 unk0000[6];       /* [0]: drops made; [2], [3]: slots used of entries[32..47], [48..63] */
    u8 count;            /* 0x0006 */
    u8 list[64];         /* 0x0007 */
    u8 unk0047;
    u32 unk0048[10];     /* 0x0048: one bit per dropped id */
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
int func_8004c934(Vis212 *e, int index, VECTOR *out);
int func_8004cc70(Vis212 *e, VECTOR *pos, int speed, VECTOR *target, SVECTOR *dir, SVECTOR *out,
                  int pitch, int turn, int n);
void func_8004acec(Vis212 *e);
void func_8004ad70(Vis212 *e);
void func_8004ae38(Vis212 *e);
void func_8004af24(Vis212 *e, Vis192 *r);
void func_8004af7c(u16 i);
void func_8004b058(Vis212 *e, u8 *p);
int func_8004b090(u8 *p, int arg);
void func_8004b098(int arg);
void func_8004b19c(void);
u8 *func_8004b1e8(Vis192 *r, u8 id);
void func_8004b230(Vis212 *e, u8 id);
void func_8004b270(void);
void func_8004b2b4(u8 v);
void func_8004b2dc(void);
int func_8004b5d8(int a, int b, int c);
int func_8004b8ec(int x, int y, int z, int r, VECTOR *a, VECTOR *b, int extra);
void func_8004c03c(Vis212 *e);
void func_8004c0f4(Vis212 *e, short step);
void func_8004c128(Vis212 *e, short step);
int func_8004c184(Vis212 *e, u16 v);
void func_8004c1b4(Vis212 *e);
void func_8004c808(void);
VECTOR *func_8004ca34(Vis212 *e, VECTOR *out);
void func_8004ca90(SVECTOR *off, VECTOR *out, VECTOR *base, int arg3, short height, short up);
void func_8004cba0(SVECTOR *off, VECTOR *out, VECTOR *base);

/* src/st/vis_item.c */
int func_8004d75c(Vis212 *e);
void func_8004d764(Vis212 *e);
void func_8004d7fc(void);
void func_8004d804(Vis212 *e, int id);
void func_8004da2c(Vis212 *e);
void func_8004db0c(Vis212 *e);
int func_8004db9c(Vis212 *e, short reach);
int func_8004de24(Vis212 *e, int arg, u16 *levels);

#endif
