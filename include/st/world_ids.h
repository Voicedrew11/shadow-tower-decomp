/*
 * ST.EXE world entry states and kinds (st/vis.h): creatures (table A),
 * area objects (table B) and ground items (table C), as their update
 * functions step them. Values not listed stay numeric where the code does
 * not show their meaning.
 */
#ifndef ST_WORLD_IDS_H
#define ST_WORLD_IDS_H

/* Creature.state (creature_update_spawn, creature_take_damage, save.c). */
#define CREATURE_STATE_WAITING 0     /* not spawned: spawns by chance in its zone */
#define CREATURE_STATE_ACTIVE 1      /* despawns when its zone is left */
#define CREATURE_STATE_ACTIVE_NEAR 2 /* also despawns 28672 units away */
#define CREATURE_STATE_GONE 3        /* retired by creature_despawn; kept in game_progress.creatures_gone */
#define CREATURE_STATE_DYING 5

/*
 * Creature.action: the action bit its scripts' `actions` masks are matched
 * against (fdat_common.inc). A type whose actions hold DORMANT spawns
 * dormant (creature_update_spawn); a hit or the player's approach wakes it.
 * 2, 4, 16 and 32 are picked by distance, facing and aggression
 * (creature_pick_move) without a shown behaviour, and stay numeric.
 */
#define CREATURE_ACTION_DEFAULT 0x1
#define CREATURE_ACTION_TURN_AWAY 0x8
#define CREATURE_ACTION_DORMANT 0x80
#define CREATURE_ACTION_WAKING 0x100

/* AreaObject.kind (objects_load, objects_update, player_examine). */
#define OBJECT_KIND_DOOR 0           /* swaps two map-block runs open and shut */
#define OBJECT_KIND_MODEL_DOOR 1     /* the same, with a model drawn while shut */
#define OBJECT_KIND_SAVE_POINT 14    /* examining it offers a save */
#define OBJECT_KIND_GATE 50
#define OBJECT_KIND_ITEM_GATE 51     /* hides a ground item until it opens */
#define OBJECT_KIND_GROUP_GATE 52    /* opens the gates and doors of its lock group */
#define OBJECT_KIND_SEQUENCE 224     /* a button-started sequence over creatures 56..63 */
#define OBJECT_KIND_MESSAGE 225      /* examined from the front: talk_show_message */
#define OBJECT_KIND_HEALING 226      /* examining it restores all HP */
#define OBJECT_KIND_SOUND 236        /* a sound source repeating by period */
#define OBJECT_KIND_WARP 237         /* entered with the event button */
#define OBJECT_KIND_WALK_IN_WARP 239 /* entered by walking in */
#define OBJECT_KIND_NONE 255

/* AreaObject.params.door.lock: a key item id - KEY_ITEM_BASE, or these. */
#define DOOR_LOCK_OPENS_FROM_FRONT 251 /* unlocks when examined facing its front */
#define DOOR_LOCK_OPENS_FROM_BACK 252
#define DOOR_UNLOCKED 255
#define KEY_ITEM_BASE 272            /* object_try_key: lock = item id - 272 */

/* GroundItem.state (ground_items_update, world_draw_ground_items). */
#define GROUND_ITEM_NONE 0
#define GROUND_ITEM_FALLING 1        /* bounces four times, then rests */
#define GROUND_ITEM_RESTING 2
#define GROUND_ITEM_PICKED_UP 3      /* ground_item_pick_up shows it */
#define GROUND_ITEM_HIDDEN 4         /* not drawn (an item gate's, until it opens) */
#define GROUND_ITEM_SCRIPTED 5       /* drawn, moved by an overlay */

#endif
