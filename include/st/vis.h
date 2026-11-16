#ifndef ST_VIS_H
#define ST_VIS_H

/*
 * ST.EXE visible-set lists (band 0x8001c100..0x8001cb88, WIP). All objects
 * here are unresolved .bss globals; only the members these functions touch
 * are named, as unkXXX until their meaning is proven.
 */
#include <kf/lib/types.h>
#include <sys/types.h>

/* 0x801b5920: a state block; vis_build_visible reads a halfword at +0 and a
 * byte at +0x1d, vis_collect_table_c a byte at +9. */
typedef struct {
    s16 unk00;
    u8 unk02[7];
    u8 unk09;
    u8 unk0a[0x13];
    u8 unk1d;
    u8 unk1e[0xa];
} AreaState;

/* 0x801b5948: the source id list vis_collect_sources fills and the visible id
 * list vis_build_visible builds from it; vis_record_is_visible/vis_id_is_visible search the
 * visible list. */
typedef struct {
    u8 unk0000[0x44ca];
    u16 n_sources;       /* 0x44ca */
    u16 n_visible;       /* 0x44cc */
    u16 sources[0x80];   /* 0x44ce */
    u16 visible[0x80];   /* 0x45ce */
} VisLists;

/* 0x8017bed0: 12-byte records indexed by visible id. */
typedef struct {
    u8 unk00[4];
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

extern AreaState area_state;
extern VisLists vis_lists;
extern VisRecord vis_records[];
extern VisSourceMask vis_source_masks[];

/* The scratchpad (0x1f800000) as the culling code uses it; the mesh
 * renderers' view of the same memory is MeshScratch (st/mesh.h). */
typedef struct {
    u8 unk000[0x128];
    long x;              /* 0x128 */
    long y;              /* 0x12c */
    long z;              /* 0x130 */
    u8 unk134[0xb4];
    u8 no_plane_test;    /* 0x1e8 */
    u8 unk1e9;
    short offset_x;      /* 0x1ea */
    short unk1ec;        /* 0x1ec */
    short offset_z;      /* 0x1ee */
    long plane[6];       /* 0x1f0 */
    long unk208;         /* 0x208 */
} VisScratch;

#define VIS_SCRATCH ((VisScratch *)0x1f800000)

/* 0x80198f28: the viewer block: position at +0x248, angles at +0x268. */
typedef struct {
    u8 unk000[0x248];
    long x;              /* 0x248 */
    long y;              /* 0x24c */
    long z;              /* 0x250 */
    u8 unk254[0x14];
    short unk268;        /* 0x268 */
    short unk26a;        /* 0x26a */
} Viewer;

extern Viewer viewer;

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
    u8 unk71a0[0x14];
    u16 list[96];        /* 0x71b4 */
    u8 count;            /* 0x7274 */
} VisTableB;

/* 0x801c76f8: the index list, then 64 entries of 48 bytes at +0x3710. */
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
    u8 unk0047[0x36c9];
    Vis48 entries[64];   /* 0x3710 */
} VisTableC;

extern VisTableA vis_table_a;
extern VisTableB vis_table_b;
extern VisTableC vis_table_c;

u_long *fill_words(u_long *dst, u_long value, int count);
void vis_collect_sources(u8 *arg, int a1, int a2);
void vis_build_visible(u8 *arg);
int vis_measure_position(void);
void vis_cull_by_range(void);
int vis_record_is_visible(int a, int b, int c);
int vis_id_is_visible(u16 id);
void vis_collect_table_a(void);
void vis_collect_table_b(void);
void vis_collect_table_c(void);

#endif
