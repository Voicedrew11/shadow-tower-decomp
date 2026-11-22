/*
 * ST.EXE sound functions (src/st/sound.c, WIP module): the sound system's
 * start and stop, the one-shot and positional sound players, the listener,
 * and the VAB bank loads through the CD request ring. The sound state block
 * itself is st/data.h's Data801dd1c0. The band's functions that other
 * modules already call keep their st/funcs.h and st/player.h prototypes.
 */
#ifndef ST_SOUND_H
#define ST_SOUND_H

#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/data.h>
#include <st/cd.h>

/* src/st/sound.c */
void play_landing_sound(int damage);
void play_footstep_sound(int volume);
void func_80057c30(void);
void play_warp_sound(void);
void func_80057c78(void);
void func_80057ca4(void);
void play_confirm_sound(void);
void play_cancel_sound(void);
void play_cursor_sound(void);
void play_refuse_sound(void);
void play_unlock_sound(void);
void play_locked_sound(void);
s16 sound_play_positional(int id, VECTOR *pos, int volume, int range, int fade, int note);
s16 sound_key_on(int id, int left, int right, int note);
void sound_key_off(short voice, int id, int note);
StSoundBuf *sound_bank_find_buffer(int first, int count, int buf, int bufs, int *index);
void sound_bank_read_done(StCdRequest *request);

#endif
