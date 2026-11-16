/*
 * VS.EXE: the empty function VS's effect_emit_current calls where ST.EXE's
 * calls effect_emit (WIP module); VS emits no effect sub-effects.
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
void func_80032200(u_long flags, int kind, StEffectDef *def, int arg, VECTOR *pos)
{
}
