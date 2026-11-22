/*
 * ST.EXE menu screens (src/st/render.c): the scratchpad
 * layout they use, the message table they draw from and their functions. The screens share
 * one scratchpad block at 0x1f800000 that differs from the renderer's
 * (st/scratch.h): screen state words at the start, the list fields around
 * +0x1f4, the input and layout fields at +0x2dc and the drawing records
 * from +0x300. Field names stay address-derived until behaviour is proven.
 */
#ifndef ST_MENU_H
#define ST_MENU_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>
#include <LIBCD.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/text.h>
#include <st/menu_ids.h>

/*
 * The message table: byte offsets of each message from text, then the
 * messages, halfword glyph codes ending at 0xffff (0 breaks the line).
 */
typedef struct {
    u16 offsets[0x600];
    u8 text[0x5400 - 0xc00];
} MenuMessages;

typedef char MenuMessagesSize[sizeof(MenuMessages) == 0x5400 ? 1 : -1];

/*
 * The id grid two screens share (menu_arrange_icons_draw_caption, menu_arrange_icons_draw_details): two tables
 * of four ids per row, the first row each one shows, and the message table
 * the ids index.
 */
typedef struct {
    s16 hand_row;        /* 0x00: first row shown of hand_ids */
    s16 bag_row;         /* 0x02: first row shown of bag_ids */
    s16 *hand_ids;       /* 0x04 */
    s16 *bag_ids;        /* 0x08 */
    s16 hand_count;      /* 0x0c: ids in hand_ids (src/st/idgrid.c, menu_arrange_icons_run) */
    s16 bag_count;       /* 0x0e: ids in bag_ids */
    DVECTOR *cell_tex;   /* 0x10: cell images, twenty per block (menu_arrange_icons_draw_hand) */
    MenuMessages *messages; /* 0x14 */
} ArrangeGrids;

/*
 * The item list the shop and the equip menu share: its message table,
 * the listed ids and their count.
 */
typedef struct {
    MenuMessages *messages; /* 0x00 */
    s16 *ids;            /* 0x04 */
    s16 count;           /* 0x08 */
    u16 confirm_choice;  /* 0x0a */
} ItemList;

/* A 44-byte creature kind record menu_creature_book_draw_entry details. */
typedef struct {
    u8 category;         /* 0x00: indexes the type names (MESSAGE(150 + n)) */
    u8 sp_gain;          /* 0x01 */
    u16 hp;              /* 0x02 */
    s16 height;          /* 0x04: shown as height * 10 / 3048 and its remainder / 254 */
    s16 weight;          /* 0x06: shown as weight * 100 / 454 with tenths */
    s16 pairs[4][2];     /* 0x08: value, then the optional value before it */
    u32 abilities;       /* 0x18: message nibbles at bits 0, 8 and 16 */
    s8 attributes[14];   /* 0x1c */
    u8 unk2A[2];
} CreatureBookEntry;

/*
 * The creature book grid (menu_creature_book_draw_grid): the kind records, the message
 * table, one kind per entry, the entry count, the last row and its length,
 * the VRAM image of each of the fifteen visible cells and the detail page's.
 */
typedef struct {
    CreatureBookEntry *entries; /* 0x00 */
    MenuMessages *names; /* 0x04 */
    u8 *kinds;           /* 0x08 */
    s16 count;           /* 0x0c */
    s16 rows;            /* 0x0e */
    s16 last;            /* 0x10: cells on the last row */
    u8 unk12[2];
    DVECTOR cell_tex[15]; /* 0x14 */
    DVECTOR image;       /* 0x50: menu_creature_book_draw_entry's image */
} CreatureBook;

