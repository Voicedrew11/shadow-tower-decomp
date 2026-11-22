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
/* The request fields VisLists starts with, as a stack request (src/st/item.c,
 * func_80053aa4 in src/st/vis_c_run.c). */
typedef struct {
    s32 unk0;
    VECTOR unk4;
    SVECTOR unk14;
    u16 unk1C;
    u16 unk1E;
    s16 unk20;
    s16 unk22;
} VisRequest;

typedef struct {
    s32 unk0;            /* 0x00: the kind of the request func_80049544 takes */
    VECTOR unk4;         /* 0x04: its position */
    SVECTOR unk14;       /* 0x14: its velocity */
    u16 unk1C;           /* 0x1c: reach; added to a table B record radius (func_8004f53c) */
    u16 unk1E;           /* 0x1e: height added while resolving */
    s16 unk20;           /* 0x20: the velocity's heading */
    s16 unk22;           /* 0x22: its horizontal length */
    u8 unk24[0x2c - 0x24];
    s32 unk2C;           /* 0x2c: floor height found; func_80048ad0 resolves it (func_80053aa4) */
    s32 unk30;           /* 0x30: a height func_8003bdb0 tests */
    s32 unk34;           /* 0x34: func_80048ad0's heights (unk2C..unk58) */
    s32 unk38;
    s32 unk3C;
    s32 unk40;
    s32 unk44;
    s32 unk48;
    s32 unk4C;           /* 0x4c: 0x80000 when unset (func_8003be10) */
    s32 unk50;
    s32 unk54;
    s32 unk58;           /* 0x58: 0x80000 when unset */
    s32 unk5C;           /* 0x5c: an index into vis_table_a.entries */
    s32 unk60;           /* 0x60: func_8004be80's result */
    s32 unk64;           /* 0x64: an index into vis_table_b.entries */
    u8 unk68[0xc];
    s32 unk74;           /* 0x74: the kind bits func_80049544 resolved */
    s32 unk78;           /* 0x78: heading of the wall slid along */
    SVECTOR unk7C;       /* 0x7c: func_80048ad0's normals (unk7C..unkA4) */
    SVECTOR unk84;
    s16 unk8C;           /* 0x8c..0x90: wall normal (bit 1 hits) */
    u8 unk8E[2];
    s16 unk90;
    u8 unk92[2];
    SVECTOR unk94;
    s16 unk9C;           /* 0x9c..0xa0: wall normal (bit 3 hits) */
    u8 unk9E[2];
    s16 unkA0;
    u8 unkA2[2];
    SVECTOR unkA4;
    VECTOR unkAC;        /* 0xac: the resolved source's position */
    s32 unkBC;           /* 0xbc: its horizontal distance */
    VECTOR unkC0[272];   /* 0xc0: collision vertices in world space (src/st/vis_hit.c) */
    VECTOR unk11C0[272]; /* 0x11c0..: the same relative to the three test points */
    VECTOR unk22C0[272];
    VECTOR unk33C0[272];
    u16 unk44C0;         /* 0x44c0 */
    u16 unk44C2;         /* 0x44c2 */
    u8 unk44C4[2];
    s16 unk44C6;         /* 0x44c6: push along the wall */
    s16 unk44C8;         /* 0x44c8: the visible record the viewer stands in, -1: none (func_8004162c) */
    u16 n_sources;       /* 0x44ca */
    u16 n_visible;       /* 0x44cc */
    u16 sources[0x80];   /* 0x44ce */
    u16 visible[0x80];   /* 0x45ce */
    u8 unk46CE[0x47ce - 0x46ce];
    s16 unk47CE;         /* 0x47ce..0x47d2: cleared by player_update_status */
    s16 unk47D0;
    s16 unk47D2;
    u8 unk47D4[0x47d8 - 0x47d4];
    u32 unk47D8;         /* 0x47d8: record index of the floor found */
    s32 unk47DC;         /* 0x47dc: cleared by func_8003c054 */
    u32 unk47E0;         /* 0x47e0: flag bits func_80044404 tests */
    s32 unk47E4;         /* 0x47e4: floor height (func_80048ad0) */
    s32 unk47E8;         /* 0x47e8: ceiling height */
    u8 unk47EC[4];
} VisLists;

