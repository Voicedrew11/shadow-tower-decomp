/*
 * VS.EXE fighter states and animation slots (VsFighter.state,
 * VsFighterData.anims and .sounds, vs/data.h).
 *
 * Evidence: fighter_update (src/vs/fighter_control.c) runs the state
 * machine: triangle starts the combo (anims[2]), circle and cross chain it
 * (anims[3], anims[4]); square starts the gauge attack when the gauge is
 * full (anims[5], chained by anims[6], anims[7]); circle held guards;
 * fighter_apply_hit and fighter_apply_attack_hit set HURT, or DOWN at no
 * HP (src/vs/fighter_damage.c). The walking moves use anims[0], [1], [10]
 * [11] and [12] (fighter_walk_*). fighters_play_anim_sounds plays sounds[n]
 * with the animation of slot n.
 */
#ifndef VS_FIGHTER_IDS_H
#define VS_FIGHTER_IDS_H

#define FIGHTER_STATE_STAND 0
#define FIGHTER_STATE_WALK 1
#define FIGHTER_STATE_COMBO 2
#define FIGHTER_STATE_HURT 3
#define FIGHTER_STATE_DOWN 4
#define FIGHTER_STATE_GUARD 5
#define FIGHTER_STATE_GAUGE_ATTACK 6

#define FIGHTER_ANIM_STAND 0
#define FIGHTER_ANIM_WALK 1
#define FIGHTER_ANIM_COMBO 2
#define FIGHTER_ANIM_COMBO_CIRCLE 3
#define FIGHTER_ANIM_COMBO_CROSS 4
#define FIGHTER_ANIM_GAUGE_ATTACK 5
#define FIGHTER_ANIM_GAUGE_CIRCLE 6
#define FIGHTER_ANIM_GAUGE_CROSS 7
#define FIGHTER_ANIM_HURT 8
#define FIGHTER_ANIM_DOWN 9
#define FIGHTER_ANIM_WALK_START 10
#define FIGHTER_ANIM_WALK_START_BACK 11 /* played backwards */
#define FIGHTER_ANIM_WALK_BACK 12

#endif