/* The screen state at the start of the scratchpad, per screen. */
typedef union {
    s32 word;
    u32 *block;          /* menu_equip_open's 21504-byte message copy */
    s16 half[2];
    ArrangeGrids grid;
    ItemList list;
    DVECTOR vram[2];     /* ui_draw_cell_grid's texture and CLUT positions */
    CreatureBook book;
    DVECTOR icons[6];    /* the pause list page's icon texture positions, used from [1] */
    struct {
        MenuMessages *messages; /* 0x00: the list's message table */
        DVECTOR icons[5]; /* 0x04: the rows' icon texture positions (MenuState.icons[1..5]) */
        s16 count;       /* 0x18: entries in the list */
        u8 unk1A[2];
        s16 *entries;    /* 0x1c: entry ids */
    } page_list;         /* the pause menu list page (menu_use_item_draw) */
    struct {
        u8 unk00[0x20];
        s16 value;       /* 0x20 */
        s16 selected;    /* 0x22 */
        s16 base[2][7];  /* 0x24: left and right column base values */
        s16 bonus[2][7]; /* 0x40: and their bonuses */
    } stats;
    struct {
        s32 scroll;
        s32 current;     /* 0x04: the entry marked current, -1 for none */
        s32 ids[10];     /* 0x08 */
    } map;
    struct {
        s16 flags;       /* 0x00: tested against pad_state.buttons */
        u8 level[12];    /* 0x02 */
    } levels;            /* menu_button_config_draw */
} MenuState;

/*
 * A 32-byte list record card_draw_file_list lists: one memory card save file
 * (src/st/menu.c): its directory name, the number in its name and bytes
 * from its header.
 */
typedef struct {
    char name[0x14];     /* 0x00: the file name firstfile/nextfile found */
    s32 number;          /* 0x14: the two digits at name[17..18] */
    u8 unk18;
    u8 area;             /* 0x19: indexes the record title table */
    u16 sp_total;        /* 0x1a */
    u16 items_found;     /* 0x1c */
    u8 data_sum;         /* 0x1e */
    u8 unk1F;
} CardFileEntry;

/* One memory card port's directory as the save screens keep it. */
typedef struct {
    CardFileEntry files[7]; /* 0x00 */
    u16 status;          /* 0xe0: 0 ready, 1 new card, 2 error, 3 unformatted;
                          * 4/5 are set by card_read_directory */
    s16 count;           /* 0xe2: files found */
    s16 has_room;        /* 0xe4: 1 when at least two blocks are free */
    u8 unkE6[2];
} CardDirectory;

/*
 * The memory card screens' view of the menu scratchpad block's first 0x1f4
 * bytes (src/st/menu.c): the screen state halfwords (MenuState.half), the
 * SwCARD and HwCARD event descriptors (EvSpIOE, EvSpERROR, EvSpTIMOUT,
 * EvSpNEW each) and both ports' directories. Later fields are MenuScratch's.
 */
typedef struct {
    s16 message;         /* 0x00: message id */
    s16 stage;           /* 0x02 */
    long sw_events[4];   /* 0x04 */
    long hw_events[4];   /* 0x14 */
    CardDirectory cards[2]; /* 0x24 */
} MenuCardScratch;

#define MENU_CARD_SCRATCH ((MenuCardScratch *)0x1f800000)

