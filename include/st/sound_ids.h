/*
 * ST.EXE fixed sound effect ids (sound_play, src/st/sound.c): voices of the
 * common sound bank. Named by the code that plays them; ids not listed
 * stay numeric.
 */
#ifndef ST_SOUND_IDS_H
#define ST_SOUND_IDS_H

#define SOUND_DEATH 3              /* play_death_sound */
#define SOUND_ITEM_BROKEN 4        /* player_wear_item: durability reached 0 */
#define SOUND_POISON_TICK 5        /* the poison timer's HP loss */
#define SOUND_PARALYSIS_TICK 6     /* every 20 paralysed frames */
#define SOUND_FLOOR_DAMAGE 7       /* a damaging floor, every 40 frames */
#define SOUND_RECOVER 8            /* HP restored (item_use, healing spots) */
#define SOUND_WARP 10
#define SOUND_CURSOR 11
#define SOUND_CONFIRM 12
#define SOUND_CANCEL 13
#define SOUND_REFUSE 14
#define SOUND_LOCKED 15
#define SOUND_UNLOCK 16
#define SOUND_FOOTSTEP 17          /* play_footstep_sound: floor kind 1 and the default */
#define SOUND_FOOTSTEP_2 18        /* floor kind 2 */
#define SOUND_FOOTSTEP_3 19        /* floor bits 0x34 */
#define SOUND_LANDING 20
#define SOUND_DOUBLE_DAMAGE 21     /* creature_take_damage's luck roll */

#endif
