/*
 * ST.EXE player figure recomputation (player_total_attributes .. player_compute_derived_stats; the
 * band P side is src/st/menu.c). It works in the scratchpad block at
 * 0x1f800000 in a layout of its own: rows of fourteen figures. Only the
 * rows the reconstructed code reads are named. WIP.
 */
#ifndef ST_STATS_H
#define ST_STATS_H

#include <kf/lib/types.h>

typedef struct {
    u8 unk0[0x1ec];
    u16 unk1EC[14];      /* 0x1ec: copied to viewer.attributes */
    u16 unk208[14];      /* 0x208: copied to viewer.attribute_bonuses */
} StatsScratch;

#define STATS_SCRATCH ((StatsScratch *)0x1f800000)

void player_store_attributes(void);

#endif