typedef struct {
    MenuState state;     /* 0x000 */
    u8 unk5C[0x174 - 0x5c]; /* 0x05c */
    s16 *category_lists[10]; /* 0x174: the equip info page's id lists, by kind (menu_equip_info_run) */
    s16 category_counts[10]; /* 0x19c: and their lengths */
    s16 *choices;        /* 0x1b0: the ids the open list shows */
    s16 attributes_clamped[14]; /* 0x1b4: attribute values */
    u8 unk1D0[0x24];     /* 0x1d0 */
    CardDirectory *card; /* 0x1f4: the listed records: the current port's directory */
    s16 port;            /* 0x1f8: the memory card port (src/st/menu.c) */
    s16 row_count;       /* 0x1fa: list length */
    s16 has_new_row;     /* 0x1fc */
    s16 file_row;        /* 0x1fe: cursor row */
    s16 first_row;       /* 0x200: first visible list entry */
    s16 dialog_choice;   /* 0x202: dialog choice (card_ask_yes_no returns it) */
    s16 free_file_number; /* 0x204: lowest unused save file number (card_read_directory) */
    s16 show_backdrop;   /* 0x206 */
    s32 save_fresh;      /* 0x208: card_screen_run hands it to save_pack_data */
    u8 unk20C[0x50];     /* 0x20c */
    s16 bonus_changes[14]; /* 0x25c: attribute changes */
    u8 unk278[0x292 - 0x278]; /* 0x278 */
    u16 saved_equipment[21]; /* 0x292: copy of viewer.equipment (equip_apply_slot_change, equip_restore_slots) */
    s16 unk2BC;          /* 0x2bc */
    s16 choice_index;    /* 0x2be */
    s16 choice_id;       /* 0x2c0 */
    u16 choice_count;    /* 0x2c2: choices' length */
    s16 bar_scale;       /* 0x2c4: attribute bar scale */
    u8 show_item_values; /* 0x2c6 */
    u8 list_kind;        /* 0x2c7 */
    u8 unk2C8[4];        /* 0x2c8 */
    MenuMessages *item_messages; /* 0x2cc */
    u32 *ring_magic_bits; /* 0x2d0: 36 words of grid cell bits (menu_equip_info_run) */
    u32 *ring_magic;     /* 0x2d4: one bit per grid cell */
    s16 hp_before;       /* 0x2d8: viewer.hp when the equip info page opened (src/st/menu_equip_arrange.c, src/st/menu.c) */
    s16 mp_before;       /* 0x2da: viewer.mp */
    u16 mode;            /* 0x2dc */
    u16 main_row;        /* 0x2de: selected row of the pause menu (and the save prompt's choice) */
    s16 cursor;          /* 0x2e0 */
    s16 scroll;          /* 0x2e2 */
    u8 blink_phase;      /* 0x2e4: cursor blink phase */
    u8 font_mode;        /* 0x2e5: text font mode (0: 6x12 half-width cells) */
    DVECTOR image_vram[5]; /* 0x2e6: VRAM positions card_screen_open sets (image, CLUT pairs);
                          * [0]/[1] menu_game_over_run's texture and CLUT, [2]/[3] menu_equip_open's;
                          * src/st/idgrid.c uploads its images to all five */
    u8 unk2FA[6];        /* 0x2fa */
    CVECTOR style[8];    /* 0x300: text colours; .cd carries the glyph size */
    CVECTOR *value_style; /* 0x320: figure colours of the status pages */
    CVECTOR *increase_style; /* 0x324 */
    CVECTOR *decrease_style; /* 0x328 */
    CVECTOR cursor_color; /* 0x32c: highlight colour */
    RECT cursor_rect;    /* 0x330: highlight bar */
    CVECTOR border_color; /* 0x338 */
    RECT rect;           /* 0x33c */
    CVECTOR fill_color;  /* 0x344 */
    RECT inner_rect;     /* 0x348 */
    RECT image_rect;     /* 0x350 */
    DVECTOR tex_pos;     /* 0x358: texture position in VRAM */
    DVECTOR clut_pos;    /* 0x35c: CLUT position in VRAM */
    u16 blink_level;     /* 0x360: blink offset */
} MenuScratch;

/* One 32-byte record player_total_attributes writes from scratch +0x8 per shown
 * equipment slot (MenuAttrScratch.recs). */
typedef struct {
    u8 weight;
    u8 element;          /* 0x1: kind - 9, indexes unk27A/unk286 */
    u16 element_amount;  /* 0x2 */
    s16 attributes[14];  /* 0x4 */
} EquipSlotRecord;

/*
 * The status screens' phase of the same block: seven attribute rows of 14
 * from +0x1b4 (MenuScratch's unk1B4 and unk25C are the first and last),
 * over the list fields the other screens keep there (player_total_attributes).
 */
typedef struct {
    u8 unk0[8];
    EquipSlotRecord recs[11]; /* 0x008: one per shown equipment slot */
    EquipSlotRecord *next_rec; /* 0x168: next record player_total_attributes fills */
    u16 right_magic_attr; /* 0x16c: viewer.equipment[6] / 3, 0xffff: none */
    u16 right_magic_bonus; /* 0x16e */
    u16 left_magic_attr; /* 0x170: viewer.equipment[12] / 3, 0xffff: none */
    u16 left_magic_bonus; /* 0x172 */
    u8 unk174[0x1b4 - 0x174];
    s16 attributes_clamped[14]; /* 0x1b4: shown values, clamped at 0 */
    s16 attribute_bonuses_clamped[14]; /* 0x1d0: equipment sums, clamped at 0 */
    s16 attributes[14];  /* 0x1ec: totals, at most 999 */
    s16 attribute_bonuses[14]; /* 0x208: equipment sums */
    s16 prev_attribute_bonuses[14]; /* 0x224: viewer.attribute_bonuses */
    s16 base_attributes[14]; /* 0x240: viewer.base_attributes */
    s16 bonus_changes[14]; /* 0x25c: attribute_bonuses - prev_attribute_bonuses */
    u8 unk278[2];
    u16 accessory_elements[6]; /* 0x27a: per kind sums of the records' unk2 */
    u16 armour_elements[6]; /* 0x286 */
    u16 saved_equipment[21]; /* 0x292: MenuScratch.saved_equipment */
    s16 equip_weight;    /* 0x2bc: sum of the shown records' unk24 */
} MenuAttrScratch;

