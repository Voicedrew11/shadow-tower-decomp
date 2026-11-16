/*
 * ST.EXE graphics state at 0x8015d090 (bss): the two DRAWENVs the frame
 * alternates between, the primitive work buffers and the screen fade
 * channels. Field names stay address-derived until their behaviour is
 * proven.
 *
 * Band C's include/st/mesh.h holds a view of the same object (Dat8015d090
 * with tint_level at 0x1edf8 and render_mode at 0x1edfc); this definition
 * keeps those two members at the same offsets so the views can be merged.
 */
#ifndef ST_GFX_H
#define ST_GFX_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/* A primitive allocation buffer: the next free byte runs up to `end`. */
typedef struct {
    u8 *unk0;
    u8 *end;
    u8 *next;
} StPrimBuffer;

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

typedef struct {
    DRAWENV draw[2];              /* 0x0 */
    u8 unkB8[0xe0 - 0xb8];
    u_long *unkE0;                /* 0xe0: CD load buffer, also a primitive area */
    u8 unkE4[0x80e4 - 0xe4];
    u_long *ot;                   /* 0x80e4 */
    u8 unk80E8[0x8304 - 0x80e8];
    StPrimBuffer *prims;          /* 0x8304 */
    u8 unk8308[0x830c - 0x8308];
    void *unk830C[6];             /* 0x830c: heap block owners (func_800239ec) */
    s32 unk8324;                  /* 0x8324: primitives added by func_80021518 */
    s32 unk8328;                  /* 0x8328: primitives allocated by func_80021518 */
    u8 unk832C[0x834c - 0x832c];
    void *unk834C[8];             /* 0x834c: heap block owners (func_800237a8); WIP count */
    u8 unk836C[0x1ecc2 - 0x836c];
    u8 frame;                     /* 0x1ecc2: index of the current DRAWENV */
    u8 unk1ECC3;
    u8 unk1ECC4;                  /* 0x1ecc4: set while the loading display runs */
    u8 unk1ECC5[0x1ecd6 - 0x1ecc5];
    StFade fade;                  /* 0x1ecd6 */
    u8 unk1ECE6[0x1edf8 - 0x1ece6];
    int tint_level;               /* 0x1edf8 (band C) */
    u8 render_mode;               /* 0x1edfc (band C) */
    u8 unk1EDFD[3];
} Dat8015d090;

extern Dat8015d090 DAT_8015d090;

/* src/st/gfx.c */
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
