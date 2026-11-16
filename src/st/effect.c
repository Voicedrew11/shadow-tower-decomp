/*
 * ST.EXE: effect helpers (WIP module): the sound and attack requests the
 * effect script interpreter makes for the current effect, an effect's
 * part vertices, allocating a slot and three slot presets.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/vis.h>
#include <st/funcs.h>
#include <st/anim.h>
#include <st/effect.h>

/* Plays sound `id` at the effect's position. */
ADDRESS(0x80035cc8, 0x3c)
void effect_play_sound(StEffect *e, int id)
{
    func_80058254(id, &e->unk20, 128, 28000, 29000, 0);
}

/*
 * Requests an attack of the current effect's kind from its position
 * (raised by the low 28 bits of `height`, lowered by `drop`) along its
 * velocity, once its countdown has run out; returns the request's result,
 * 0 while counting down or for a kind without attacks.
 */
ADDRESS(0x80035d04, 0x194)
int effect_attack(int arg0, u_long height, int drop)
{
    StEffect *e = effects.cur;
    VisLists *req;
    VECTOR pos;

    if (e->unk8 == 0) {
        pos.vx = e->unk20.vx;
        pos.vy = e->unk20.vy - (height & 0x0fffffff) + drop;
        pos.vz = e->unk20.vz;
        req = &vis_lists;
        req->unk4 = pos;
        req->unk1C = arg0;
        req->unk1E = height;
        req->unk14 = e->unk40;
        req->unk22 = SquareRoot0(e->unk40.vx * e->unk40.vx + e->unk40.vz * e->unk40.vz);
        req->unk20 = ratan2(e->unk40.vz, e->unk40.vx);
        switch (e->unk0 & 7) {
        case 2:
            vis_lists.unk0 = 50;
            break;
        case 1:
            vis_lists.unk0 = 162;
            break;
        case 3:
            vis_lists.unk0 = 178;
            break;
        case 4:
            vis_lists.unk0 = 2;
            break;
        default:
            return 0;
        }
        return func_80049544(req);
    }
    e->unk8--;
    return 0;
}

/*
 * Emits effect output for an effect of kind `kind` with definition `def`:
 * func_8003dbe0 when `flags` has 0x80, and when it has 0x10 (and the
 * target entry faces `pos`, when given) func_8004def8 on the target entry
 * with channel levels from `def` and the viewer. Bits 16-19 of `flags`
 * are options: 0x10000 marks the target, 0x20000 picks the second channel.
 */
ADDRESS(0x80035e98, 0x2b8)
void effect_emit(u_long flags, int kind, StEffectDef *def, int arg, VECTOR *pos)
{
    u16 levels[12];
    u16 bias[6];
    u16 *src;
    u_long options;
    u8 second;
    u8 level;
    u16 base;
    u16 value;
    int result;
    int i;
    Vis212 *target;

    memset(bias, 0, sizeof(bias));
    second = (kind & 0x20) >> 5;
    options = flags & 0x000f0000;
    flags &= ~0x000f0000;
    if (flags & 0x80)
        func_8003dbe0(def->unk4, 4096, def->unk16, pos);
    if (!(flags & 0x10))
        return;
    target = &vis_table_a.entries[vis_lists.unk5C];
    if (pos != NULL) {
        if (!func_8004a148(target->unk8E,
                           func_8004a1b4(target->unk74 - pos->vx, target->unk7C - pos->vz) + 2048,
                           2048))
            return;
    }
    kind &= 0xc0;
    if (options & 0x20000)
        kind |= 0x10;
    else
        kind |= 0x20;
    if (second)
        value = viewer.unk38[12];
    else
        value = viewer.unk38[6];
    level = value / 6;
    if (level < 6) {
        /* As retail: levels 3-5 write past the six bias entries. */
        bias[level * 2] = bias[level * 2 + 1] = viewer.unk78[level];
        base = viewer.unkB8;
    } else {
        base = 0;
    }
    src = level < 6 ? def->unk4 : viewer.unkBA;
    for (i = 0; i < 3; i++, src++)
        levels[i] = *src;
    src = def->unkA;
    for (i = 0; i < 6; i++, src++) {
        if (*src == 0)
            levels[3 + i] = 0;
        else
            levels[3 + i] = *src + base + bias[i] / 6;
    }
    if (level < 6)
        result = func_8004def8(target, 1, arg, levels, 1);
    else
        result = func_8004def8(target, 1, arg, levels, 0);
    if (kind & 0x40)
        func_80026a88(result);
    if (options & 0x10000)
        target->unk70 |= 0x800;
}

