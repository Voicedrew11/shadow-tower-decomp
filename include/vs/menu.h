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

/* A fighter team: three character slots, -1 for an empty one (VS band 6). */
typedef struct {
    s32 slot[3];
} VsTeam;

/* The team screen's edit record (a local of func_800182ec): the slot picked
 * (unk4) and a copy of the team being edited. */
typedef struct {
    u8 unk0;             /* 0x00: a slot is held (func_800182ec) */
    u8 unk1[3];
    s32 unk4;            /* 0x04: the picked slot */
    VsTeam team;         /* 0x08 */
} VsTeamEdit;

/* Both players' teams (bss, 0x801624b8). */
extern VsTeam DAT_801624b8[2];

/* The menu screens' scratchpad block; only proven fields are spelled. */
typedef struct {
    union {
        MenuState menu;  /* the menu screens' state */
        VsCardState card; /* the memory card screens' state (vs/card.h) */
        struct {
            u8 unk00[4];
            u8 *kinds;   /* 0x04: the item grid, four per row (func_80021140) */
            s16 count;   /* 0x08: items in the grid (func_80020aa4) */
            s16 rows;    /* 0x0a: its last row */
            s16 last;    /* 0x0c: items on the last row */
            u8 unk0E[2];
            DVECTOR pos[16]; /* 0x10: VRAM positions; [12..14] the team slot pictures, [15] the detail image */
            MenuStoredItem *items; /* 0x50: item records by kind (func_80021140) */
        } team;          /* the team screen (func_80017c84) */
    } unk0;              /* 0x000 */
    VsCardPort *unk1B8;  /* 0x1b8: the card slot being scanned */
    s16 unk1BC;          /* 0x1bc: its card port */
    s16 unk1BE;          /* 0x1be: the save file count, for the file list */
    u16 unk1C0;          /* 0x1c0 */
    s16 unk1C2;          /* 0x1c2: the file list cursor row, -1 on cancel */
    s16 unk1C4;          /* 0x1c4: the file list scroll */
    u16 unk1C6;          /* 0x1c6 (ST MenuScratch.unk202) */
    s16 unk1C8;          /* 0x1c8: the first free save number */
    s16 unk1CA;          /* 0x1ca: func_8001bb48's mode (func_80016f10 clears it) */
    u16 unk1CC;          /* 0x1cc (ST unk2DC); func_8001bb48 shifts its step bit through it */
    s16 unk1CE;          /* 0x1ce: the versus setup cursor line (func_80022a00) */
    s16 unk1D0;          /* 0x1d0: the list cursor row (ST unk2E0) */
    s16 unk1D2;          /* 0x1d2: the item grid row (func_80021140) */
    u8 unk1D4;           /* 0x1d4: cursor blink phase (ST unk2E4) */
    u8 unk1D5;           /* 0x1d5: font mode (ST unk2E5) */
    u8 unk1D6;           /* 0x1d6: a team slot is picked up (func_80020aa4) */
    u8 unk1D7;
    DVECTOR unk1D8;      /* 0x1d8: the message/menu font page's VRAM position (func_8001bb48, func_80018c84) */
    DVECTOR unk1DC;      /* 0x1dc: its CLUT position */
    DVECTOR unk1E0;      /* 0x1e0: the item name page's VRAM position (func_8001bb48) */
    DVECTOR unk1E4;      /* 0x1e4: its CLUT position */
    DVECTOR unk1E8;      /* 0x1e8: the setup page picture's VRAM position (func_80016f10) */
    u8 unk1EC[0x1f0 - 0x1ec];
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

/* A 21504-byte message table: byte offsets of each message from text,
 * then the messages (ST.EXE's MenuMessages layout). */
typedef struct {
    u16 offsets[0x600];
    u8 text[0x5400 - 0xc00];
} VsMessages;

typedef char VsMessagesSize[sizeof(VsMessages) == 0x5400 ? 1 : -1];

/* The two message tables at 0x80196230 (bss): the menus' and the item
 * names'. */
extern VsMessages DAT_80196230[2];

#define VS_MESSAGE(i) ((u16 *)(DAT_80196230[0].text + DAT_80196230[0].offsets[i]))
#define VS_ITEM_NAME(i) ((u16 *)(DAT_80196230[1].text + DAT_80196230[1].offsets[i]))

/* VS's copies of ST.EXE's text and primitive helpers (src/st/ui.c,
 * src/st/text.c), not reconstructed yet. */
void func_8001d8b8(u16 *s, u16 x, u16 y, CVECTOR *color); /* ST ui_draw_text */
void func_800204b0(void);                                /* the menu screens' frame: draws and flips */
void func_8001e3d8(RECT *r, CVECTOR *color, u16 semi);   /* ST ui_fill_rect */
void func_8001e530(RECT *r, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int mode); /* ST ui_draw_image */
void func_8001e890(int abr, int x, int y);               /* ST ui_add_draw_mode */
void func_8001f23c(int x, int y, int w, int h);          /* ST ui_draw_panel */
void func_8001de8c(RECT *r, CVECTOR *color, u16 border, u16 semi); /* ST ui_draw_rect_outline */
void func_8001eef4(RECT *r, CVECTOR *color, u16 dir);    /* ST ui_draw_arrow */
void func_8001d3f0(u16 c, u16 x, u16 y, CVECTOR *color);  /* ST ui_draw_glyph */
void func_8001d9cc(u32 num, u16 x, u16 y, CVECTOR *color); /* ST ui_draw_number */
void func_8001f30c(short x, short y, short w, short h); /* ST ui_draw_window */
void func_8001f828(void);                                /* ST menu_draw_background */
void func_8001f8c4(void);
void func_8001f960(void);                                /* ST card_draw_checking */
void func_8001f9a0(void);
void func_8001fb98(void);                                /* ST card_draw_format_prompt */
void func_8001fcac(void);
void func_80020384(void);                                /* ST card_draw_transfer */
void func_80020420(void);                                /* ST card_draw_finished */
void func_80020460(void);                                /* ST card_draw_message */
void func_80020aa4(int page);
void func_80021140(int page);                            /* ST menu_creature_book_draw_entry */
void func_80021cfc(void);
void func_80022228(void);
void func_800222d8(int page);
void func_80022794(void);                                /* ST menu_draw_button_prompt */
void func_80022a00(void);
void func_80022f14(int page);
void func_80023114(int page);
void func_8002326c(void);

#endif
