/*
 * ST.EXE archive member loaders, their completion callbacks and the sound
 * bank slots (src/st/bank.c, src/st/bankload.c).
 */
#ifndef ST_BANK_H
#define ST_BANK_H

#include <kf/lib/types.h>

u16 object_model_slot(u16 id);
void creature_model_loaded(void *data);
void creature_tim_loaded(void *data);
void item_model_loaded(void *data);
void item_tim_loaded(void *data);
void object_model_loaded(void *data);
/* Old-style definitions; these are their promoted prototypes. */
void creature_model_load(int archive, int index, int slot, int tag);
void creature_second_model_load(int archive, int index, int slot, int tag);
void item_model_load(int archive, int index, int slot, int tag);

#endif
