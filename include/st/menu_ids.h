/*
 * ST.EXE menu ids: the pause menu's rows, the menu font modes and the
 * message ids the screens draw (MESSAGE(), st/menu.h).
 *
 * Message ids index the table loaded into menu_messages: STAT.T (archive 3)
 * member 0 for the low-resolution pages (MSG_*), member 1 for the
 * 640x240 screens: equip info, creature book and both arrange-items views
 * (MSG_HI_*). VS.EXE's card screen loads member 0 too (card_screen_run)
 * and shares the card ids. Names follow each message's text in the game's
 * own table; a base marks the first of a run the code indexes
 * (MSG_AREA_NAMES + area).
 */
#ifndef ST_MENU_IDS_H
#define ST_MENU_IDS_H

/* MenuScratch.main_row: the pause menu's rows, drawn as MSG_PAUSE_ROWS + row
 * (menu_main_draw) and dispatched by menu_main_run. */
#define MENU_ROW_USE_ITEM 0
#define MENU_ROW_EQUIP 1
#define MENU_ROW_ARRANGE_ITEMS 2
#define MENU_ROW_CREATURE_BOOK 3
#define MENU_ROW_EQUIP_INFO 4
#define MENU_ROW_OPTIONS 5
#define MENU_ROW_LOAD_DATA 6
#define MENU_ROW_END_GAME 7
#define MENU_ROWS 8

/* MenuScratch.font_mode: the glyph set ui_draw_glyph uses. */
#define MENU_FONT_LOW_RES 0      /* 6x12 half-width cells (320-wide pages) */
#define MENU_FONT_HIGH_RES 1     /* full-width cells (640x240 screens) */

/* MenuScratch.mode bits of the equip screen (menu_equip_loop): which list
 * has the input. */
#define EQUIP_MENU_SLOT_LIST 0x1  /* the 15 slots (also set under the item list) */
#define EQUIP_MENU_MAGIC_LIST 0x2 /* a ring's magic */
#define EQUIP_MENU_ITEM_LIST 0x8  /* the items for one slot */

/* STAT.T member 0. */
#define MSG_PAUSE_ROWS 11        /* "use item" .. "end game" */
#define MSG_STATUS_NORMAL 19
#define MSG_STATUS_POISON 20
#define MSG_STATUS_PARALYZE 21
#define MSG_STATUS_DARK 22
#define MSG_STATUS_SLOW 23
#define MSG_STATUS_CURSED 24
#define MSG_ELEMENT_NONE 25
#define MSG_ELEMENTS 26          /* "fire" .. "dark": item element kinds 9.. */
#define MSG_SHOP_CUNES 100       /* "cunes" */
#define MSG_SHOP_HP_LEFT 101     /* "hp left" */
#define MSG_CUNES_SIGN 102       /* "c" after a price */
#define MSG_ATTRIBUTE_COLUMNS 103 /* "str spr / spd foc / ..." */
#define MSG_WEIGHT 104           /* "wgt" */
#define MSG_SHOP_BUY 105
#define MSG_SHOP_REPAIR 106
#define MSG_SHOP_TRADE 107
#define MSG_SHOP_CANCEL 108
#define MSG_DURABILITY 109       /* "dura" */
#define MSG_OPTION_LABELS 201    /* "adjust sfx" .. "button config", option rows 0.. */
#define MSG_ON 207
#define MSG_OFF 208
#define MSG_BUTTON_FORWARD 209   /* the twelve action labels, PAD_ACTION_* order */
#define MSG_BUTTON_LOOK_UP 215
#define MSG_STANDARD_SET 221
#define MSG_DONE 222
#define MSG_OPTION_HELP 224      /* "adjusts sfx volume" ..: the option rows' help */
#define MSG_CARD_CHECKING 310
#define MSG_CARD_UNFORMATTED 311 /* "... do you wish to format?" */
#define MSG_CARD_FORMATTING 312
#define MSG_CARD_SLOT_1 320
#define MSG_CARD_SLOT_2 321
#define MSG_CARD_NEW_DATA 322
#define MSG_CARD_LOAD_PROMPT 323 /* "load data?" */
#define MSG_CARD_SAVE_PROMPT 324 /* "save data?" */
#define MSG_CARD_YES 325
#define MSG_CARD_NO 326
#define MSG_CARD_SP 328
#define MSG_CARD_LOADING 330
#define MSG_CARD_SAVING 331
#define MSG_CARD_DO_NOT_REMOVE 332
#define MSG_CARD_FINISHED 340
#define MSG_CARD_CORRUPTED 349
#define MSG_CARD_NOT_INSERTED 350
#define MSG_CARD_SWITCHED 351
#define MSG_CARD_NO_DATA 352     /* "no shadow tower data" */
#define MSG_CARD_NO_ROOM 353     /* "not enough empty blocks" */
#define MSG_CARD_LOAD_FAILED 354
#define MSG_CARD_SAVE_FAILED 355
#define MSG_CARD_FORMAT_FAILED 356
#define MSG_CARD_CHANGED 357
#define MSG_CARD_NOT_LOADED 358
#define MSG_CARD_INSERT_1P 370   /* "insert 1p memory card ..."; 371 for 2p (VS.EXE) */
#define MSG_END_GAME_PROMPT 400
#define MSG_END_GAME_YES 401
#define MSG_END_GAME_NO 402
#define MSG_ATTR_STRENGTH 530    /* the 14 attribute names, strength .. soul */
#define MSG_ATTR_SPIRIT 537
#define MSG_REMAINING_SP 544
#define MSG_AREA_INFO_LABELS 600 /* "location", "cunes", "items", "creatures", "time" */
#define MSG_WORLD_NAMES 610      /* "human world" .. "unknown world": world ids 90.. */
#define MSG_WORLD_UNKNOWN 617
#define MSG_AREA_UNKNOWN 618     /* "???" */
#define MSG_AREA_NAMES 700       /* by area id */
#define MSG_NEW_GAME 800
#define MSG_LOAD_GAME 801
#define MSG_SAVE_PROMPT 850
#define MSG_SAVE_YES 851
#define MSG_SAVE_NO 852
#define MSG_EQUIP_SLOT_NAMES 900 /* the slot list's 15 rows, "helmet" .. "l mag" */
#define MSG_EQUIP_WEIGHT 915
#define MSG_MAGIC_NAMES 916      /* by magic_name_messages[effect kind] */
#define MSG_NO_EQUIP 934
#define MSG_NO_SELECT 935
#define MSG_MP_USED 936

/* STAT.T member 1. */
#define MSG_HI_ATTRIBUTES 14     /* "STRENGTH" .. "SOUL" */
#define MSG_HI_WEIGHT 28         /* "WGT" */
#define MSG_HI_DURABILITY 29     /* "DURA" */
#define MSG_HI_BOOK_ATTRIBUTES 100 /* the creature book's 14 attribute names */
#define MSG_HI_BOOK_LABELS 117   /* "TYPE", "HP", "SP GAIN", "HEIGHT", ... */
#define MSG_HI_ABILITIES 129     /* + an ability nibble: 130 "LAUGH" .. */
#define MSG_HI_CREATURE_TYPES 150 /* by CreatureBookEntry.category */
#define MSG_HI_HAND 200          /* "HAND", then "BAG" */
#define MSG_HI_MOVE_PROMPT 216   /* "MOVE / OK / CANCEL" */

#endif
