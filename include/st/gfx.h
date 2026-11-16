/*
 * ST.EXE graphics state at 0x8015d090 (world_state) and the functions of
 * src/st/view.c, texanim.c, gfx.c, prim.c, quad.c and fade.c. Unmodeled ranges stay opaque;
 * address-derived names stay until behaviour is proven.
 */
#ifndef ST_GFX_H
#define ST_GFX_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/* One animated-texture slot (see src/st/texanim.c). */
typedef struct {
    u8 type;
    u8 period;
    u8 countdown;
    u8 step;
    s16 area;
    s16 phase;
    RECT dst;
    RECT src;
} StTexAnim;

#define ST_TEXANIM_SLOTS 16

/*
 * A 0x6c-byte lighting environment: a light colour matrix (the first 18
 * bytes of a MATRIX, which gte_SetColorMatrix loads), the back colour and
 * two halfwords copied to the scratchpad.
 */
/* A light direction matrix as gte_SetLightMatrix loads it (five words). */
typedef struct {
    short m[3][3];
    short pad;
} StLightMatrix;

typedef struct {
    StLightMatrix light[4]; /* 0x00: the world renderers pick one per part */
    short color[3][3];
    u8 unk62[2];
    u8 back[3];
    u8 unk67;
    s16 unk68;
    s16 unk6A; /* also vis_cull_by_range's far range (/ 2048) */
} StLight;

/* One packet buffer: its start, its end and the next free word (primitives
 * are allocated from `cur` up to `end`). */
typedef struct {
    u_long *start;
    u_long *end;
    u_long *cur;
} StPacketBuffer;

/*
 * The graphics state: double-buffered drawing and display environments,
 * ordering tables and packet buffers, the animated-texture slots, the
 * camera matrices, the mesh renderers' render mode and the current
 * lighting environment.
 */
/*
 * The screen fade channels func_80021994 accumulates and func_8002181c /
 * func_800218d8 draw and clear once per frame: index 0 for modes 1 and 3,
 * index 1 for mode 2. `mode` selects the semi-transparency rate.
 */
typedef struct {
    u8 active[2];
    u8 mode[2];
    u16 r[2];
    u16 g[2];
    u16 b[2];
} StFade;

/* One 40-byte model part: its primitive offset and per-kind counts (the
 * records st/mesh.h's MeshHeader views from 12 bytes earlier). */
typedef struct {
    u32 unk0;
    u32 offset;
    u32 unk8;
    u16 counts[8];
    u8 unk1C[0xc];
} ModelPart;

typedef struct {
    u8 unk0[8];
    u32 parts_count;
    u8 unkC[0xc];
    ModelPart parts[1];
} Model;

typedef struct {
    DRAWENV draw[2];
    DISPENV disp[2];
    u_long *unkE0;       /* 0xe0: CD load buffer, also a primitive work area */
    u_long ot[2][4096];
    u_long *ot_cur;      /* 0x80e4 */
    u_long ot2[2][64];
    u_long *ot2_cur;
    StPacketBuffer packets[2];
    StPacketBuffer *packet; /* 0x8304 */
    void *unk8308[4];    /* 0x8308: primitive-pointer slots (func_80017c5c) */
    void *unk8318;
    s32 unk831C[2];
    s32 unk8324;         /* 0x8324: primitives added by func_80021518 */
    s32 unk8328;         /* 0x8328: primitives allocated by func_80021518 */
    s32 unk832C;
    s32 unk8330;
    s32 unk8334;
    s32 unk8338[5];
    void *unk834C[8];    /* 0x834c: heap block owners: func_800237a8 indexes them by slot,
                          * func_800239ec and src/st/slots.c by slot - 16; WIP count */
    u8 unk836C[0xab3c - 0x836c];
    u8 world_vertices[0x1eb40 - 0xab3c]; /* 0xab3c: the world parts' transformed vertices (MeshVertex); WIP extent */
    StTexAnim texanim[ST_TEXANIM_SLOTS];
    u8 unk1ECC0;
    u8 unk1ECC1;
    u8 frame;            /* 0x1ecc2: index of the current DRAWENV and buffers */
    u8 unk1ECC3;
    u8 unk1ECC4;         /* 0x1ecc4: set while the loading display runs */
    u8 unk1ECC5[8];
    u8 unk1ECCD;
    u8 unk1ECCE;
    u8 unk1ECCF;
    u8 unk1ECD0[2];
    u8 unk1ECD2;
    u8 unk1ECD3[3];
    StFade fade;         /* 0x1ecd6 */
    u8 unk1ECE6[2];
    MATRIX unk1ECE8;
    u8 unk1ED08[0x20];
    MATRIX unk1ED28[4];  /* 0x1ed28: one per light set (ScratchPad.light_index) */
    MATRIX unk1EDA8;
    VECTOR unk1EDC8;
    SVECTOR unk1EDD8;
    u8 unk1EDE0[0x10];
    s32 unk1EDF0;
    s32 unk1EDF4;
    int tint_level;      /* 0x1edf8: the mesh renderers' tint strength */
    u8 render_mode;      /* 0x1edfc: MESH_* bits (st/mesh.h) */
    u8 unk1EDFD;
    u8 unk1EDFE;
    u8 unk1EDFF;
    u_long unk1EE00;     /* 0x1ee00: vis_cull_by_range's near range (<< 11) */
    u_long unk1EE04;     /* 0x1ee04: its middle range (<< 11) */
    StLight *light;      /* 0x1ee08: the current lighting environment */
    u8 unk1EE0C[9];
    u8 unk1EE15;
    u8 unk1EE16;
    /* 0x1ee16..0x1ee19 (src/st/text.c): title-card state, mode, fade length, frame counter */
    u8 unk1EE17;
    u8 unk1EE18;
    u8 unk1EE19;
} WorldState;

