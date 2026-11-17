/*
 * The 21 functions every FDAT overlay starts with (WIP; shared fragment).
 * Each overlay's unit includes this file after its handler table; the
 * claims are FDAT_002.BIN's addresses and the other overlays bind by name.
 */

/* Clears the entry's unkBC. */
ADDRESS(0x801eb7ac, 0x8)
void fdat_801eb7ac(Vis212 *e)
{
    e->unkBC = 0;
}

/* Starts script `id` of the entry's record. */
ADDRESS(0x801eb7b4, 0x60)
void fdat_801eb7b4(Vis212 *e, int id)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];

    e->unk18 = id;
    e->unkA8 = r->unk60[id];
    e->unk0F = e->unkA8->unk00;
    e->unk08 = e->unkA8->unk01;
    e->unkB0 = e->unkA8->unk0C;
    e->unk1A = 0;
    e->unk2A = 0;
}

/* Turns the current entry towards `angle` by at most `step`. */
ADDRESS(0x801eb814, 0x6c)
void fdat_801eb814(short angle, int step)
{
    Vis212 *e = vis_table_a.unk6138;
    int d = (angle - e->unk8E) & 0xfff;
    int a;

    if (d > 2048)
        d -= 4096;
    a = abs(d);
    if (a < step)
        step = a;
    if (d != 0) {
        if (d > 0)
            e->unk8E += step;
        else
            e->unk8E -= step;
    }
}

/* Sets the current entry's horizontal velocity: `speed` along `angle`. */
ADDRESS(0x801eb880, 0x74)
void fdat_801eb880(short angle, int speed)
{
    Vis212 *e = vis_table_a.unk6138;

    e->unk9C.vx = speed * rcos(angle) >> 12;
    e->unk9C.vz = speed * rsin(angle) >> 12;
}

/* Steers the current entry's vertical speed towards height `y`. */
ADDRESS(0x801eb8f4, 0x4c)
void fdat_801eb8f4(int y, int step)
{
    Vis212 *e = vis_table_a.unk6138;

    y -= e->unk74.vy;
    if (y > 0)
        e->unk16 += step;
    else if (y < 0)
        e->unk16 -= step;
}

/* A random note offset, -2..2. */
ADDRESS(0x801eb940, 0x2c)
int fdat_801eb940(Vis212 *e)
{
    return (rand() * 5 >> 15) - 2;
}

/* Plays the entry's script sound. */
ADDRESS(0x801eb96c, 0xe0)
void fdat_801eb96c(Vis212 *e)
{
    int sound = e->unkA8->unk05;

    e->unk3A = e->unk0B * 10 + 48 + sound;
    if (e->unk70 & 0x800000)
        e->unk22 = func_80058254(e->unk0B * 10 + 48 + sound, &e->unk74, 127, 0x6000, 0x7800,
                                 fdat_801eb940(e));
    else
        e->unk22 = func_80058220(e->unk0B * 10 + 48 + sound, &e->unk74, 127, fdat_801eb940(e));
}

/*
 * Plays the script sound when its trigger fires this frame.
 * Not exact: retail keeps the 0x1000/0x4000 frame in a0 and copies it to s0
 * only for the 0x8000 (random) case; this build keeps it in s0 throughout,
 * and loads the trigger before the sound byte.
 */
ADDRESS(0x801eba4c, 0x17c)
void fdat_801eba4c(Vis212 *e)
{
    VisScript *s = e->unkA8;
    u16 frame = e->unk2A;
    u8 sound = s->unk05;
    u16 trigger = s->unk0E;
    u16 step = s->unk0C;
    int at;
    int chance;

    if (sound == 255)
        return;
    at = trigger & 0xfff;
    switch (trigger & 0xf000) {
    case 0x1000:
        if (at >= (short)frame - (short)step && at < (short)frame)
            fdat_801eb96c(e);
        break;
    case 0x2000:
        at = trigger & 0x7ff;
        if (at >= (frame & 0x7ff) - (short)step && at < (frame & 0x7ff))
            fdat_801eb96c(e);
        break;
    case 0x8000:
        chance = at;
        if (chance >= (short)frame - (short)step && chance < (short)frame
            && rand() >> 3 < chance)
            fdat_801eb96c(e);
        break;
    case 0x4000:
        if (cd_state.unk8 / 20 % at == 0)
            fdat_801eb96c(e);
        break;
    }
}

/* Hits the player with the script's damage. */
ADDRESS(0x801ebbc8, 0xc0)
void fdat_801ebbc8(Vis212 *e, VisScript *s)
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
    player_hit(damage, 4096, effect, (long *)&e->unk74);
}

/* Spawns the hits of attack `a` whose frames the animation passes. */
ADDRESS(0x801ebc88, 0x23c)
void fdat_801ebc88(Vis212 *e, FdatAttack *a)
{
    VECTOR pos;
    SVECTOR v;
    VECTOR node;
    SVECTOR off;
    MATRIX m;
    SVECTOR rot;
    StAnimFile *file;
    int id = e->unk0B + 128;
    int i;
    int plain;

    file = world_state.unk834C[id];
    if (file == NULL)
        return;
    plain = file->unk6 == 0;
    for (i = 2; i != -1; i--) {
        if (a->unk14[i] == 0xffff)
            continue;
        if (!((short)e->unk2A - e->unkB0 < a->unk14[i]))
            continue;
        if ((short)e->unk2A < a->unk14[i])
            continue;
        pos = e->unk74;
        rot.vx = 0;
        rot.vy = -e->unk8E;
        rot.vz = 0;
        RotMatrix(&rot, &m);
        if (a->unk1C[i] != 0xffff) {
            if (plain) {
                anim_get_vertex(id, e->unk08, (short)e->unk2A, a->unk1C[i], &v);
            } else {
                anim_set_node_vertex((StAnimSet *)e->unkA4, id, a->unk22[i], a->unk1C[i], &node);
                v.vx = node.vx;
                v.vy = node.vy;
                v.vz = node.vz;
            }
            ApplyMatrixSV(&m, &v, &v);
            pos.vx += v.vx;
            pos.vy += v.vy;
            pos.vz += v.vz;
        }
        off.vx = a->unk28[0];
        off.vy = a->unk28[1];
        off.vz = a->unk28[2];
        ApplyMatrixSV(&m, &off, &off);
        pos.vx += off.vx;
        pos.vy += off.vy;
        pos.vz += off.vz;
        func_8004cee8(e, a->unk1A, &pos);
        e->unk62 = 0;
    }
}

