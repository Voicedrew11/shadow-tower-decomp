/*
 * The last nine of the 21 functions every FDAT overlay starts with (shared
 * fragment): built by src/lib/fdat_common_b.c and included in place by the
 * fused overlay sources. The claims are FDAT_002.BIN's addresses.
 */

/*
 * Picks one of the current record's scripts allowed in the entry's state
 * by weight and starts it; 0 when none is allowed.
 */
ADDRESS(0x801ecbcc, 0x2b0)
int creature_pick_weighted_script(void)
{
    int limit = 24;
    CreatureScript *list[24];
    int index[24];
    CreatureType *r = vis_table_a.current_type;
    Creature *e = vis_table_a.current;
    CreatureScript **p = r->scripts;
    CreatureScript *s;
    CreatureScript *prev;
    int count = 0;
    int total = 0;
    int pick;
    int i;

    for (i = 0; i < limit; i++) {
        s = *p++;
        if (s == NULL)
            break;
        if (s->actions & e->action) {
            index[count] = i;
            list[count] = s;
            count++;
            total += s->weight;
        }
    }
    if (count == 0)
        return 0;
    pick = rand() * total >> 15;
    count--;
    while (count != -1) {
        pick -= list[count]->weight;
        if (pick <= 0)
            break;
        count--;
    }
    if (count < 0)
        count = 0;
    e->script_index = index[count];
    prev = e->chain_script;
    e->script = list[count];
    e->command = e->script->command;
    e->anim_step = e->script->anim_step;
    if (prev != NULL && e->script != prev) {
        if (prev->command == 17 && prev->arg.b[9] != 255)
            e->script_index = prev->arg.b[9];
        else if (e->chain_script->command == 20 && e->chain_script->arg.b[13] != 255)
            e->script_index = e->chain_script->arg.b[13];
        e->script = r->scripts[e->script_index];
        e->anim_step = e->script->anim_step;
        e->command = e->script->command;
        e->chain_script = NULL;
    }
    if ((e->command == 17 || e->command == 20) && e->chain_script == NULL) {
        e->chain_script = e->script;
        if (e->command == 17 && e->script->arg.b[8] != 255)
            e->script_index = e->script->arg.b[8];
        else if (e->command == 20 && e->script->arg.b[12] != 255)
            e->script_index = e->script->arg.b[12];
        e->script = r->scripts[e->script_index];
        e->anim_step = e->script->anim_step;
        e->command = e->script->command;
    }
    e->script_state = 0;
    if (e->anim_step >= 0)
        e->anim_frame = 0;
    else
        e->anim_frame = 4096;
    return 1;
}

/* Counts the entry's unk34 down by its step; 1 once it runs out. */
ADDRESS(0x801ece7c, 0xc8)
int creature_step_hurt_timer(Creature *e)
{
    if (e->hurt_timer < 0)
        return 0;
    e->hurt_timer -= abs(e->anim_step);
    if (e->hurt_timer < 0) {
        if (e->hp == 0) {
            if ((short)e->anim_frame >= 4095)
                creature_despawn(e);
            return 1;
        }
        e->hurt_timer = -1;
        e->status.bytes[2] &= 0xdf;
    } else if ((short)e->anim_frame >= 4096 && e->state == 5) {
        e->anim_frame = 4095;
    }
    return 0;
}

/* Fades the entry out over 32 frames, then removes it. */
ADDRESS(0x801ecf44, 0x80)
void creature_fade_out_despawn(Creature *e)
{
    e->fade = 0;
    e->status.bytes[2] |= 0xd;
    while (1) {
        e->fade += 128;
        if (e->fade > 4096)
            break;
        world_run_frame(NULL, (SVECTOR *)&viewer.pitch, 0);
    }
    creature_despawn(e);
}

/*
 * Moves the view to `pos`, turned to `rot`, over 16 frames.
 */
