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
void func_80057b48(int damage);
void func_80057bac(int volume);
void func_80057c30(void);
void func_80057c54(void);
void func_80057c78(void);
void func_80057ca4(void);
void func_80057cc8(void);
void func_80057cec(void);
void func_80057d10(void);
void func_80057d34(void);
void func_80057d58(void);
void func_80057d7c(void);
s16 func_80057fc4(int id, VECTOR *pos, int volume, int range, int fade, int note);
s16 func_80058318(int id, int left, int right, int note);
void func_80058534(short voice, int id, int note);
StSoundBuf *func_80058770(int first, int count, int buf, int bufs, int *index);
void func_800588b4(StCdRequest *request);

#endif
