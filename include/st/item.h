/*
 * ST.EXE item counts, the examine action and item use (src/st/item.c,
 * src/st/creature_object.c). Field names stay address-derived.
 */
#ifndef ST_ITEM_H
#define ST_ITEM_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>

Vis88 *creature_alloc_object(Creature *e, int id);
int item_is_held(int id);
int item_take(int id);
int item_add(int id);
/* player_examine is declared in st/player.h. */
/* Uses item `id`, leaving the result in game_progress.unk16D8 (src/st/item_use.c;
 * src/st/menu.c's item page calls it). */
void item_use(int id);

#endif
