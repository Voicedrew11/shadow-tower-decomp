/*
 * VS.EXE: emitting from the current effect, slot allocation and slot
 * presets (WIP module). VS's copies of ST.EXE's src/st/effect.c functions.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/effect.h>
#include <vs/effect.h>

/* effect_emit for the current effect. */
ADDRESS(0x80032228, 0x3c)
void effect_emit_current(u_long flags, int arg, VECTOR *pos)
{
    effect_emit(flags, vs_effects.cur->flags, vs_effects.def, arg, pos);
}

/* effect_emit_current from a point behind the current effect along its velocity. */
ADDRESS(0x80032264, 0x8c)
void effect_emit_behind(u_long flags)
{
    StEffect *e = vs_effects.cur;
    VECTOR pos;

    pos.vx = e->position.vx - e->velocity.vx * 8;
    pos.vy = e->position.vy - e->velocity.vy * 8;
    pos.vz = e->position.vz - e->velocity.vz * 8;
    effect_emit_current(flags, (u8)(e->kind - 44) < 4 ? e->state2.h : 5120, &pos);
}

/* A free effect slot, or NULL. */
ADDRESS(0x800322f0, 0x44)
StEffect *effect_alloc(void)
{
    StEffect *e = vs_effects.slots;
    u16 i;

    i = ST_EFFECT_SLOTS;
    do {
        if (e->flags == ST_EFFECT_FREE)
            return e;
        e++;
    } while (--i);
    return NULL;
}

/* Slot presets. */
ADDRESS(0x80032334, 0x38)
void effect_preset_1(StEffect *e, s16 a, s16 scale, u8 b)
{
    e->draw_mode = 1;
    e->part = 128;
    e->unk9 = 52;
    e->model = a;
    e->draw_flags = b;
    e->light = 4096;
    e->scale.vx = e->scale.vy = e->scale.vz = scale;
}

ADDRESS(0x8003236c, 0x38)
void effect_preset_5(StEffect *e, s16 a, s16 scale, u8 b)
{
    e->draw_mode = 5;
    e->part = 128;
    e->unk9 = 51;
    e->model = a;
    e->draw_flags = b;
    e->light = 4096;
    e->scale.vx = e->scale.vy = e->scale.vz = scale;
}

ADDRESS(0x800323a4, 0x60)
void effect_preset_14(StEffect *e, s16 a, s16 yaw, int height)
{
    e->draw_mode = 14;
    e->part = 128;
    e->draw_flags = 5;
    e->unk9 = 52;
    e->light = 4096;
    e->scale.vx = e->scale.vz = e->scale.vy = 256;
    e->model = a;
    e->position.vx = 160;
    e->position.vy = height + 120;
    e->position.vz = 0;
    e->rotation.vx = yaw;
    e->rotation.vy = e->rotation.vz = 0;
}
