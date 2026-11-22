/*
 * ST.EXE player states and status bits (viewer.state, viewer.status_flags,
 * st/data.h).
 */
#ifndef ST_PLAYER_IDS_H
#define ST_PLAYER_IDS_H

/* viewer.state. player_update reads input only in NORMAL, QUICK_ITEM,
 * DYING and HURT; the others hold the player while something else runs. */
#define VIEWER_STATE_NORMAL 0      /* walk, hands, examine (player_enter_normal_state) */
#define VIEWER_STATE_QUICK_ITEM 1  /* choosing one of the five quick items */
#define VIEWER_STATE_FROZEN 2      /* player_enter_frozen_state (the ending overlays) */
#define VIEWER_STATE_PICK_UP 3     /* ground_item_pick_up holds an item in view */
#define VIEWER_STATE_DYING 4       /* player_kill */
#define VIEWER_STATE_SCRIPTED 5    /* warps and the overlays' door and event scenes */
#define VIEWER_STATE_SHOP 6        /* map_enter_shop (FDAT_442..472) */
#define VIEWER_STATE_HURT 16       /* player_enter_hurt_state, until the view settles */

/*
 * viewer.status_flags. Evidence: the pause menu's status line
 * (menu_main_draw) shows STAT.T messages 20..24 (poison, paralyze, dark,
 * slow, cursed) by these bits; player_update and player_update_status run
 * their timers (viewer.timers, in the same order) and effects. A hit's
 * effect code (player_hit) uses the same bits for the status it inflicts.
 */
#define STATUS_POISON_1 0x1        /* HP -16 every 20 frames */
#define STATUS_POISON_2 0x2        /* HP -64 */
#define STATUS_POISON_3 0x4        /* HP -256 */
#define STATUS_POISON 0x7
#define STATUS_PARALYZE_1 0x8      /* no walking */
#define STATUS_PARALYZE_2 0x10     /* no turning, casting or menu either */
#define STATUS_PARALYZE 0x18
#define STATUS_DARK 0x60
#define STATUS_SLOW 0x80
#define STATUS_CURSED 0x100
#define STATUS_FLOOR_HIT 0x400     /* a damaging floor hit this frame */
#define STATUS_FIRE_WARD 0x80000   /* quarters fire damage */
#define STATUS_POISON_WARD 0x100000 /* blocks poison */
#define STATUS_ATTACK_BOOST 0x200000 /* the player's hits on creatures x3/2 */
#define STATUS_ACID_WARD 0x800000  /* quarters acid damage and halves acid wear */
#define STATUS_DURABILITY_WARNING 0x1000000

#endif