/* 0x8017bed0: 12-byte records indexed by visible id. */
typedef struct {
    u8 part;             /* 0x00: the model part drawn for this record */
    u8 unk01;            /* 0x01: copied to a table A entry's unk06 (func_8004e960) */
    u8 unk02;
    u8 unk03;            /* 0x03: bits 0-1 select the light set */
    u8 unk04;            /* bit 1: flagged by vis_cull_by_range */
    s8 unk05;            /* 0x05, 0x06: FDAT: an offset (x, z) its entries' headings bend by */
    s8 unk06;
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

/* One 40-byte collision part header in the area's collision file
 * (world_state.unk8308[1]); offsets are from the file start + 12. */
typedef struct {
    s32 vertices;        /* 0x00: SVECTOR vertices */
    s32 n_vertices;      /* 0x04 */
    s32 normals;         /* 0x08: SVECTOR normals, by byte offset */
    s32 unkC;
    s32 faces;           /* 0x10: face blocks */
    s32 unk14;
    u16 n18;             /* 0x18: 20-byte records before the faces */
    u16 n1a;             /* 0x1a: 24-byte records */
    u16 n1c;             /* 0x1c: 28-byte records */
    u16 n1e;             /* 0x1e: 32-byte records */
    u16 n_tris;          /* 0x20: VisHitTri faces */
    u16 unk22;
    u16 n_quads;         /* 0x24: VisHitQuad faces after the triangles */
    u16 unk26;
} VisHitPart;

/* A collision triangle: a push direction, the face flags, its normal and
 * vertices (SVECTOR byte offsets). */
typedef struct {
    u8 x;
    u8 z;
    u16 flags;           /* 1 floor, 2 wall, 4 ceiling, 0x10..0x800 floor kinds */
    u16 normal;
    u16 v[3];
} VisHitTri;

typedef struct {
    u8 x;
    u8 z;
    u16 flags;
    u16 normal;
    u16 v[4];
    u16 pad;
} VisHitQuad;

/* The scratchpad while a request is tested (src/st/vis_hit.c). */
typedef struct {
    u8 *base;            /* 0x000: the collision file */
    VisHitPart *part;    /* 0x004 */
    u8 *normals;         /* 0x008 */
    u8 *vertices;        /* 0x00c */
    VECTOR *world;       /* 0x010: vis_lists.unkC0 cursor */
    VECTOR *rel[3];      /* 0x014: unk11C0..unk33C0 cursors */
    u16 unk20;           /* 0x020: the record's light set */
    s16 hit;             /* 0x022: the record hit, -1 none */
    VECTOR e[3][4];      /* 0x024: per point, the face edges' outer products */
    VECTOR pos[3];       /* 0x0e4: the test points */
    s32 h114;            /* 0x114: nearest floor */
    s32 h118;
    s32 h11c;
    s32 h120;            /* 0x120: nearest ceiling */
    s32 h124;
    s32 h128;            /* 0x128: nearest wall */
    s32 h12c;
    s32 h130;
    s32 h134;
    s32 h138;
    s32 h13c;
    s32 h140;
    s32 h144;
    SVECTOR dir;         /* 0x148: the request's reach */
    SVECTOR n150;
    SVECTOR n158;
    SVECTOR n160;
    SVECTOR n168;
    SVECTOR n170;
    SVECTOR n178;
    SVECTOR n180;
    SVECTOR n188;
    SVECTOR v190;
    u32 flags;           /* 0x198: floor kind bits */
} VisHitScratch;

#define VIS_HIT_SCRATCH ((VisHitScratch *)0x1f800000)

/* 0x8008042c: the four light-set rotations (load data, WIP owner). */
extern MATRIX DAT_8008042c[4];

extern VisLists vis_lists;
extern VisRecord vis_records[];
extern VisSourceMask vis_source_masks[];
/* 0x801856d0: per axis (x, y, z) and 4096-unit cell, the sources that cover it. */
extern VisSourceMask DAT_801856d0[3][128];

/* 0x801ba138: 64 entries of 212 bytes, then the collected index list. */
/* A flags word whose byte +2 is also updated on its own (world_draw_creatures). */
typedef union {
    u32 word;
    u8 bytes[4];
} VisFlags;

/* A table A record's script: a command stream (WIP). */
typedef struct {
    u8 unk00;            /* 0x00: kind; 'P' and '@' are picked by func_8004def8 */
    u8 unk01;            /* 0x01: the FDAT overlays copy it to Vis212.unk08 */
    u8 unk02;
    u8 unk03;            /* 0x03: an FDAT overlay squares it into a range */
    u8 unk04;            /* 0x04: an FDAT overlay's pick weight */
    u8 unk05;            /* 0x05: FDAT sound bank offset; 0xff none */
    u8 unk06[2];
    u32 unk08;           /* 0x08: FDAT: the Vis212.unk44 states it may run in */
    u16 unk0C;           /* 0x0c */
    u16 unk0E;           /* 0x0e: FDAT sound trigger: kind (0xf000) and frame/period */
    /* 0x10: FDAT command arguments; signed or unsigned by command (WIP) */
    union {
        s16 s[16];
        u16 u[16];
        u8 b[32];
        s32 w[8];
        struct {             /* FDAT attack commands (fdat_801ebc88) */
            u16 unk10[2];
            u16 frame[3];    /* 0x14: frame of each hit, 0xffff none */
            s16 unk1A;       /* 0x1a: passed to func_8004cee8 */
            u16 vertex[3];   /* 0x1c: model vertex of each hit, 0xffff the origin */
            u16 node[3];     /* 0x22: skeleton node of each hit */
            s16 offset[3];   /* 0x28: offset added after the turn */
        } hit;
    } arg;
} VisScript;

typedef struct {
    u8 unk00;            /* 0x00: 0xff when free (func_8004afdc) */
    u8 unk01;
    u8 unk02;            /* 0x02: index of its 192-byte VisTableA record */
    u8 unk03;
    u8 unk04;
    u8 unk05;            /* 0x05: state (func_8004b2dc); 1 active, 5 dying (func_8004def8) */
    u8 unk06;
    u8 unk07;            /* 0x07: area id, 0xfd: the previous area (func_8004e2d8) */
    u8 unk08;            /* 0x08 */
    u8 unk09;
    u8 unk0A;
    u8 unk0B;            /* 0x0b: model slot - 128 */
    u8 unk0C;            /* 0x0c: block tag */
    u8 unk0D;            /* 0x0d: added to vis_table_a.unk6128 while live */
    u8 unk0E;
    u8 unk0F;
    u16 unk10;           /* 0x10..0x14: the three items it drops: table C ids (func_8004e960) */
    u16 unk12;
    u16 unk14;
    s16 unk16;           /* 0x16: vertical speed */
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
    u16 unk3A;           /* 0x3a: the FDAT overlays' last sound id */
    u32 unk3C;           /* 0x3c: squared distance */
    s32 unk40;           /* 0x40: horizontal distance */
    s32 unk44;
    u8 unk48;
    u8 unk49;            /* 0x49: 106 + the table B entry func_800549c8 spawned */
    u8 unk4A[2];
    s32 unk4C;           /* 0x4c: height above the floor */
    s32 unk50;           /* 0x50: request kind bits */
    u16 unk54;
    u16 unk56;           /* 0x56: facing the viewer is compared with; heading to the viewer */
    u16 unk58;           /* 0x58: its difference from the facing */
    s16 unk5A;           /* 0x5a: FDAT: a second heading, set with unk5C */
    s16 unk5C;           /* 0x5c: FDAT: the heading an entry turns to */
    u16 unk5E;           /* 0x5e: heading of the wall slid along */
    u16 unk60;
    u16 unk62;           /* 0x62: FDAT: 1 or 2 masks a range bit of unk60 for a frame */
    u8 unk64;            /* 0x64: set when hit */
    u8 unk65;            /* 0x65..0x67: percent chances of unk10..unk14 */
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
    SVECTOR unk9C;       /* 0x9c: velocity: this frame's movement */
    u32 unkA4;           /* 0xa4: from here the anim block objects.c hands model_draw (&unkA4) */
    VisScript *unkA8;    /* 0xa8: the running script, an entry of its record's unk60 list */
    VisScript *unkAC;    /* 0xac: FDAT: the script a 17/20 command returns to */
    s16 unkB0;           /* 0xb0: magnitude of the last unk2A step; set from the script's unk0C */
    u16 unkB2;
    u8 unkB4[8];
    s16 unkBC;           /* 0xbc: cleared by the FDAT overlays */
    s16 unkBE;
    u8 unkC0[0x14];
} Vis212;

/* An entry of a Vis192 record's unk60 list (func_8004b1e8). */
typedef struct {
    u8 unk0;             /* 0x0: id */
    u8 unk1;             /* 0x1: animation (Vis212.unk08) */
    u8 unk2[3];
    u8 unk5;             /* 0x5: sound */
    u8 unk6[6];
    s16 unkC;            /* 0xc: step of Vis212.unk2A per frame */
} VisAnim;

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
    u8 unk09;            /* 0x09: its XA stream file (src/st/xa.c) */
    u8 unk0A[4];
    s16 unk0E;           /* 0x0e: gravity */
    u16 unk10;           /* 0x10: map bit, 0xffff none */
    u16 unk12;           /* 0x12: added to vis_table_a.unk612C while live */
    u16 unk14;           /* 0x14: FDAT: near range (squared << 16 against Vis212.unk3C) */
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
    VisScript *unk60[24]; /* 0x60: null-terminated (func_8004b1e8); offsets from unk3500[16] until func_8004e8ec */
} Vis192;

