/*
 * VS.EXE: clearing and running the effect pool (WIP module). VS's copies of
 * ST.EXE's src/st/effect_run.c functions.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/effect.h>
#include <vs/effect.h>

ADDRESS(0x800352cc, 0x30)
void func_800352cc(void)
{
    StEffect *e = DAT_801ce698.slots;
    u16 i;

    for (i = 0; i < ST_EFFECT_SLOTS; i++) {
        e->unk0 = ST_EFFECT_FREE;
        e++;
    }
}

/* Runs every live effect for one frame through the interpreter, freeing
 * those whose time is up. */
ADDRESS(0x800352fc, 0xd4)
void func_800352fc(void)
{
    StEffect *e = DAT_801ce698.slots;

    for (DAT_801ce698.index = 0; DAT_801ce698.index < ST_EFFECT_SLOTS; DAT_801ce698.index++) {
        if (e->unk0 != ST_EFFECT_FREE) {
            DAT_801ce698.cur = e;
            DAT_801ce698.def = &DAT_801ce698.defs[e->unk1];
            if (--e->unk10 == -1)
                e->unk0 = ST_EFFECT_FREE;
            else
                func_800335e8();
        }
        e++;
    }
}
