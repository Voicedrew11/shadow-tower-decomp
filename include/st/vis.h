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
    u16 unk1C;           /* 0x1c: added to a table B record radius (func_8004f53c) */
    u16 unk1E;           /* 0x1e: height added while resolving */
    s16 unk20;           /* 0x20: the velocity's heading */
    s16 unk22;           /* 0x22: its horizontal length */
    u8 unk24[8];
    s32 unk2C;           /* 0x2c: a floor height func_80048ad0 resolves (func_80053aa4) */
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
    u8 unk01;            /* 0x01: copied to a table A entry's unk06 (func_8004e960) */
    u8 unk02;
    u8 unk03;            /* 0x03: bits 0-1 select the light set */
    u8 unk04;            /* bit 1: flagged by vis_cull_by_range */
    u8 unk05;
    u8 unk06;
    u8 unk07;            /* 0x07: area id; 0xff none (func_8004e324) */
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

/* A table A record's script: a command stream (WIP). */
typedef struct {
    u8 unk00;            /* 0x00: kind; 'P' and '@' are picked by func_8004def8 */
    u8 unk01[0xb];
    u16 unk0C;           /* 0x0c */
} VisScript;

typedef struct {
    u8 unk00;            /* 0x00: 0xff when free */
    u8 unk01;
    u8 unk02;            /* 0x02: index of its 192-byte VisTableA record */
    u8 unk03;
    u8 unk04;
    u8 unk05;            /* 0x05: state; 1 active, 5 dying (func_8004def8) */
    u8 unk06;
    u8 unk07;            /* 0x07: area id, 0xfd: the previous area (func_8004e2d8) */
    u8 unk08;            /* 0x08 */
    u8 unk09;
    u8 unk0A;
    u8 unk0B;            /* 0x0b: model slot - 128 */
    u8 unk0C;            /* 0x0c: block tag */
    u8 unk0D;
    u8 unk0E;
    u8 unk0F;
    u16 unk10;           /* 0x10..0x14: three table C ids (func_8004e960) */
    u16 unk12;
    u16 unk14;
    s16 unk16;
    u8 unk18;            /* 0x18: index of the running script in its record */
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
    u16 unk2C;           /* 0x2c: hit points (func_8004def8) */
    u16 unk2E;           /* 0x2e */
    u16 unk30;           /* 0x30: height */
    u16 unk32;           /* 0x32: shadow size */
    s16 unk34;           /* 0x34 */
    s16 unk36;           /* 0x36 */
    u8 unk38;
    u8 unk39;
    u8 unk3A[2];
    u32 unk3C;           /* 0x3c: squared distance */
    s32 unk40;           /* 0x40: horizontal distance */
    u8 unk44[0x50 - 0x44];
    s32 unk50;
    u8 unk54[2];
    u16 unk56;           /* 0x56: heading to the viewer */
    u16 unk58;           /* 0x58: its difference from the facing */
    u8 unk5A[0x64 - 0x5a];
    u8 unk64;            /* 0x64: set when hit */
    u8 unk65;
    u8 unk66;
    u8 unk67;
    u16 unk68;           /* 0x68: facing, 0xffff random (func_8004ac9c) */
    s16 unk6A;           /* 0x6a..0x6e: offset in its cell (func_8004ac38) */
    s16 unk6C;
    s16 unk6E;
    u32 unk70;           /* 0x70: bit 11 set by effect_emit */
    VECTOR unk74;        /* 0x74: position */
    s32 unk84;
    u8 unk88[4];
    u16 unk8C;           /* 0x8c: rotation */
    s16 unk8E;           /* 0x8e: facing */
    u16 unk90;
    u8 unk92[2];
    SVECTOR unk94;       /* 0x94: scale */
    SVECTOR unk9C;       /* 0x9c: this frame's movement */
    u8 unkA4[4];         /* 0xa4 */
    VisScript *unkA8;    /* 0xa8: the running script */
    s32 unkAC;
    u16 unkB0;
    u8 unkB2[0xd4 - 0xb2];
} Vis212;

/* 0x3500 of VisTableA: 192-byte records. */
typedef struct {
    u8 unk00;            /* 0x00: 0xff ends the records (func_8004e8ec) */
    u8 unk01;
    u8 unk02;
    u8 unk03;
    u8 unk04;
    u8 unk05;
    u8 unk06[2];
    u8 unk08;
    u8 unk09[7];
    u16 unk10;           /* 0x10: map bit, 0xffff none */
    u16 unk12;
    u8 unk14[2];
    u16 unk16;
    u16 unk18;
    u8 unk1A[0x32 - 0x1a];
    u16 unk32;
    u8 unk34[0x60 - 0x34];
    VisScript *unk60[24]; /* 0x60: offsets from unk3500[16] until func_8004e8ec */
} Vis192;

