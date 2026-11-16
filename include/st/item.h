/*
 * ST.EXE item counts, the examine action and item use (src/st/item.c,
 * src/st/item_spawn.c). Field names stay address-derived.
 */
#ifndef ST_ITEM_H
#define ST_ITEM_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>

void func_800549c8(Vis212 *e, int id);
int func_8005529c(int id);
int func_800552bc(int id);
int func_80055300(int id);
/* func_80055374 is declared in st/player.h. */
/* Not reconstructed yet (0x80054a54): uses item `id`, leaving the result in
 * DAT_801cd0e0 (src/st/menu.c's item page calls it). */
void func_80054a54(int id);

#endif