#define MENU_ATTR ((MenuAttrScratch *)0x1f800000)

/* The viewer's 21 equipment slots (viewer.equipment, st/data.h) as one block:
 * the status screens copy it whole and index it through this view. */
typedef struct {
    u16 ids[21];
} EquipmentSlots;

#define MENU_SLOTS ((EquipmentSlots *)viewer.equipment)

#define MENU_SCRATCH ((MenuScratch *)0x1f800000)

/*
 * A word of story flags: ST.EXE masks it whole; the FDAT overlays read single
 * bits as 1-bit fields (a byte load for bit 7, a compare with 1).
 */
typedef union {
    u32 word;
    struct {
        u32 f0 : 1, f1 : 1, f2 : 1, f3 : 1, f4 : 1, f5 : 1, f6 : 1, f7 : 1;
        u32 f8 : 1, f9 : 1, f10 : 1, f11 : 1, f12 : 1, f13 : 1, f14 : 1, f15 : 1;
        u32 f16 : 1, f17 : 1, f18 : 1, f19 : 1, f20 : 1, f21 : 1, f22 : 1, f23 : 1;
        u32 f24 : 1, f25 : 1, f26 : 1, f27 : 1, f28 : 1, f29 : 1, f30 : 1, f31 : 1;
    } bit;
    struct {
        u32 : 10;
        u32 gate : 3; /* bits 10-12: FDAT_272.BIN's gate stage (map_update_events) */
    } field;
} EventFlags;

/* The save-game state block at 0x801cba08 (0x1714 bytes). */
typedef struct {
    /* 0x0..0x1440: per area (48), one bit per table A entry, table C slot 0..31
     * and three bits per table B entry (src/st/save.c) */
    u32 creatures_gone[48][2];
    u32 ground_items_gone[48][1];
    u32 objects_removed[48][8];
    u32 objects_unlocked[48][8];
    u32 objects_finished[48][8];
    EventFlags event_flags; /* 0x1440: flag bits; 13 and 17 gate area_reload_images's images */
    EventFlags event_flags2;
    u8 talk_counts[48];  /* 0x1448: per XA stream file, the next channel to play */
    u8 areas_visited[48];
    /* 0x14a8: one bit per creature kind (CreatureType.kind): [0] seen
     * (world_draw_creatures; the creature book lists them), [1] killed
     * (creature_take_damage); game_load_start_file's flag reset sets both but bit 14 of word 4 */
    u32 creature_kind_bits[2][5];
    u32 vs_items_lost[0x4f]; /* 0x14d0: VS.EXE: the items lost in versus */
    u32 vs_lineup[3];    /* 0x160c: VS.EXE: the lineups */
    /* 0x1618: per XA stream file, its position (xa_start_stream); game_load_common_data
     * searches the first 40 */
    CdlLOC xa_positions[48];
    s32 item_use_result; /* 0x16d8: the examine/item-use result: 0 none, 1 done, 2 nothing to use (src/st/item.c) */
    u8 warp_back_request; /* 0x16dc: nonzero runs player_warp_back after the table A pass (creatures_update); player_warp_back clears it */
    u8 unk16DD[3];
    s32 save_id;         /* 0x16e0: save id, -1 until the first save (src/st/save.c) */
    u8 area_kills[48];   /* by area */
} GameProgress;
extern GameProgress game_progress;
typedef char GameProgressSize[sizeof(GameProgress) == 0x1714 ? 1 : -1];
typedef char GameProgressEventFlagsAt0x1440[offsetof(GameProgress, event_flags) == 0x1440 ? 1 : -1];
typedef char GameProgressKindBitsAt0x14A8[offsetof(GameProgress, creature_kind_bits) == 0x14a8 ? 1 : -1];
typedef char GameProgressVsItemsLostAt0x14D0[offsetof(GameProgress, vs_items_lost) == 0x14d0 ? 1 : -1];
typedef char GameProgressVsLineupAt0x160C[offsetof(GameProgress, vs_lineup) == 0x160c ? 1 : -1];
typedef char GameProgressXaPositionsAt0x1618[offsetof(GameProgress, xa_positions) == 0x1618 ? 1 : -1];
typedef char GameProgressItemUseResultAt0x16D8[offsetof(GameProgress, item_use_result) == 0x16d8 ? 1 : -1];


