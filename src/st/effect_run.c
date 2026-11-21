/*
 * ST.EXE: clearing and running the effect pool (WIP module, after the
 * effect spawner, mover and script interpreter unit src/st/effect_spawn.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/funcs.h>
#include <st/effect.h>

ADDRESS(0x8003bc10, 0x30)
void effects_clear(void)
{
    StEffect *e = effects.slots;
    u16 i;

    for (i = 0; i < ST_EFFECT_SLOTS; i++) {
        e->unk0 = ST_EFFECT_FREE;
        e++;
    }
}

/* Runs every live effect for one frame through the interpreter, freeing
 * those whose time is up. */
ADDRESS(0x8003bc40, 0xd4)
void effects_run(void)
{
    StEffect *e = effects.slots;

    for (effects.index = 0; effects.index < ST_EFFECT_SLOTS; effects.index++) {
        if (e->unk0 != ST_EFFECT_FREE) {
            effects.cur = e;
            effects.def = &effects.defs[e->unk1];
            if (--e->unk10 == -1)
                e->unk0 = ST_EFFECT_FREE;
            else
                func_800385b0();
        }
        e++;
    }
}
