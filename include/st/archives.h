/*
 * ST.EXE archive numbers: the tables game_load_common_data (src/st/area_load.c)
 * opens with cd_archive_open, and the members of STAT.T the menus load.
 */
#ifndef ST_ARCHIVES_H
#define ST_ARCHIVES_H

#define ARCHIVE_MO 0           /* COM\MO.T: object models and area sounds */
#define ARCHIVE_FDAT 1         /* COM\FDAT.T: area files, overlays, player models, magic */
#define ARCHIVE_CHR 2          /* CHR0\M00.T: the area's creatures (reopened by area) */
#define ARCHIVE_STAT 3         /* COM\STAT.T: menu messages, fonts and pictures */
#define ARCHIVE_CRT 4          /* COM\CRT.T: the creature book's pictures */
#define ARCHIVE_ITEM 5         /* COM\ITEM.T: item models */
#define ARCHIVE_ICON 6         /* COM\ICON.T: item pictures */
#define ARCHIVE_EQUIP 7        /* COM\EQUIP.T: the equip info screen's pictures */
#define ARCHIVE_TALKMSG 8      /* COM\TALKMSG.T: talk message pictures */

/* STAT.T members: each a message table (MenuMessages) and its font image,
 * or images only. */
#define STAT_MESSAGES 0        /* the low-resolution pages' messages and font */
#define STAT_SCREEN_MESSAGES 1 /* the 640x240 screens' messages and font */
#define STAT_CREATURE_NAMES 2
#define STAT_ITEM_NAMES 3      /* item names, low-resolution font */
#define STAT_ITEM_NAMES_HIGH_RES 4 /* item names, 640x240 font */
#define STAT_BACKGROUND 5      /* the 640x240 screens' background */
#define STAT_HEADER_IMAGES 7

#endif
