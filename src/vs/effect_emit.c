/*
 * VS.EXE: the empty function VS's effect_emit_current calls where ST.EXE's
 * calls effect_emit, and an effect flag test (WIP module); VS emits no
 * effect sub-effects.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/effect.h>
#include <vs/effect.h>

ADDRESS(0x80032200, 0x8)
void effect_emit(u_long flags, int kind, StEffectDef *def, int arg, VECTOR *pos)
{
}

/* 0 for an effect whose flag 0x40 is set, else 5. */
ADDRESS(0x80032208, 0x20)
int func_80032208(StEffect *e)
{
    int kind = e->unk0 & 0x40;

    if (kind)
        kind = 0;
    else
        kind = 5;
    return kind;
}
