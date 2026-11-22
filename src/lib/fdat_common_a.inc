/*
 * The first ten of the 21 functions every FDAT overlay starts with (shared
 * fragment): built by src/lib/fdat_common_a.c and included in place by the
 * fused overlay sources. The claims are FDAT_002.BIN's addresses.
 */

/* Clears the entry's unkBC. */
ADDRESS(0x801eb7ac, 0x8)
void creature_clear_contact_hit(Creature *e)
{
    e->contact_hit = 0;
}

/* Starts script `id` of the entry's record. */
ADDRESS(0x801eb7b4, 0x60)
void creature_start_script(Creature *e, int id)
{
    CreatureType *r = &vis_table_a.types[e->type];

    e->script_index = id;
    e->script = r->scripts[id];
    e->command = e->script->command;
    e->anim = e->script->anim;
    e->anim_step = e->script->anim_step;
    e->script_state = 0;
    e->anim_frame = 0;
}

/* Turns the current entry towards `angle` by at most `step`. */
ADDRESS(0x801eb814, 0x6c)
void creature_turn_toward(short angle, int step)
{
    Creature *e = vis_table_a.current;
    int d = (angle - e->facing) & 0xfff;
    int a;

    if (d > 2048)
        d -= 4096;
    a = abs(d);
    if (a < step)
        step = a;
    if (d != 0) {
        if (d > 0)
            e->facing += step;
        else
            e->facing -= step;
    }
}

/* Sets the current entry's horizontal velocity: `speed` along `angle`. */
ADDRESS(0x801eb880, 0x74)
void creature_set_velocity(short angle, int speed)
{
    Creature *e = vis_table_a.current;

    e->velocity.vx = speed * rcos(angle) >> 12;
    e->velocity.vz = speed * rsin(angle) >> 12;
}

/* Steers the current entry's vertical speed towards height `y`. */
ADDRESS(0x801eb8f4, 0x4c)
void creature_steer_height(int y, int step)
{
    Creature *e = vis_table_a.current;

    y -= e->position.vy;
    if (y > 0)
        e->vertical_speed += step;
    else if (y < 0)
        e->vertical_speed -= step;
}

/* A random note offset, -2..2. */
ADDRESS(0x801eb940, 0x2c)
int creature_random_note(Creature *e)
{
    return (rand() * 5 >> 15) - 2;
}

/* Plays the entry's script sound. */
ADDRESS(0x801eb96c, 0xe0)
void creature_emit_script_sound(Creature *e)
{
    int sound = e->script->sound;

    e->sound_id = e->model_slot * 10 + 48 + sound;
    if (e->flags & 0x800000)
        e->voice = sound_play_at_range(e->model_slot * 10 + 48 + sound, &e->position, 127, 0x6000, 0x7800,
                                 creature_random_note(e));
    else
        e->voice = sound_play_at(e->model_slot * 10 + 48 + sound, &e->position, 127, creature_random_note(e));
}

/* Plays the script sound when its trigger fires this frame. */
ADDRESS(0x801eba4c, 0x17c)
void creature_update_script_sound(Creature *e)
{
    CreatureScript *s = e->script;
    int sound = s->sound;
    u16 trigger = s->sound_trigger;
    u16 frame = e->anim_frame;
    u16 step = s->anim_step;
    int at;
    int chance;

    if (sound == 255)
        return;
    at = trigger & 0xfff;
    switch (trigger & 0xf000) {
    case 0x1000:
        if (at >= (short)frame - (short)step && at < (short)frame)
            creature_emit_script_sound(e);
        break;
    case 0x2000:
        at = trigger & 0x7ff;
        if (at >= (frame & 0x7ff) - (short)step && at < (frame & 0x7ff))
            creature_emit_script_sound(e);
        break;
    case 0x8000:
        chance = at;
        at = 0;
        if (chance >= (short)frame - (short)step && chance < (short)frame
            && rand() >> 3 < chance)
            creature_emit_script_sound(e);
        break;
    case 0x4000:
        if (cd_state.rate_vsync / 20 % at == 0)
            creature_emit_script_sound(e);
        break;
    }
}

/* Hits the player with the script's damage. */
ADDRESS(0x801ebbc8, 0xc0)
void creature_hit_player(Creature *e, CreatureScript *s)
{
    u16 damage[9];
    u16 effect;
    int i;

    effect = 0;
    damage[0] = s->arg.u[5];
    damage[1] = s->arg.u[6];
    damage[2] = s->arg.u[7];
    for (i = 5; i != -1; i--) {
        if ((s->arg.u[8] >> i) & 1)
            damage[3 + i] = s->arg.u[9];
        else
            damage[3 + i] = 0;
    }
    for (i = 0; i < 8; i++) {
        if ((s->arg.u[8] >> (i + 8)) & 1)
            effect |= 1 << i;
    }
    player_hit(damage, 4096, effect, (long *)&e->position);
}

/* Spawns the hits of attack script `s` whose frames the animation passes. */
ADDRESS(0x801ebc88, 0x23c)
void creature_spawn_attack_hits(Creature *e, CreatureScript *s)
{
    VECTOR pos;
    SVECTOR v;
    VECTOR node;
    SVECTOR off;
    MATRIX m;
    SVECTOR rot;
    StAnimFile *file;
    int id = e->model_slot + 128;
    int i;
    int plain;

    file = world_state.model_files[id];
    if (file == NULL)
        return;
    plain = file->node_count == 0;
    for (i = 2; i != -1; i--) {
        if (s->arg.hit.frame[i] == 0xffff)
            continue;
        if (!((short)e->anim_frame - e->anim_step < s->arg.hit.frame[i]))
            continue;
        if ((short)e->anim_frame < s->arg.hit.frame[i])
            continue;
        pos = e->position;
        rot.vx = 0;
        rot.vy = -e->facing;
        rot.vz = 0;
        RotMatrix(&rot, &m);
        if (s->arg.hit.vertex[i] != 0xffff) {
            if (plain) {
                anim_get_vertex(id, e->anim, (short)e->anim_frame, s->arg.hit.vertex[i], &v);
            } else {
                anim_set_node_vertex((StAnimSet *)e->anim_slot, id, s->arg.hit.node[i],
                                     s->arg.hit.vertex[i], &node);
                v.vx = node.vx;
                v.vy = node.vy;
                v.vz = node.vz;
            }
            ApplyMatrixSV(&m, &v, &v);
            pos.vx += v.vx;
            pos.vy += v.vy;
            pos.vz += v.vz;
        }
        off.vx = s->arg.hit.offset[0];
        off.vy = s->arg.hit.offset[1];
        off.vz = s->arg.hit.offset[2];
        ApplyMatrixSV(&m, &off, &off);
        pos.vx += off.vx;
        pos.vy += off.vy;
        pos.vz += off.vz;
        effect_spawn_attack(e, s->arg.hit.effect, &pos);
        e->attack_miss = 0;
    }
}