/* .sbss words the area list page (menu_area_info_*) reads with lui; their
 * owner is unknown. */
extern s32 DAT_8008818c;
extern u16 area_info_list_shown;

/* Option page state bytes; owner unknown. */


extern MenuMessages menu_messages;

#define MESSAGE(i) ((u16 *)(menu_messages.text + menu_messages.offsets[i]))
/* A message of another table (a list's own). */
#define MESSAGE_IN(table, i) ((u16 *)((table)->text + (table)->offsets[i]))

/* Display scratch for figures shown in imperial units (weight / 4.54 in
 * pounds, heights in feet): the whole part and the tenths. The shop, the
 * equipment info, the creature book and the arrange screens use them. */
extern u16 menu_figure_whole;
extern u16 DAT_80088188;

/* The status pages' toggle between the attribute and the figure page. */
extern u16 menu_second_page;
extern u16 DAT_80088192;


/* Seven list slots cleared together by card_draw_port_choice. */
extern s32 file_row_clipped[7];
extern s16 DAT_800881f0[7];

/* src/st/render.c: the pause menu pages */
void menu_main_draw(void);
void menu_use_item_draw(void);
void menu_attribute_points_draw(void);
void menu_area_info_draw(void);
void menu_options_draw(void);
void menu_button_config_draw(void);

/* the first menu page bodies */
void card_draw_checking(void);
void card_draw_port_choice(void);
void card_draw_format_prompt(void);
void card_draw_file_list(void);

/* the page frame and dialogs */
void card_draw_transfer(void);
void card_draw_finished(void);
void card_draw_message(void);
void menu_draw_background(void);
void card_screen_draw(void);
void menu_end_game_draw_dialog(void);
void menu_end_game_draw(void);
void menu_game_over_draw(void);
void menu_save_prompt_draw(void);
void shop_draw_header(void);

/* the item list screens */
void shop_draw_cursor(void);
void shop_draw_buy_list(void);
void shop_draw_trade_list(void);
void shop_draw_repair_list(void);
void shop_draw_confirm(void);
void shop_draw(void);

/* the status screens */
void menu_equip_info_draw_panels(void);
void menu_equip_info_draw_slots(void);
void menu_equip_info_draw_attributes(void);

void menu_equip_info_draw_stats(void);
/* VS.EXE has its own port under this name, with another signature; VS units define VS_EXE. */
#ifndef VS_EXE
void ui_draw_cell_grid(DVECTOR *pos, DVECTOR *tex, DVECTOR *clut, CVECTOR *color, int arg4);
#endif
void menu_equip_info_draw_item(void);
void menu_equip_info_draw_magic(void);

void menu_equip_info_draw(void);

/* the creature book screens */
void menu_creature_book_draw_grid(void);
/* VS.EXE has its own port under this name, with another signature; VS units define VS_EXE. */
#ifndef VS_EXE
void menu_creature_book_draw_entry(void);
#endif

void menu_creature_book_draw(void);
void menu_arrange_icons_draw_hand(void);
void menu_arrange_icons_draw_bag(void);
/* the id grid screens */
void menu_arrange_icons_draw_caption(void);
void menu_arrange_icons_draw_details(void);
void menu_draw_button_prompt(void);
void menu_arrange_icons_draw(void);
void clear_vram(void);
void pause_screen_draw(void);


#endif
