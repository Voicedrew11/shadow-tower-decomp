/*
 * VS.EXE menu screens: the scratchpad block they share and the message
 * table. The record types are ST.EXE's (st/menu.h); VS's scratchpad block
 * places the colours and rectangles lower than ST's MenuScratch.
 */
#ifndef VS_MENU_H
#define VS_MENU_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/menu.h>
#include <vs/card.h>

/* The menu screens' scratchpad block; only proven fields are spelled. */
typedef struct {
    union {
        MenuState menu;  /* the menu screens' state */
        VsCardState card; /* the memory card screens' state (vs/card.h) */
    } unk0;              /* 0x000 */
    VsCardPort *unk1B8;  /* 0x1b8: the card slot being scanned */
    s16 unk1BC;          /* 0x1bc: its card port */
    s16 unk1BE;          /* 0x1be: the save file count, for the file list */
    u16 unk1C0;          /* 0x1c0 */
    s16 unk1C2;          /* 0x1c2: the file list cursor row, -1 on cancel */
    s16 unk1C4;          /* 0x1c4: the file list scroll */
    u16 unk1C6;          /* 0x1c6 (ST MenuScratch.unk202) */
    s16 unk1C8;          /* 0x1c8: the first free save number */
    u8 unk1CA[2];
    u16 unk1CC;          /* 0x1cc (ST unk2DC) */
    u8 unk1CE[0x1d4 - 0x1ce];
    u8 unk1D4;           /* 0x1d4: cursor blink phase (ST unk2E4) */
    u8 unk1D5[0x1f0 - 0x1d5];
    CVECTOR style[8];    /* 0x1f0: text colours (ST 0x300) */
    u8 unk210[4];
    CVECTOR unk214;      /* 0x214: highlight colour (ST unk32C) */
    RECT unk218;         /* 0x218: highlight bar (ST unk330) */
    CVECTOR unk220;      /* 0x220 (ST unk338) */
    RECT unk224;         /* 0x224 (ST unk33C) */
    CVECTOR unk22C;      /* 0x22c (ST unk344) */
    RECT unk230;         /* 0x230 (ST unk348) */
    RECT unk238;         /* 0x238 (ST unk350) */
    DVECTOR unk240;      /* 0x240: texture position in VRAM (ST unk358) */
    DVECTOR unk244;      /* 0x244: CLUT position in VRAM (ST unk35C) */
    u16 unk248;          /* 0x248: blink offset (ST unk360) */
} VsMenuScratch;

/* Layout check: the offset the code proves. */
typedef char VsMenuScratchUnk1B8At0x1B8[offsetof(VsMenuScratch, unk1B8) == 0x1b8 ? 1 : -1];
typedef char VsMenuScratchStyleAt0x1F0[offsetof(VsMenuScratch, style) == 0x1f0 ? 1 : -1];
typedef char VsMenuScratchUnk244At0x244[offsetof(VsMenuScratch, unk244) == 0x244 ? 1 : -1];

#define VS_MENU_SCRATCH ((VsMenuScratch *)0x1f800000)

/* The message table at 0x80196230 (bss), as ST.EXE's menu_messages. */
extern MenuMessages DAT_80196230;

#define VS_MESSAGE(i) ((u16 *)(DAT_80196230.text + DAT_80196230.offsets[i]))

/* VS's copies of ST.EXE's text and primitive helpers (src/st/ui.c,
 * src/st/text.c), not reconstructed yet. */
void func_8001d8b8(u16 *s, u16 x, u16 y, CVECTOR *color); /* ST func_8002732c */
void func_800204b0(void);                                /* the menu screens' frame: draws and flips */
void func_8001e3d8(RECT *r, CVECTOR *color, u16 semi);   /* ST func_80028220 */
void func_8001e530(RECT *r, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int mode); /* ST func_80028364 */
void func_8001e890(int abr, int x, int y);               /* ST func_800286a8 */
void func_8001f23c(int x, int y, int w, int h);          /* ST func_80029548 */
void func_8001f30c(int x, int y, int w, int h);
void func_8001f828(void);                                /* ST func_8002c9e8 */
void func_8001f8c4(void);
void func_8001f960(void);                                /* ST func_8002c050 */
void func_8001f9a0(void);
void func_8001fb98(void);                                /* ST func_8002c1a4 */
void func_8001fcac(void);
void func_80020384(void);                                /* ST func_8002c8bc */
void func_80020420(void);                                /* ST func_8002c958 */
void func_80020460(void);                                /* ST func_8002c998 */
void func_80020aa4(int page);
void func_80021140(int page);                            /* ST func_8003025c */
void func_80021cfc(void);
void func_800222d8(int page);
void func_80022794(void);                                /* ST func_800319a8 */
void func_80022a00(void);
void func_80022f14(int page);
void func_80023114(int page);
void func_8002326c(void);

#endif
