/*
 * ST.EXE font glyphs, text and 2D menu primitives (src/st/text.c, WIP).
 */
#ifndef ST_TEXT_H
#define ST_TEXT_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

/* .sbss words the glyph drawer reads with lui; their owner is unknown. */
extern u16 DAT_800881a0;
extern u16 DAT_800881a2;
extern u16 DAT_800881ac;
extern u16 DAT_800881ae; /* set by func_80031aec around the grid columns */

/* src/st/text.c: the text clip window (.sbss, $gp-relative; also read at 0x8002c6d0) */
extern s16 DAT_800881ba;
extern s16 DAT_800881bc;
extern s32 DAT_800881c0;
extern s32 DAT_800881c4;
extern s32 DAT_800881c8;

/* Read by the title-card overlay; owners unknown. */
extern u16 DAT_80088190;
extern u8 DAT_801cd124;

void func_80026ecc(u16 c, u16 x, u16 y, CVECTOR *color);
void func_8002732c(u16 *s, u16 x, u16 y, CVECTOR *color);
void func_80027440(u32 num, u16 x, u16 y, CVECTOR *color);
void func_800276bc(u32 num, u16 x, u16 y, CVECTOR *color);
void func_80027900(DVECTOR *pos, CVECTOR *color, u16 dir);
void func_80027d18(RECT *r, CVECTOR *color, u16 border, u16 semi);
void func_80028220(RECT *r, CVECTOR *color, u16 semi);
void func_80028364(RECT *r, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int mode);
void func_800286a8(int abr, int x, int y);
void func_80028784(void);

void func_80028c2c(DVECTOR *xy, CVECTOR *rgb, int semi);
void func_80028db8(RECT *r, CVECTOR *mid, CVECTOR *edge, int semi);
void func_8002916c(RECT *r, CVECTOR *color, u16 dir);
void func_8002948c(void);
void func_80029548(int x, int y, int w, int h);
void func_80029618(short x, short y, short w, short h);

#endif
