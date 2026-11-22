/*
 * ST.EXE archive member loaders, their completion callbacks and the sound
 * bank slots (src/st/bank.c, src/st/bankload.c).
 */
#ifndef ST_BANK_H
#define ST_BANK_H

#include <kf/lib/types.h>
#include <sys/types.h>

struct StAnimFile; /* st/anim.h */

u16 object_model_slot(u16 id);
/* CD read callbacks: each gets the heap block the member was read into. */
void creature_model_loaded(struct StAnimFile *file);
void creature_tim_loaded(u_long *tim);
void item_model_loaded(struct StAnimFile *file);
void item_tim_loaded(u_long *tim);
void object_model_loaded(struct StAnimFile *file);
/* Old-style definitions; these are their promoted prototypes. */
void creature_model_load(int archive, int index, int slot, int tag);
void creature_second_model_load(int archive, int index, int slot, int tag);
void item_model_load(int archive, int index, int slot, int tag);

#endif
