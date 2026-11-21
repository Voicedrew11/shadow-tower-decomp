/*
 * VS.EXE effect pool. The definition and effect records are ST.EXE's
 * (st/effect.h); VS's pool at 0x801ce698 holds 96 definitions (ST: 176)
 * before the same 128 effects and the interpreter's current ones.
 */
#ifndef VS_EFFECT_H
#define VS_EFFECT_H

#include <kf/lib/types.h>
#include <kf/lib/offsetof.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/effect.h>

typedef struct {
    StEffectDef defs[96];
    StEffect slots[ST_EFFECT_SLOTS]; /* 0x900 */
    StEffectDef *def;    /* 0x3500: the current effect's definition */
    StEffect *cur;       /* 0x3504: the effect being run */
    int index;           /* 0x3508: its slot index */
} VsEffects;

/* Layout check: the offset the code proves. */
typedef char VsEffectsCurAt0x3504[offsetof(VsEffects, cur) == 0x3504 ? 1 : -1];

extern VsEffects DAT_801ce698;

/* src/vs/effect_80032228.c: a free effect slot, or NULL */
StEffect *func_800322f0(void);
/* src/vs/effect_80032200.c: an empty stand-in for ST.EXE's effect_emit */
void func_80032200(u_long flags, int kind, StEffectDef *def, int arg, VECTOR *pos);
/* VS's effect attack (ST effect_attack's place; not reconstructed) */
int func_80032110(int arg0, u_long height);
/* src/vs/effect_spawn_80032404.c: spawns an effect for fighter n's attack
 * (ST.EXE's func_80036484); the StEffect or NULL */
StEffect *func_80032404(int n, u8 attack, u8 flags, u8 kind, VECTOR *pos, SVECTOR *dir, ...);
/* src/vs/effect_80032404.c: the effect script interpreter (ST func_800385b0's place) */
void func_800335e8(void);

#endif
