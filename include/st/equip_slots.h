/*
 * The 21 equipment slots of viewer.equipment (st/data.h), the save image's
 * copy (StSaveData.equipment, st/save.h) and VS.EXE's copy of a save.
 *
 * Evidence: the equip screen's slot list (menu_equip_draw_slots,
 * src/st/menu_equip_arrange.c) draws row n with STAT.T member 0 message
 * 900 + n and reads the slot each row maps to; the quick-item select of
 * player_update (src/st/player.c) cycles over the five quick slots in the
 * order listed; menu_equip_pick_item copies the gloves slot into
 * EQUIP_SLOT_GLOVES_MIRROR whenever it sets it. The right hand's casts wear
 * the right ring and use the right magic slot, the left hand's the left ones
 * (player_update_cast).
 */
#ifndef ST_EQUIP_SLOTS_H
#define ST_EQUIP_SLOTS_H

#define EQUIP_SLOT_RIGHT_RING 0       /* message 911 "r ring" */
#define EQUIP_SLOT_RIGHT_BRACELET 1   /* 908 "r brac" */
#define EQUIP_SLOT_GLOVES 2           /* 902 "gloves" */
#define EQUIP_SLOT_RIGHT_HAND 3       /* 904 "r hand": the weapon, model slot 0 */
#define EQUIP_SLOT_RIGHT_RESERVE 4    /* 905 "r resr": model slot 2, swapped with the hand */
#define EQUIP_SLOT_QUICK_1 5          /* quick item 1 */
#define EQUIP_SLOT_RIGHT_MAGIC 6      /* 912 "r mag": an effect kind 0..17 */
#define EQUIP_SLOT_AMULET 7           /* 910 "amulet" */
#define EQUIP_SLOT_HELMET 8           /* 900 "helmet" */
#define EQUIP_SLOT_ARMOR 9            /* 901 "armor" */
#define EQUIP_SLOT_BOOTS 10           /* 903 "boots" */
#define EQUIP_SLOT_QUICK_3 11         /* quick item 3 */
#define EQUIP_SLOT_LEFT_MAGIC 12      /* 914 "l mag": an effect kind 0..17 */
#define EQUIP_SLOT_LEFT_RING 13       /* 913 "l ring" */
#define EQUIP_SLOT_LEFT_BRACELET 14   /* 909 "l brac" */
#define EQUIP_SLOT_GLOVES_MIRROR 15   /* not listed: set with EQUIP_SLOT_GLOVES */
#define EQUIP_SLOT_LEFT_HAND 16       /* 906 "l hand": the shield, model slot 1 */
#define EQUIP_SLOT_LEFT_RESERVE 17    /* 907 "l resr": model slot 3, swapped with the hand */
#define EQUIP_SLOT_QUICK_5 18         /* quick item 5 */
#define EQUIP_SLOT_QUICK_2 19         /* quick item 2 */
#define EQUIP_SLOT_QUICK_4 20         /* quick item 4 */

#define EQUIP_SLOTS 21

/* An empty slot. */
#define EQUIP_NONE 0xffff

#endif
