/*
 * VS.EXE menu screens: the scratchpad block they share and the message
 * table. The record types are ST.EXE's (st/menu.h); VS's scratchpad block
 * places the colours and rectangles lower than ST's MenuScratch.
 */
#ifndef VS_MENU_H
#define VS_MENU_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/menu.h>

/* The menu screens' scratchpad block; only proven fields are spelled. */
typedef struct {
    MenuState unk0;      /* 0x000 */
    u8 unk5C[0x1c6 - 0x5c];
    u16 unk1C6;          /* 0x1c6 (ST MenuScratch.unk202) */
    u8 unk1C8[4];
    u16 unk1CC;          /* 0x1cc (ST unk2DC) */
    u8 unk1CE[0x1f0 - 0x1ce];
    CVECTOR style[8];    /* 0x1f0: text colours (ST 0x300) */
    u8 unk210[0x10];
    CVECTOR unk220;      /* 0x220 (ST unk338) */
    RECT unk224;         /* 0x224 (ST unk33C) */
    CVECTOR unk22C;      /* 0x22c (ST unk344) */
    RECT unk230;         /* 0x230 (ST unk348) */
    RECT unk238;         /* 0x238 (ST unk350) */
    DVECTOR unk240;      /* 0x240: texture position in VRAM (ST unk358) */
    DVECTOR unk244;      /* 0x244: CLUT position in VRAM (ST unk35C) */
} VsMenuScratch;

#define VS_MENU_SCRATCH ((VsMenuScratch *)0x1f800000)

/* The message table at 0x80196230 (bss), as ST.EXE's menu_messages. */
extern MenuMessages DAT_80196230;

#define VS_MESSAGE(i) ((u16 *)(DAT_80196230.text + DAT_80196230.offsets[i]))

/* VS's copies of ST.EXE's text and primitive helpers (src/st/ui.c,
 * src/st/text.c), not reconstructed yet. */
void func_8001d8b8(u16 *s, u16 x, u16 y, CVECTOR *color); /* ST func_8002732c */
void func_8001e3d8(RECT *r, CVECTOR *color, u16 semi);   /* ST func_80028220 */
void func_8001e530(RECT *r, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int mode); /* ST func_80028364 */
void func_8001e890(int abr, int x, int y);               /* ST func_800286a8 */
void func_8001f23c(int x, int y, int w, int h);          /* ST func_80029548 */

#endif