/* effect_emit for the current effect. */
ADDRESS(0x80036150, 0x3c)
void effect_emit_current(u_long flags, int arg, VECTOR *pos)
{
    effect_emit(flags, effects.cur->unk0, effects.def, arg, pos);
}

/* effect_emit for the current effect from a point behind it along its
 * velocity. */
ADDRESS(0x8003618c, 0x8c)
void effect_emit_behind(u_long flags)
{
    StEffect *e = effects.cur;
    VECTOR pos;

    pos.vx = e->unk20.vx - e->unk40.vx * 8;
    pos.vy = e->unk20.vy - e->unk40.vy * 8;
    pos.vz = e->unk20.vz - e->unk40.vz * 8;
    effect_emit_current(flags, (u8)(e->unk1 - 44) < 4 ? e->unk52 : 5120, &pos);
}

/* Vertex `index` of the effect's model, scaled by `scale` (4096ths) and
 * turned by its rotation (half a turn added to the yaw), into *out. */
ADDRESS(0x80036218, 0xe8)
void effect_vertex(StEffect *e, int index, VECTOR *out, SVECTOR *scale)
{
    SVECTOR rot;
    SVECTOR v;

    anim_get_vertex(e->unkA + 40, e->unk2, e->unk14, index, &v);
    v.vx = (v.vx * scale->vx) >> 12;
    v.vy = (v.vy * scale->vy) >> 12;
    v.vz = (v.vz * scale->vz) >> 12;
    rot.vx = e->unk30.vx;
    rot.vy = e->unk30.vy + 2048;
    rot.vz = e->unk30.vz;
    func_8004a078(&rot, &v, out);
}

/* effect_vertex moved to the effect's position. */
ADDRESS(0x80036300, 0x70)
void effect_vertex_world(StEffect *e, int index, VECTOR *out, SVECTOR *scale)
{
    effect_vertex(e, index, out, scale);
    out->vx += e->unk20.vx;
    out->vy += e->unk20.vy;
    out->vz += e->unk20.vz;
}

/* A free effect slot, or NULL. */
ADDRESS(0x80036370, 0x44)
StEffect *effect_alloc(void)
{
    StEffect *e = effects.slots;
    u16 i;

    i = ST_EFFECT_SLOTS;
    do {
        if (e->unk0 == ST_EFFECT_FREE)
            return e;
        e++;
    } while (--i);
    return NULL;
}

/* Slot presets (no direct callers). */
ADDRESS(0x800363b4, 0x38)
void effect_preset_1(StEffect *e, s16 a, s16 scale, u8 b)
{
    e->unk6 = 1;
    e->unk2 = 128;
    e->unk9 = 52;
    e->unkA = a;
    e->unk7 = b;
    e->unk12 = 4096;
    e->unk38.vx = e->unk38.vy = e->unk38.vz = scale;
}

ADDRESS(0x800363ec, 0x38)
void effect_preset_5(StEffect *e, s16 a, s16 scale, u8 b)
{
    e->unk6 = 5;
    e->unk2 = 128;
    e->unk9 = 51;
    e->unkA = a;
    e->unk7 = b;
    e->unk12 = 4096;
    e->unk38.vx = e->unk38.vy = e->unk38.vz = scale;
}

ADDRESS(0x80036424, 0x60)
void effect_preset_14(StEffect *e, s16 a, s16 yaw, int height)
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
