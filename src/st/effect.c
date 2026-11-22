/*
 * ST.EXE: effect helpers (WIP module): the sound and attack requests the
 * per-kind effect update makes for the current effect, an effect's
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
#include <st/hud.h>
#include <st/funcs.h>
#include <st/anim.h>
#include <st/effect.h>

/* Plays sound `id` at the effect's position. */
ADDRESS(0x80035cc8, 0x3c)
void effect_play_sound(StEffect *e, int id)
{
    sound_play_at_range(id, &e->position, 128, 28000, 29000, 0);
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
    CollisionState *req;
    VECTOR pos;

    if (e->attack_delay == 0) {
        pos.vx = e->position.vx;
        pos.vy = e->position.vy - (height & 0x0fffffff) + drop;
        pos.vz = e->position.vz;
        req = &vis_lists;
        req->position = pos;
        req->radius = arg0;
        req->height = height;
        req->velocity = e->velocity;
        req->speed = SquareRoot0(e->velocity.vx * e->velocity.vx + e->velocity.vz * e->velocity.vz);
        req->heading = ratan2(e->velocity.vz, e->velocity.vx);
        switch (e->flags & 7) {
        case 2:
            vis_lists.tests = 50;
            break;
        case 1:
            vis_lists.tests = 162;
            break;
        case 3:
            vis_lists.tests = 178;
            break;
        case 4:
            vis_lists.tests = 2;
            break;
        default:
            return 0;
        }
        return collision_query(req);
    }
    e->attack_delay--;
    return 0;
}

/*
 * Emits effect output for an effect of kind `kind` with definition `def`:
 * player_hit when `flags` has 0x80, and when it has 0x10 (and the
 * target entry faces `pos`, when given) creature_take_damage on the target entry
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
    Creature *target;

    memset(bias, 0, sizeof(bias));
    second = (kind & 0x20) >> 5;
    options = flags & 0x000f0000;
    flags &= ~0x000f0000;
    if (flags & 0x80)
        player_hit(def->physical_damage, 4096, def->hit_effect, (long *)pos);
    if (!(flags & 0x10))
        return;
    target = &vis_table_a.entries[vis_lists.creature_in_reach];
    if (pos != NULL) {
        if (!angle_within(target->facing,
                           angle_from_xz(target->position.vx - pos->vx, target->position.vz - pos->vz) + 2048,
                           2048))
            return;
    }
    kind &= 0xc0;
    if (options & 0x20000)
        kind |= 0x10;
    else
        kind |= 0x20;
    if (second)
        value = viewer.equipment[EQUIP_SLOT_LEFT_MAGIC];
    else
        value = viewer.equipment[EQUIP_SLOT_RIGHT_MAGIC];
    level = value / 6;
    if (level < 6) {
        /* As retail: levels 3-5 write past the six bias entries. */
        bias[level * 2] = bias[level * 2 + 1] = (u16)viewer.attributes[level + 11];
        base = viewer.magic_base;
    } else {
        base = 0;
    }
    src = level < 6 ? def->physical_damage : viewer.weapon_attack;
    for (i = 0; i < 3; i++, src++)
        levels[i] = *src;
    src = def->element_damage;
    for (i = 0; i < 6; i++, src++) {
        if (*src == 0)
            levels[3 + i] = 0;
        else
            levels[3 + i] = *src + base + bias[i] / 6;
    }
    if (level < 6)
        result = creature_take_damage(target, 1, arg, levels, 1);
    else
        result = creature_take_damage(target, 1, arg, levels, 0);
    if (kind & 0x40)
        hud_queue_damage_number(result);
    if (options & 0x10000)
        target->flags |= 0x800;
}

/* effect_emit for the current effect. */
ADDRESS(0x80036150, 0x3c)
void effect_emit_current(u_long flags, int arg, VECTOR *pos)
{
    effect_emit(flags, effects.cur->flags, effects.def, arg, pos);
}

/* effect_emit for the current effect from a point behind it along its
 * velocity. */
ADDRESS(0x8003618c, 0x8c)
void effect_emit_behind(u_long flags)
{
    StEffect *e = effects.cur;
    VECTOR pos;

    pos.vx = e->position.vx - e->velocity.vx * 8;
    pos.vy = e->position.vy - e->velocity.vy * 8;
    pos.vz = e->position.vz - e->velocity.vz * 8;
    effect_emit_current(flags, (u8)(e->kind - 44) < 4 ? e->state2.h : 5120, &pos);
}

/* Vertex `index` of the effect's model, scaled by `scale` (4096ths) and
 * turned by its rotation (half a turn added to the yaw), into *out. */
ADDRESS(0x80036218, 0xe8)
void effect_vertex(StEffect *e, int index, VECTOR *out, SVECTOR *scale)
{
    SVECTOR rot;
    SVECTOR v;

    anim_get_vertex(e->model + 40, e->part, e->anim_frame, index, &v);
    v.vx = (v.vx * scale->vx) >> 12;
    v.vy = (v.vy * scale->vy) >> 12;
    v.vz = (v.vz * scale->vz) >> 12;
    rot.vx = e->rotation.vx;
    rot.vy = e->rotation.vy + 2048;
    rot.vz = e->rotation.vz;
    vector_rotate(&rot, &v, out);
}

/* effect_vertex moved to the effect's position. */
ADDRESS(0x80036300, 0x70)
void effect_vertex_world(StEffect *e, int index, VECTOR *out, SVECTOR *scale)
{
    effect_vertex(e, index, out, scale);
    out->vx += e->position.vx;
    out->vy += e->position.vy;
    out->vz += e->position.vz;
}

/* A free effect slot, or NULL. */
ADDRESS(0x80036370, 0x44)
StEffect *effect_alloc(void)
{
    StEffect *e = effects.slots;
    u16 i;

    i = ST_EFFECT_SLOTS;
    do {
        if (e->flags == ST_EFFECT_FREE)
            return e;
        e++;
    } while (--i);
    return NULL;
}

/* Slot presets, called by the spawner (src/st/effect_spawn.c) without
 * prototypes. */
ADDRESS(0x800363b4, 0x38)
void effect_preset_1(e, a, scale, b)
    StEffect *e;
    s16 a;
    s16 scale;
    u8 b;
{
    e->draw_mode = 1;
    e->part = 128;
    e->unk9 = 52;
    e->model = a;
    e->draw_flags = b;
    e->light = 4096;
    e->scale.vx = e->scale.vy = e->scale.vz = scale;
}

ADDRESS(0x800363ec, 0x38)
void effect_preset_5(e, a, scale, b)
    StEffect *e;
    s16 a;
    s16 scale;
    u8 b;
{
    e->draw_mode = 5;
    e->part = 128;
    e->unk9 = 51;
    e->model = a;
    e->draw_flags = b;
    e->light = 4096;
    e->scale.vx = e->scale.vy = e->scale.vz = scale;
}

ADDRESS(0x80036424, 0x60)
void effect_preset_14(e, a, yaw, height)
    StEffect *e;
    s16 a;
    s16 yaw;
    int height;
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
