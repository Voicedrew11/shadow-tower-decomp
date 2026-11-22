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
} VsLineup;

/* The team screen's edit record (a local of menu_creature_select_loop): the slot picked
 * (unk4) and a copy of the team being edited. */
typedef struct {
    u8 holding;          /* 0x00: a slot is held (menu_creature_select_loop) */
    u8 unk1[3];
    s32 held_slot;       /* 0x04: the picked slot */
    VsLineup lineup;     /* 0x08 */
} VsLineupEdit;

/* Both players' lineups (bss, 0x801624b8). */
extern VsLineup lineups[2];

/* The menu screens' scratchpad block; only proven fields are spelled. */
typedef struct {
    union {
        MenuState menu;  /* the menu screens' state */
        VsCardState card; /* the memory card screens' state (vs/card.h) */
        struct {
            u8 unk00[4];
            u8 *kinds;   /* 0x04: the item grid, four per row (menu_creature_book_draw_entry) */
            s16 count;   /* 0x08: items in the grid (menu_creature_select_draw_grid) */
            s16 rows;    /* 0x0a: its last row */
            s16 last;    /* 0x0c: items on the last row */
            u8 unk0E[2];
            DVECTOR pos[16]; /* 0x10: VRAM positions; [12..14] the team slot pictures, [15] the detail image */
            CreatureBookEntry *entries; /* 0x50: item records by kind (menu_creature_book_draw_entry) */
        } creature_select; /* the team screen (menu_creature_select_pick_up_slot) */
    } state;             /* 0x000 */
    VsCardDirectory *card; /* 0x1b8: the card slot being scanned */
    s16 port;            /* 0x1bc: its card port */
    s16 row_count;       /* 0x1be: the save file count, for the file list */
    u16 has_new_row;     /* 0x1c0 */
    s16 file_row;        /* 0x1c2: the file list cursor row, -1 on cancel */
    s16 first_row;       /* 0x1c4: the file list scroll */
    u16 dialog_choice;   /* 0x1c6 (ST MenuScratch.dialog_choice) */
    s16 free_file_number; /* 0x1c8: the first free save number */
    s16 show_backdrop;   /* 0x1ca: card_screen_run's mode (menu_vs_setup_run clears it) */
    u16 mode;            /* 0x1cc (ST unk2DC); card_screen_run shifts its step bit through it */
    s16 main_row;        /* 0x1ce: the versus setup cursor line (menu_vs_setup_draw_options) */
    s16 cursor;          /* 0x1d0: the list cursor row (ST unk2E0) */
    s16 scroll;          /* 0x1d2: the item grid row (menu_creature_book_draw_entry) */
    u8 blink_phase;      /* 0x1d4: cursor blink phase (ST unk2E4) */
    u8 font_mode;        /* 0x1d5: font mode (ST unk2E5) */
    u8 holding;          /* 0x1d6: a team slot is picked up (menu_creature_select_draw_grid) */
    u8 unk1D7;
    DVECTOR message_font_vram; /* 0x1d8: the message/menu font page's VRAM position (card_screen_run, menu_creature_select_run) */
    DVECTOR message_font_clut; /* 0x1dc: its CLUT position */
    DVECTOR item_font_vram; /* 0x1e0: the item name page's VRAM position (card_screen_run) */
    DVECTOR item_font_clut; /* 0x1e4: its CLUT position */
    DVECTOR picture_vram; /* 0x1e8: the setup page picture's VRAM position (menu_vs_setup_run) */
    u8 unk1EC[0x1f0 - 0x1ec];
    CVECTOR style[8];    /* 0x1f0: text colours (ST 0x300) */
    u8 unk210[4];
    CVECTOR cursor_color; /* 0x214: highlight colour (ST unk32C) */
    RECT cursor_rect;    /* 0x218: highlight bar (ST unk330) */
    CVECTOR border_color; /* 0x220 (ST unk338) */
    RECT rect;           /* 0x224 (ST unk33C) */
    CVECTOR fill_color;  /* 0x22c (ST unk344) */
    RECT inner_rect;     /* 0x230 (ST unk348) */
    RECT image_rect;     /* 0x238 (ST unk350) */
    DVECTOR tex_pos;     /* 0x240: texture position in VRAM (ST unk358) */
    DVECTOR clut_pos;    /* 0x244: CLUT position in VRAM (ST unk35C) */
    u16 blink_level;     /* 0x248: blink offset (ST unk360) */
} VsMenuScratch;

/* Layout check: the offset the code proves. */
typedef char VsMenuScratchUnk1B8At0x1B8[offsetof(VsMenuScratch, card) == 0x1b8 ? 1 : -1];
typedef char VsMenuScratchStyleAt0x1F0[offsetof(VsMenuScratch, style) == 0x1f0 ? 1 : -1];
typedef char VsMenuScratchUnk244At0x244[offsetof(VsMenuScratch, clut_pos) == 0x244 ? 1 : -1];

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
extern VsMessages vs_menu_messages[2];

#define VS_MESSAGE(i) ((u16 *)(vs_menu_messages[0].text + vs_menu_messages[0].offsets[i]))
#define VS_ITEM_NAME(i) ((u16 *)(vs_menu_messages[1].text + vs_menu_messages[1].offsets[i]))

/* VS's copies of ST.EXE's text and primitive helpers (src/st/render.c,
 * src/st/text.c), not reconstructed yet. */
void ui_draw_text(u16 *s, u16 x, u16 y, CVECTOR *color); /* ST ui_draw_text */
void card_screen_draw(void);                                /* the menu screens' frame: draws and flips */
void ui_fill_rect(RECT *r, CVECTOR *color, u16 semi);   /* ST ui_fill_rect */
void ui_draw_image(RECT *r, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int mode); /* ST ui_draw_image */
void ui_add_draw_mode(int abr, int x, int y);               /* ST ui_add_draw_mode */
void ui_draw_panel(int x, int y, int w, int h);          /* ST ui_draw_panel */
void ui_draw_rect_outline(RECT *r, CVECTOR *color, u16 border, u16 semi); /* ST ui_draw_rect_outline */
void ui_draw_arrow(RECT *r, CVECTOR *color, u16 dir);    /* ST ui_draw_arrow */
void ui_draw_glyph(u16 c, u16 x, u16 y, CVECTOR *color);  /* ST ui_draw_glyph */
void ui_draw_number(u32 num, u16 x, u16 y, CVECTOR *color); /* ST ui_draw_number */
void ui_draw_window(short x, short y, short w, short h); /* ST ui_draw_window */
void menu_draw_background(void);                                /* ST menu_draw_background */
void menu_draw_background_lowres(void);
void card_draw_checking(void);                                /* ST card_draw_checking */
void card_draw_port_choice(void);
void card_draw_format_prompt(void);                                /* ST card_draw_format_prompt */
void card_draw_file_list(void);
void card_draw_transfer(void);                                /* ST card_draw_transfer */
void card_draw_finished(void);                                /* ST card_draw_finished */
void card_draw_message(void);                                /* ST card_draw_message */
void menu_creature_select_draw_grid(int page);
void menu_creature_book_draw_entry(int page);                            /* ST menu_creature_book_draw_entry */
void menu_item_request_draw_list(void);
void menu_arrange_icons_draw_caption(void);
void menu_item_request_draw_details(int page);
void menu_draw_button_prompt(void);                                /* ST menu_draw_button_prompt */
void menu_vs_setup_draw_options(void);
void menu_vs_setup_draw_choice(int page);
void menu_vs_setup_draw_notice(int page);
void menu_vs_setup_draw_no_item_notice(void);

#endif
