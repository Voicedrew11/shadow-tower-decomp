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

/* func_80032200 for the current effect. */
ADDRESS(0x80032228, 0x3c)
void func_80032228(u_long flags, int arg, VECTOR *pos)
{
    func_80032200(flags, DAT_801ce698.cur->unk0, DAT_801ce698.def, arg, pos);
}

/* func_80032228 from a point behind the current effect along its velocity. */
ADDRESS(0x80032264, 0x8c)
void func_80032264(u_long flags)
{
    StEffect *e = DAT_801ce698.cur;
    VECTOR pos;

    pos.vx = e->unk20.vx - e->unk40.vx * 8;
    pos.vy = e->unk20.vy - e->unk40.vy * 8;
    pos.vz = e->unk20.vz - e->unk40.vz * 8;
    func_80032228(flags, (u8)(e->unk1 - 44) < 4 ? e->unk52 : 5120, &pos);
}

/* A free effect slot, or NULL. */
ADDRESS(0x800322f0, 0x44)
StEffect *func_800322f0(void)
{
    StEffect *e = DAT_801ce698.slots;
    u16 i;

    i = ST_EFFECT_SLOTS;
    do {
        if (e->unk0 == ST_EFFECT_FREE)
            return e;
        e++;
    } while (--i);
    return NULL;
}

/* Slot presets. */
ADDRESS(0x80032334, 0x38)
void func_80032334(StEffect *e, s16 a, s16 scale, u8 b)
{
    e->unk6 = 1;
    e->unk2 = 128;
    e->unk9 = 52;
    e->unkA = a;
    e->unk7 = b;
    e->unk12 = 4096;
    e->unk38.vx = e->unk38.vy = e->unk38.vz = scale;
}

ADDRESS(0x8003236c, 0x38)
void func_8003236c(StEffect *e, s16 a, s16 scale, u8 b)
{
    e->unk6 = 5;
    e->unk2 = 128;
    e->unk9 = 51;
    e->unkA = a;
    e->unk7 = b;
    e->unk12 = 4096;
    e->unk38.vx = e->unk38.vy = e->unk38.vz = scale;
}

ADDRESS(0x800323a4, 0x60)
void func_800323a4(StEffect *e, s16 a, s16 yaw, int height)
{
    e->unk6 = 14;
    e->unk2 = 128;
    e->unk7 = 5;
    e->unk9 = 52;
    e->unk12 = 4096;
    e->unk38.vx = e->unk38.vz = e->unk38.vy = 256;
    e->unkA = a;
    e->unk20.vx = 160;
    e->unk20.vy = height + 120;
    e->unk20.vz = 0;
    e->unk30.vx = yaw;
    e->unk30.vy = e->unk30.vz = 0;
}