extern WorldState world_state;

/* The two 0x20000-byte GPU packet buffers. */
extern u_long DAT_8011c488[];
/* 128 animated objects of 24 bytes; id 0xffff marks a free one. */
typedef struct {
    u16 unk0;            /* 0x00: model id - 40; bit 15 hides it */
    u8 unk2;             /* 0x02: bits 0-5 kind, bits 6-7 a rotation */
    u8 unk3;             /* 0x03: frame count */
    u8 unk4;             /* 0x04: frame period, 0 still */
    u8 unk5;             /* 0x05: frame */
    u8 unk6[2];
    VECTOR unk8;         /* 0x08: position */
} StObject24;

/*
 * The frame counter func_80016fb8 clears and func_800255f0 advances, then
 * the animated objects it steps. -G8 code reads the counter with lui, so it
 * is not a small object of its own.
 */
typedef struct {
    s32 frame;
    StObject24 objects[128];
} StAnimObjects;
extern StAnimObjects DAT_8015c488;
/* The CLUT a type-49 texture slot cycles (entries 1-14 rotate). */
extern u16 DAT_8009ac88[16];
/* The three lighting blend targets of func_80017d28. */
extern StLight DAT_8018ca38[3];

/* src/st/view.c */
int func_80016554(void);

/* src/st/texanim.c */
void func_80016878(void);
StTexAnim *func_800168a4(void);
void func_800168e0(RECT *rect, int period, int step, int type, short area);
void func_80016a14(void);

/* src/st/gfx.c */
void func_80016fb8(void);
void func_800171c8(void);
void func_800172bc(void);
void func_800173a8(void);
void func_800174c4(void);
void func_800175a4(void);
void func_80017634(void);
void func_80017680(void);
void func_800176a8(VECTOR *pos, SVECTOR *rot);

/* src/st/prim.c (the FT3 tint pair is declared with the other renderers'
 * in st/mesh.h) */
void func_800177a0(Model *model, DVECTOR *uv, DVECTOR *clut, u8 count);
void func_80017c5c(u16 index, void *p);
void func_80017c8c(u16 index, void *p);
u_char *func_80017cb0(u16 offset);
void func_80017cc4(u16 i0, u16 i1, u16 i2);
void func_80017d28(u32 t, int flags);

/* src/st/quad.c */
void func_80020ea8(int x0, int y0, int x1, int y1, u8 u, u8 v, u8 w, u8 h, u8 semi,
                   u16 tpage, u16 clut, u8 r, u8 g, u8 b);
void func_80020fcc(int x0, int y0, int x1, int y1, int u, int v, int w, int h, u8 semi,
                   int tpage, int clut, int r, int g, int b);
void func_800210f4(void);
void func_80021158(void);
void func_800214a4(void);
void func_800214cc(void);
/* An old-style definition; this is its promoted prototype. */
void func_80021518(int x0, int y0, int x1, int y1, int u, int v, int w, int h, int semi,
                   int tpage, int clut, int r, int g, int b, int depth);
void func_80021654(void);

/* src/st/fade.c */
void func_8002181c(void);
void func_800218d8(void);
void func_80021994(int r, int g, int b, int scale, int mode);

#endif