ADDRESS(0x801ecfc4, 0x220)
void player_glide_to_pose(VECTOR *pos, SVECTOR *rot)
{
    VECTOR from;
    VECTOR to;
    SVECTOR a;
    SVECTOR b;
    int n;

    to = *pos;
    b = *rot;
    from = viewer.position;
    a = viewer.view_rot;
    if (a.vx > 2048)
        a.vx -= 4096;
    if (a.vy > 2048)
        a.vy -= 4096;
    if (b.vx > 2048)
        b.vx -= 4096;
    if (b.vy > 2048)
        b.vy -= 4096;
    n = 16;
    while (n--) {
        viewer.position.vx = (from.vx * n + to.vx * (16 - n)) >> 4;
        viewer.position.vz = (from.vz * n + to.vz * (16 - n)) >> 4;
        viewer.view_rot.vx = (a.vx * n + b.vx * (16 - n)) >> 4;
        viewer.view_rot.vy = (a.vy * n + b.vy * (16 - n)) >> 4;
        viewer.pitch = viewer.view_rot.vx + viewer.rot_offset.halves[0] + viewer.shake_pitch + viewer.fall_pitch;
        viewer.yaw = viewer.view_rot.vy + viewer.rot_offset.halves[1] + viewer.shake_yaw + viewer.unk292;
        world_run_frame(NULL, (SVECTOR *)&viewer.pitch, 0);
    }
}

/* Turns the view to face the entry over 16 frames. */
ADDRESS(0x801ed1e4, 0x10c)
void player_turn_to_creature(Creature *e)
{
    SVECTOR to;
    SVECTOR from;
    int dx;
    int dz;
    int angle;
    u16 yaw;
    int d;
    short turn;
    short step;
    int n = 16;

    dx = e->position.vx - viewer.position.vx;
    dz = e->position.vz - viewer.position.vz;
    angle = ratan2(dz, dx) - 1024;
    to.vy = angle;
    yaw = viewer.view_rot.vy;
    from.vy = yaw;
    d = (angle - yaw) & 0xfff;
    turn = d;
    if (d > 2048)
        turn = d | 0xf000;
    step = turn / n;
    while (n--) {
        viewer.view_rot.vy += step;
        viewer.yaw = viewer.view_rot.vy + viewer.rot_offset.halves[1] + viewer.shake_yaw + viewer.unk292;
        world_run_frame(NULL, (SVECTOR *)&viewer.pitch, 0);
    }
}

/* Waits for the entry's model file, then runs its fade-in to the end. */
ADDRESS(0x801ed2f0, 0x98)
void creature_finish_fade_in(Creature *e)
{
    StAnimFile *file;

    if (e->fade == -1)
        return;
    do
        anim_get_file(e->model_slot + 128, &file);
    while (file == NULL);
    while (1) {
        e->fade -= 128;
        if (e->fade < 0)
            break;
        world_run_frame(NULL, NULL, 0);
    }
    e->fade = -1;
    e->status.bytes[2] &= 0xf2;
}

/* Hands the entry to creature_talk, finishing its fade-in first when unk04 is set. */
ADDRESS(0x801ed388, 0x88)
void creature_force_talk(Creature *e, int fixed, int arg2)
{
    world_run_frame(NULL, NULL, 0);
    e->flags |= 0x2000000;
    if (e->draw_state) {
        creature_finish_fade_in(e);
        creature_talk(e, fixed, arg2);
    } else {
        creature_talk(e, 1, arg2);
    }
}

/* Puts the entry's record back into the live sums and resets it. */
ADDRESS(0x801ed410, 0x88)
void creature_spawn_now(Creature *e)
{
    CreatureType *r = &vis_table_a.types[e->type];

    vis_table_a.unk612C += r->unk12;
    vis_table_a.spawn_cost_total += e->spawn_cost;
    creature_spawn(e, r);
    e->unk1B = 255;
    e->range_bits = 0xffff;
    e->aggressive = 0xffff;
}

/* The default handler: does nothing. */
ADDRESS(0x801ed498, 0x8)
void map_hook_none(void)
{
}
