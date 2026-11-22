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
    VisScript *list[24];
    int index[24];
    Vis192 *r = vis_table_a.unk6134;
    Vis212 *e = vis_table_a.unk6138;
    VisScript **p = r->unk60;
    VisScript *s;
    VisScript *prev;
    int count = 0;
    int total = 0;
    int pick;
    int i;

    for (i = 0; i < limit; i++) {
        s = *p++;
        if (s == NULL)
            break;
        if (s->unk08 & e->unk44) {
            index[count] = i;
            list[count] = s;
            count++;
            total += s->unk04;
        }
    }
    if (count == 0)
        return 0;
    pick = rand() * total >> 15;
    count--;
    while (count != -1) {
        pick -= list[count]->unk04;
        if (pick <= 0)
            break;
        count--;
    }
    if (count < 0)
        count = 0;
    e->unk18 = index[count];
    prev = e->unkAC;
    e->unkA8 = list[count];
    e->unk0F = e->unkA8->unk00;
    e->unkB0 = e->unkA8->unk0C;
    if (prev != NULL && e->unkA8 != prev) {
        if (prev->unk00 == 17 && prev->arg.b[9] != 255)
            e->unk18 = prev->arg.b[9];
        else if (e->unkAC->unk00 == 20 && e->unkAC->arg.b[13] != 255)
            e->unk18 = e->unkAC->arg.b[13];
        e->unkA8 = r->unk60[e->unk18];
        e->unkB0 = e->unkA8->unk0C;
        e->unk0F = e->unkA8->unk00;
        e->unkAC = NULL;
    }
    if ((e->unk0F == 17 || e->unk0F == 20) && e->unkAC == NULL) {
        e->unkAC = e->unkA8;
        if (e->unk0F == 17 && e->unkA8->arg.b[8] != 255)
            e->unk18 = e->unkA8->arg.b[8];
        else if (e->unk0F == 20 && e->unkA8->arg.b[12] != 255)
            e->unk18 = e->unkA8->arg.b[12];
        e->unkA8 = r->unk60[e->unk18];
        e->unkB0 = e->unkA8->unk0C;
        e->unk0F = e->unkA8->unk00;
    }
    e->unk1A = 0;
    if (e->unkB0 >= 0)
        e->unk2A = 0;
    else
        e->unk2A = 4096;
    return 1;
}

/* Counts the entry's unk34 down by its step; 1 once it runs out. */
ADDRESS(0x801ece7c, 0xc8)
int creature_step_hurt_timer(Vis212 *e)
{
    if (e->unk34 < 0)
        return 0;
    e->unk34 -= abs(e->unkB0);
    if (e->unk34 < 0) {
        if (e->unk2C == 0) {
            if ((short)e->unk2A >= 4095)
                creature_despawn(e);
            return 1;
        }
        e->unk34 = -1;
        e->unk1C.bytes[2] &= 0xdf;
    } else if ((short)e->unk2A >= 4096 && e->unk05 == 5) {
        e->unk2A = 4095;
    }
    return 0;
}

/* Fades the entry out over 32 frames, then removes it. */
ADDRESS(0x801ecf44, 0x80)
void creature_fade_out_despawn(Vis212 *e)
{
    e->unk36 = 0;
    e->unk1C.bytes[2] |= 0xd;
    while (1) {
        e->unk36 += 128;
        if (e->unk36 > 4096)
            break;
        world_run_frame(NULL, (SVECTOR *)&viewer.unk268, 0);
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
    a = viewer.unk278;
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
        viewer.unk278.vx = (a.vx * n + b.vx * (16 - n)) >> 4;
        viewer.unk278.vy = (a.vy * n + b.vy * (16 - n)) >> 4;
        viewer.unk268 = viewer.unk278.vx + viewer.unk280.halves[0] + viewer.unk288 + viewer.unk290;
        viewer.unk26A = viewer.unk278.vy + viewer.unk280.halves[1] + viewer.unk28A + viewer.unk292;
        world_run_frame(NULL, (SVECTOR *)&viewer.unk268, 0);
    }
}

/* Turns the view to face the entry over 16 frames. */
ADDRESS(0x801ed1e4, 0x10c)
void player_turn_to_creature(Vis212 *e)
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

    dx = e->unk74.vx - viewer.position.vx;
    dz = e->unk74.vz - viewer.position.vz;
    angle = ratan2(dz, dx) - 1024;
    to.vy = angle;
    yaw = viewer.unk278.vy;
    from.vy = yaw;
    d = (angle - yaw) & 0xfff;
    turn = d;
    if (d > 2048)
        turn = d | 0xf000;
    step = turn / n;
    while (n--) {
        viewer.unk278.vy += step;
        viewer.unk26A = viewer.unk278.vy + viewer.unk280.halves[1] + viewer.unk28A + viewer.unk292;
        world_run_frame(NULL, (SVECTOR *)&viewer.unk268, 0);
    }
}

/* Waits for the entry's model file, then runs its fade-in to the end. */
ADDRESS(0x801ed2f0, 0x98)
void creature_finish_fade_in(Vis212 *e)
{
    StAnimFile *file;

    if (e->unk36 == -1)
        return;
    do
        anim_get_file(e->unk0B + 128, &file);
    while (file == NULL);
    while (1) {
        e->unk36 -= 128;
        if (e->unk36 < 0)
            break;
        world_run_frame(NULL, NULL, 0);
    }
    e->unk36 = -1;
    e->unk1C.bytes[2] &= 0xf2;
}

/* Hands the entry to creature_talk, finishing its fade-in first when unk04 is set. */
ADDRESS(0x801ed388, 0x88)
void creature_force_talk(Vis212 *e, int fixed, int arg2)
{
    world_run_frame(NULL, NULL, 0);
    e->unk70 |= 0x2000000;
    if (e->unk04) {
        creature_finish_fade_in(e);
        creature_talk(e, fixed, arg2);
    } else {
        creature_talk(e, 1, arg2);
    }
}

/* Puts the entry's record back into the live sums and resets it. */
ADDRESS(0x801ed410, 0x88)
void creature_spawn_now(Vis212 *e)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];

    vis_table_a.unk612C += r->unk12;
    vis_table_a.unk6128 += e->unk0D;
    creature_spawn(e, r);
    e->unk1B = 255;
    e->unk60 = 0xffff;
    e->unk54 = 0xffff;
}

/* The default handler: does nothing. */
ADDRESS(0x801ed498, 0x8)
void map_hook_none(void)
{
}