typedef struct {
    Vis212 entries[64];
    Vis192 unk3500[58];
    u8 unk6080[0x80];
    u8 count;            /* 0x6100 */
    u8 unk6101[2];
    u8 unk6103;          /* 0x6103: the previous area id */
    u16 list[12];        /* 0x6104 */
    u8 unk611C[0x6128 - 0x611c];
    s32 unk6128;         /* 0x6128 */
    s32 unk612C;         /* 0x612c */
    s32 unk6130;
    u8 unk6134[0x6144 - 0x6134];
    s32 unk6144;         /* 0x6144: loop index func_800402ac keeps in memory */
    u8 unk6148[4];
    s32 unk614C;         /* 0x614c: active entries counted this frame */
    s32 unk6150;         /* 0x6150: frame counter */
    u8 unk6154[4];
    u16 unk6158;         /* 0x6158..0x615e: the last four area ids */
    u16 unk615A;
    u16 unk615C;
    u16 unk615E;
    s16 unk6160;         /* 0x6160..0x6164: cleared by player_update_status */
    s16 unk6162;
    s16 unk6164;
    u8 unk6166[2];
    u8 unk6168;          /* 0x6168: the current area id */
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
    u16 unk0C;           /* 0x0c: state of a moving entry (func_8004fdd0) */
    u8 unk0E[2];
    u16 unk10;           /* 0x10 */
    u16 unk12;           /* 0x12: height */
    s16 unk14;           /* 0x14: OT depth bias */
    s16 unk16;           /* 0x16 */
    u32 unk18;           /* 0x18: squared distance */
    s32 unk1C;           /* 0x1c: distance (sound type 236) */
    u16 unk20;           /* 0x20: facing */
    u8 unk22[2];
    VECTOR unk24;        /* 0x24: position */
    u16 unk34;           /* 0x34: rotation */
    s16 unk36;           /* 0x36: yaw */
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
    s32 unk50;           /* 0x50: next tick to play; other kinds keep halfwords here */
    s16 unk54;           /* 0x54: playing voice, -1 none */
    s16 unk56;
} Vis88;

/* VisTableB+0: 24-byte records. */
typedef struct {
    u8 unk00[3];
    u8 unk03;            /* 0x03 */
    u16 unk04;           /* 0x04 */
    u16 unk06;           /* 0x06: trigger radius (func_8004f718) */
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
    u8 unk71AA[6];
    Vis88 *unk71B0;      /* 0x71b0: an entry func_8004f53c skips */
    u16 list[96];        /* 0x71b4 */
    u8 count;            /* 0x7274 */
    u8 unk7275;
    s16 unk7276;         /* 0x7276..0x727a: cleared by func_8004eda8 */
    s16 unk7278;
    s16 unk727A;
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
    u8 unk1E[2];
    u16 unk20;           /* 0x20 */
    u8 unk22;            /* 0x22: kind; 0 and 4 are not drawn */
    u8 unk23;
    u8 unk24;            /* 0x24 */
    s8 unk25;            /* 0x25: bounces (func_80053aa4) */
    s16 unk26;           /* 0x26: fall speed */
    s32 unk28;           /* 0x28: distance */
    u8 unk2c;
    s8 unk2D;            /* 0x2d: OT depth bias */
    u8 unk2E[2];
} Vis48;

typedef struct {
    u8 unk0000[6];
    u8 count;            /* 0x0006 */
    u8 list[64];         /* 0x0007 */
    u8 unk0047;
    u32 unk0048[10];     /* 0x0048: one bit per loaded id (func_80052f30) */
    u32 unk0070[9];      /* 0x0070: one bit per id (266 ids) */
    u8 unk0094[0x80];    /* 0x0094: a slot per Vis44 unk04, 0xff unassigned (func_80052d10) */
    s32 unk0114;         /* 0x0114: the next slot to assign */
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

/* 0x8004acec..0x8004def8: no source yet. */
void func_8004acec(Vis212 *e);
void func_8004b2dc(void);
void func_8004c03c(Vis212 *e);
void func_8004c1b4(Vis212 *e);
void func_8004da2c(Vis212 *e);
void func_8004db0c(Vis212 *e);
int func_8004de24(Vis212 *e, int arg1, u16 *levels);

/* One 24-byte table A entry of an area file (func_8004e960). */
typedef struct {
    u8 unk00;            /* 0x00: 0xff: unused */
    u8 unk01;            /* 0x01: record index */
    u16 unk02;           /* 0x02: VisRecord index */
    u16 unk04[3];        /* 0x04: table C ids, 0xffff none */
    u8 unk0A;
    u8 unk0B;
    u8 unk0C;
    u8 unk0D;
    u8 unk0E[2];
    u16 unk10;           /* 0x10: facing */
    s16 unk12;           /* 0x12..0x16: offset in its cell */
    s16 unk14;
    s16 unk16;
} VisAFileEntry;

/* src/st/vis_a.c */
int func_8004def8(Vis212 *e, int kind, int power, u16 *levels, int crit);
void func_8004e2d8(void);
void func_8004e8ec(void);
void func_8004e960(VisAFileEntry *src);

/* src/st/vis_b_init.c (func_8004eda8: st/funcs.h) */
void func_8004ee00(Vis88 *e);
void func_8004ee48(int index, int mode);

/* src/st/vis_b.c (func_8004fc94, func_8004fd28, func_800500ac: st/funcs.h) */
int func_8004f718(int i, VisLists *req);
void func_8004f974(Vis88 *e, int kind);
Vis88 *func_8004f994(int start, int n, int now);
void func_8004fa38(int key);
int func_8004fb88(Vis88 *e, int id);
void func_8004fc0c(int key);
void func_8004fc84(void);
int func_8004fdd0(Vis88 *src, Vis88 *e, SVECTOR *from, SVECTOR *to, int fade, int steps);
void func_80050170(int dx, int dy, int dz);
Vis88 *func_80050288(Vis88 *src, int kind, int state, int arg3, int size, int angle, int ox,
                     int oy, int oz);
void func_80050468(Vis88 *e, int arg);
void func_80050564(Vis88 *e, int arg);
void func_8005066c(Vis88 *e, int arg);

/* src/st/vis_c.c (func_80052ca4: st/funcs.h; func_80052d10: st/area.h) */
void func_80052894(StWarp *to, int keep);
void func_80052c24(void);
void func_80052c5c(void);
int func_80052d40(int id);
void func_80052d98(u16 *id);
void func_80052e50(Vis48 *c);
int func_80053174(VisLists *req);

/* 0x8005335c: no source yet. */
void func_8005335c(Vis48 *c, int arg1);


#endif