/* Runs the entry's script command (not reconstructed yet). */
ADDRESS(0x801ebec4, 0x7e0)
void fdat_801ebec4(Vis212 *e)
{
}

ADDRESS(0x801ec6a4, 0x528)
void fdat_801ec6a4(void)
{
}

/*
 * Picks one of the current record's scripts allowed in the entry's state
 * by weight and starts it; 0 when none is allowed.
 * Not exact: retail compares the list index with 24 held in t0 (`slt`,
 * loaded first in the prologue); this build compares with `slti`.
 */
ADDRESS(0x801ecbcc, 0x2b0)
int fdat_801ecbcc(void)
{
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

    for (i = 0; i < 24; i++) {
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
int fdat_801ece7c(Vis212 *e)
{
    if (e->unk34 < 0)
        return 0;
    e->unk34 -= abs(e->unkB0);
    if (e->unk34 < 0) {
        if (e->unk2C == 0) {
            if ((short)e->unk2A >= 4095)
                func_8004ae38(e);
            return 1;
        }
        e->unk34 = -1;
        e->unk1C.bytes[2] &= 0xdf;
    } else if ((short)e->unk2A >= 4096 && e->unk05 == 5) {
        e->unk2A = 4095;
    }
    return 0;
}

/* Fades the entry in over 32 frames, then removes it. */
ADDRESS(0x801ecf44, 0x80)
void fdat_801ecf44(Vis212 *e)
{
    e->unk36 = 0;
    e->unk1C.bytes[2] |= 0xd;
    while (1) {
        e->unk36 += 128;
        if (e->unk36 > 4096)
            break;
        func_80026dd4(NULL, (SVECTOR *)&viewer.unk268, 0);
    }
    func_8004ae38(e);
}

/*
 * Moves the view to `pos`, turned to `rot`, over 16 frames.
 * Not exact: retail copies the viewer position as one VECTOR member of the
 * viewer block (offsets 0x248.. from &viewer); st/data.h spells it as the
 * longs x, y, z, so this build copies through a cast (&viewer.x + 0..).
 */
ADDRESS(0x801ecfc4, 0x220)
void fdat_801ecfc4(VECTOR *pos, SVECTOR *rot)
{
    VECTOR from;
    VECTOR to;
    SVECTOR a;
    SVECTOR b;
    int n;

    to = *pos;
    b = *rot;
    from = *(VECTOR *)&viewer.x;
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
        viewer.x = (from.vx * n + to.vx * (16 - n)) >> 4;
        viewer.z = (from.vz * n + to.vz * (16 - n)) >> 4;
        viewer.unk278.vx = (a.vx * n + b.vx * (16 - n)) >> 4;
        viewer.unk278.vy = (a.vy * n + b.vy * (16 - n)) >> 4;
        viewer.unk268 = viewer.unk278.vx + viewer.unk280 + viewer.unk288 + viewer.unk290;
        viewer.unk26A = viewer.unk278.vy + viewer.unk282 + viewer.unk28A + viewer.unk292;
        func_80026dd4(NULL, (SVECTOR *)&viewer.unk268, 0);
    }
}

/* Turns the view to face the entry over 16 frames. */
ADDRESS(0x801ed1e4, 0x10c)
void fdat_801ed1e4(Vis212 *e)
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

    dx = e->unk74.vx - viewer.x;
    dz = e->unk74.vz - viewer.z;
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
        viewer.unk26A = viewer.unk278.vy + viewer.unk282 + viewer.unk28A + viewer.unk292;
        func_80026dd4(NULL, (SVECTOR *)&viewer.unk268, 0);
    }
}

/* Waits for the entry's model file, then fades it out. */
ADDRESS(0x801ed2f0, 0x98)
void fdat_801ed2f0(Vis212 *e)
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
        func_80026dd4(NULL, NULL, 0);
    }
    e->unk36 = -1;
    e->unk1C.bytes[2] &= 0xf2;
}

/* Hands the entry to func_80054368, fading it out first when unk04 is set. */
ADDRESS(0x801ed388, 0x88)
void fdat_801ed388(Vis212 *e, int fixed, int arg2)
{
    func_80026dd4(NULL, NULL, 0);
    e->unk70 |= 0x2000000;
    if (e->unk04) {
        fdat_801ed2f0(e);
        func_80054368(e, fixed, arg2);
    } else {
        func_80054368(e, 1, arg2);
    }
}

/* Puts the entry's record back into the live sums and resets it. */
ADDRESS(0x801ed410, 0x88)
void fdat_801ed410(Vis212 *e)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];

    vis_table_a.unk612C += r->unk12;
    vis_table_a.unk6128 += e->unk0D;
    func_8004af24(e, r);
    e->unk1B = 255;
    e->unk60 = 0xffff;
    e->unk54 = 0xffff;
}

/* The default handler: does nothing. */
ADDRESS(0x801ed498, 0x8)
void fdat_801ed498(void)
{
}