typedef struct {
    Vis212 entries[64];
    Vis192 unk3500[58];
    u8 unk6080[0x80];
    u8 count;            /* 0x6100 */
    u8 unk6101;
    u8 unk6102;
    u8 unk6103;          /* 0x6103: the previous area id */
    u16 list[12];        /* 0x6104 */
    u8 unk611C[0xc];
    s32 unk6128;         /* 0x6128: sum of the live entries' unk0D */
    s32 unk612C;         /* 0x612c: sum of their records' unk12 */
    s32 unk6130;
    Vis192 *unk6134;     /* 0x6134: the current record */
    Vis212 *unk6138;     /* 0x6138: the current entry */
    Vis192 *unk613C;     /* 0x613c: the record of unk6140 */
    Vis212 *unk6140;     /* 0x6140: the entry the current one follows */
    s32 unk6144;         /* 0x6144: loop index func_800402ac keeps in memory */
    s32 unk6148;         /* 0x6148: the current entry's record index */
    s32 unk614C;         /* 0x614c: active entries counted this frame */
    s32 unk6150;         /* 0x6150: frame counter */
    u8 unk6154[4];
    u16 unk6158[4];      /* 0x6158: the last four area ids */
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
    u8 unk00;            /* 0x00: 2 skips the entry (world_draw_objects) */
    u8 unk01;
    u8 unk02;            /* 0x02 */
    u8 unk03;            /* 0x03: bits 6/7 near and far range */
    u8 unk04;
    u8 unk05;
    u8 unk06;
    u8 unk07;
    u16 unk08;
    u16 unk0a;           /* 0x0a: index of its 24-byte VisTableB record */
    u16 unk0C;           /* 0x0c: state of a moving entry (func_8004fdd0); 0xff once opened (src/st/save.c) */
    u16 unk0E;
    u16 unk10;           /* 0x10 */
    u16 unk12;           /* 0x12: height */
    s16 unk14;           /* 0x14: OT depth bias */
    s16 unk16;           /* 0x16 */
    u32 unk18;           /* 0x18: squared distance */
    s32 unk1C;           /* 0x1c: distance (sound type 236) */
    u16 unk20;           /* 0x20: facing */
    u16 unk22;           /* 0x22: a door's opening angle (src/st/item.c) */
    VECTOR unk24;        /* 0x24: position */
    u16 unk34;           /* 0x34: rotation */
    s16 unk36;           /* 0x36: yaw */
    u16 unk38;
    u8 unk3A[2];
    SVECTOR unk3C;       /* 0x3c: scale */
    u8 unk44[4];         /* 0x44 */
    u8 unk48;            /* 0x48: sound area width (2048 units) */
    u8 unk49;            /* 0x49: sound area depth */
    /* 0x4a: by kind: a sound source's sound (src/st/objects.c) or the record
     * swap and open state of a door (src/st/save.c) */
    union {
        struct {
            u8 unk4A;    /* 0x4a: sound id */
            u8 unk4B;    /* 0x4b: volume */
            u8 unk4C;    /* 0x4c: fade range */
            u8 unk4D;    /* 0x4d: sound flags */
            u16 unk4E;   /* 0x4e: repeat period */
        } snd;
        struct {
            u16 unk4A;   /* 0x4a: first record of one swap range */
            u16 unk4C;   /* 0x4c: first record of the other */
            u8 unk4E;    /* 0x4e: records to swap */
            u8 unk4F;    /* 0x4f: state, 0xff idle */
        } door;
    } unk4A;
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
    u16 unk0A;           /* 0x0a: the entry's OT depth bias (func_8004eebc) */
    u16 unk0C;           /* 0x0c */
    u16 unk0E;           /* 0x0e */
    u8 unk10[2];
    u16 unk12;           /* 0x12: which side a door swings the viewer from (func_80050768) */
    u8 unk14[4];
} Vis24;

