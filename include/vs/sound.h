/*
 * VS.EXE's menu sound effects and their VAB bank. Only the globals and
 * functions the reconstructed code proves are declared.
 */
#ifndef VS_SOUND_H
#define VS_SOUND_H

#include <kf/lib/types.h>
#include <sys/types.h>

/* The menu sound bank's VAB id (0x8006335a, bss). */
extern s16 DAT_8006335a;

/* The bank's VAB body and header buffers (bss). */
extern u_char DAT_800ff580[];
extern u_char DAT_80116580[];

/* src/vs/card_8001c8b8.c: the menu sound effects (SsUtKeyOn tones 11-14). */
void func_8001c8b8(void);
void func_8001c8f8(void);
void func_8001c938(void);
void func_8001c978(void);

#endif
