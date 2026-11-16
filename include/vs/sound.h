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

/* A sound bank record: its VAB header, VAB id and body transfer result. */
typedef struct {
    u_char head[4096];   /* 0x0000: the VAB header */
    s16 vab;             /* 0x1000: SsVabOpenHeadSticky's id, -1 on failure */
    s16 unk1002;         /* 0x1002: SsVabTransBodyPartly's result */
} VsSoundBank;

typedef char VsSoundBankSize[sizeof(VsSoundBank) == 0x1004 ? 1 : -1];

/* The two sound banks (0x801604b0, bss). */
extern VsSoundBank DAT_801604b0[2];

/* libsnd's SEQ attribute table (0x801175a8, bss) and SpuGetAllKeysStatus's
 * 24 voice states (0x80117580, bss). */
extern char DAT_801175a8[];
extern char DAT_80117580[24];

/* src/vs/card_8001c8b8.c: the menu sound effects (SsUtKeyOn tones 11-14). */
void func_8001c8b8(void);
void func_8001c8f8(void);
void func_8001c938(void);
void func_8001c978(void);

#endif