typedef struct {
    Vis24 unk0000[330];
    u8 unk1EF0[0x30];
    Vis88 entries[240];
    u8 unk71a0[4];
    s16 unk71A4;         /* 0x71a4..0x71a8: cleared by player_update_status */
    s16 unk71A6;
    s16 unk71A8;
    u8 unk71AA[2];
    Vis24 *unk71AC;      /* 0x71ac: the record of the entry func_80050768 is updating */
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
 * bytes from +0x118 (game_load_start_file reads byte +4 of each), then 64 entries
 * of 48 bytes at +0x3710.
 */
typedef struct {
    u8 unk0[2];
    u8 unk2;             /* 0x02: carry limit or price base (src/st/menu_shop.c) */
    u8 unk3;             /* 0x03 */
    u8 unk4;             /* 0xff: none (game_load_start_file); func_8003f12c's model */
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
    u8 unk0000[6];       /* [0]: drops made; [2], [3]: slots used of entries[32..47], [48..63] */
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
/* src/st/vis_hit.c */
void func_80046604(VECTOR *pos);
void func_800468c0(VECTOR *pos, int kind);
void func_80047884(VECTOR *pos, int kind);
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

/* src/st/vis_b_load.c */
void func_8004eebc(u8 *src);

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

/* One 24-byte table C entry of an area file. */
typedef struct {
    u16 unk00;           /* 0x00: Vis44 index, 0xffff none */
    u16 unk02;
    u8 unk04;            /* 0x04..0x06: cell */
    u8 unk05;
    u8 unk06;
    u8 unk07;
    s16 unk08;           /* 0x08..0x0c: offset in the cell */
    s16 unk0A;
    s16 unk0C;
    u16 unk0E;           /* 0x0e..0x12: rotation */
    u16 unk10;
    u16 unk12;
    u8 unk14[2];
    u16 unk16;
} VisCFileEntry;

/* src/st/vis_c.c (func_80052ca4: st/funcs.h; func_80052d10: st/area.h) */
void func_80052f30(VisCFileEntry *src);
void func_80052894(StWarp *to, int keep);
void func_80052c24(void);
void func_80052c5c(void);
int func_80052d40(int id);
void func_80052d98(u16 *id);
void func_80052e50(Vis48 *c);
int func_80053174(VisLists *req, int unused);

/* 0x8005335c: no source yet. */
void func_8005335c(Vis48 *c, int arg1);


#endif
