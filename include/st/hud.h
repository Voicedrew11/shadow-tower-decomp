/*
 * ST.EXE HUD sprite records and the per-frame world update (src/st/hud.c),
 * and the world object passes the frame runs.
 */
#ifndef ST_HUD_H
#define ST_HUD_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/objects.h>

/*
 * One 20-byte HUD sprite record: flags (bit 0 textured, bit 1 flat, bits 2
 * and 3 tint the colour; 0xff ends a run), the semi-transparency rate
 * (0xff opaque), the texture rectangle, the screen rectangle (width 0xffff
 * draws at the texture size), CLUT, texture page and OT depth.
 */
typedef struct {
    u8 flags;
    u8 abr;
    u8 u;
    u8 v;
    u8 uw;
    u8 vh;
    RECT r;
    u16 clut;
    u16 tpage;
    s16 otz;
} StHudSprite;

extern StHudSprite hud_sprites[];

/* Nonzero while the HUD is shown. */

/* src/st/hud.c */
void func_80025c24();
void func_80025e00();
void func_80026020();
void func_80026228(int value, int first, u8 width);
void func_80026508(void);
void func_80026964(u8 first);
void func_80026a88(short value);
void func_80026afc(VECTOR *pos, SVECTOR *rot);
/* Returns int as far as its callers can tell: FDAT_272's func_801eec74 only
 * matches when this call has a value (reload then keeps v0 free). Nothing
 * uses the result. */
int func_80026dd4(VECTOR *pos, SVECTOR *rot, int arg);
void func_80026e04(void);
void func_80026e90(void);

#endif
