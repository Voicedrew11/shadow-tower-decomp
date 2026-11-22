/*
 * ST.EXE model slot bases: the anim/model slot numbers (anim_get_file,
 * model_draw, world_state.model_files) where each world table's models
 * start. creature_models_sync, object_models_sync and item_models_sync
 * fill them (src/st/render.c); Creature.model_slot, AreaObject.type and
 * ItemType.model are offsets from these.
 */
#ifndef ST_MODEL_SLOTS_H
#define ST_MODEL_SLOTS_H

#define MODEL_SLOT_CREATURES 128   /* 16 slots */
#define MODEL_SLOT_OBJECTS 256
#define MODEL_SLOT_ITEMS 588       /* 128 slots */

#endif
