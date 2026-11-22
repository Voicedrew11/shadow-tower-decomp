/*
 * ST.EXE item ids (ItemType, GroundItem.item, inventory.counts, st/vis.h)
 * the code tests by number. Named after STAT.T member 3, the item names;
 * only ids whose own behaviour the code shows are listed.
 */
#ifndef ST_ITEM_IDS_H
#define ST_ITEM_IDS_H

#define ITEM_CUNES 266           /* the money the shops take (shop_run) */
#define ITEM_SACRED_FEATHER 270  /* item_use: warps back to the save point */
#define ITEM_DIVINE_SYMBOL 288   /* item_use changes the stats: callers recompute them */

#endif
