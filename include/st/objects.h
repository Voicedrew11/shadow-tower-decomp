/*
 * ST.EXE world object passes (src/st/objects.c): each frame they walk the
 * visible entries of the three object tables (st/vis.h) and two free-standing
 * object arrays, flag the model slots they need, and draw the models through
 * model_draw.
 */
#ifndef ST_OBJECTS_H
#define ST_OBJECTS_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

/* world_draw_effects_placements also draws the effects (effects.slots, st/effect.h). */

/* src/st/objects.c */
void world_draw_creatures(void);
void world_draw_objects(void);
void world_draw_ground_items(void);
void world_draw_effects_placements(void);

/* src/st/slots.c */
void object_model_load(int archive, int index, int slot);
void creature_models_sync(int archive, int first, int slot, int count);
void item_models_sync(int archive, int first, int slot, int count);
void object_models_sync(int archive, int first, int slot, int count);

#endif
