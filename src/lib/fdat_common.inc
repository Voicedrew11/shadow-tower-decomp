/*
 * creature_run_script_command and creature_choose_action, the 11th and 12th of the 21 functions
 * every FDAT overlay starts with (WIP; shared fragment). Each overlay's unit
 * includes it after its handler table, whose second part is creature_run_script_command's
 * switch table; the claims are FDAT_002.BIN's addresses and the other
 * overlays bind by name.
 */

/* A hit window of a script: frame `f` is passed during this step. */
#define FDAT_PASSES(e, f) \
    ((f) != 0xffff && (short)(e)->unk2A - (e)->unkB0 < (f) && (short)(e)->unk2A >= (f))

/*
 * Runs the movement or attack of the entry's current script command.
 * Command 32's two push windows share their locals: `inner` is the first
 * window's end and the second window's start, `outer` the other two bounds.
 */
ADDRESS(0x801ebec4, 0x7e0)
void creature_run_script_command(Vis212 *e)
{
    int speed = abs(e->unkB0);
    VisScript *s = e->unkA8;
    int turn;
    int move;
    int push;
    int frame;
    int inner;
    int outer;

    creature_update_script_sound(e);
    switch (e->unk0F) {
    case 0:
    case 1:
    case 2:
        e->unk08 = s->unk01;
        if (e->unk0F == 0)
            creature_turn_toward(e->unk5C, s->arg.s[0] * speed >> 12);
        else if (e->unk0F == 1 && ((short)e->unk2A & 0x7ff) < 1024)
            creature_turn_toward(e->unk5C, s->arg.s[0] * speed >> 10);
        break;
    case 22:
        e->unk08 = s->unk01;
        creature_turn_toward(e->unk5C, s->arg.s[1] * speed >> 12);
        creature_set_velocity(e->unk8E + s->arg.u[2] - 1024, s->arg.s[0] * speed >> 12);
        break;
    case 16:
    case 17:
    case 18:
        e->unk08 = s->unk01;
        if ((((short)e->unk2A - s->arg.s[3] + 4096) & 0x7ff) < 1024)
            creature_set_velocity(e->unk8E + s->arg.u[2] - 1024, s->arg.s[0] * speed >> 11);
        else
            creature_turn_toward(e->unk5C, s->arg.s[1] * speed >> 11);
        break;
    case 19:
    case 20:
    case 21:
        e->unk08 = s->unk01;
        if ((((short)e->unk2A - s->arg.s[4] + 4096) & 0xfff) < s->arg.s[5]) {
            turn = s->arg.s[1] * speed / s->arg.s[5];
            move = s->arg.s[0] * speed / s->arg.s[5];
            creature_turn_toward(e->unk5C, turn);
            creature_set_velocity(e->unk8E + s->arg.u[3] - 1024, move);
        } else {
            creature_turn_toward(e->unk5C, s->arg.s[2] * speed / (4096 - s->arg.s[5]));
        }
        break;
    case 32:
        e->unk08 = s->unk01;
        push = 0;
        if (e->unkBC == 1)
            break;
        if (s->arg.u[11] != 0xffff && (frame = (short)e->unk2A) >= (outer = s->arg.u[11])
            && (inner = s->arg.u[12]) >= frame) {
            push = s->arg.s[10] * speed / (inner - outer);
            creature_set_velocity(e->unk8E - 1024, push);
        }
        if (s->arg.u[14] != 0xffff && (frame = (short)e->unk2A) >= (inner = s->arg.u[14])
            && frame <= (outer = s->arg.u[15])) {
            push = s->arg.s[13] * speed / (outer - inner);
            creature_set_velocity(e->unk8E + 1024, push);
        }
        if (s->arg.w[1] == -1 && s->arg.u[4] == 0xffff) {
            if (e->unkBC == 0 && e->unk39 == 1 && push != 0
                && (short)e->unk58 <= s->arg.u[1] && (short)e->unk58 >= -s->arg.u[1]) {
                creature_hit_player(e, s);
                e->unk62 = 0;
                e->unkBC = 1;
                e->unkBE = 0;
                vis_table_a.unk6160 += push * rcos((short)e->unk56 - 1024) >> 11;
                vis_table_a.unk6164 += push * rsin((short)e->unk56 - 1024) >> 11;
            }
        } else if (FDAT_PASSES(e, s->arg.u[2]) || FDAT_PASSES(e, s->arg.u[3])
                   || FDAT_PASSES(e, s->arg.u[4])) {
            if (e->unk40 >= e->unk2E + s->arg.u[0] + 768
                || e->unk74.vy - e->unk30 >= viewer.position.vy + 1600 || viewer.position.vy >= e->unk74.vy)
                e->unk62 = 1;
            else if ((short)e->unk58 > s->arg.u[1] || (short)e->unk58 < -s->arg.u[1])
                e->unk62 = 2;
            else {
                creature_hit_player(e, s);
                e->unk62 = 0;
            }
        }
        break;
    case 48:
        e->unk08 = s->unk01;
        if (FDAT_PASSES(e, s->arg.u[2]) || FDAT_PASSES(e, s->arg.u[3])
            || FDAT_PASSES(e, s->arg.u[4])) {
            if (e->unk40 < e->unk2E + s->arg.u[0] + 512) {
                if ((short)e->unk58 <= s->arg.u[1] && (short)e->unk58 >= -s->arg.u[1])
                    creature_spawn_attack_hits(e, s);
                else
                    e->unk62 = 2;
            } else {
                e->unk62 = 1;
            }
        }
        break;
    case 64:
        e->unk08 = s->unk01;
        e->unkAC = NULL;
        break;
    case 80:
        e->unk08 = s->unk01;
        break;
    }
    if (e->unk0E == 4)
        creature_steer_height(viewer.position.vy, 16);
}

/*
 * Updates the current entry's alert state (unk1D) and picks its next action
 * (unk44) from the viewer's range and bearing (unk60 keeps the range bits).
 */
ADDRESS(0x801ec6a4, 0x528)
void creature_choose_action(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    u32 range;
    int near;
    int bearing;
    int alert;
    int n;
    u16 state;

    creature_clear_contact_hit(e);
    if (e->unk44 == 128) {
        int reach = e->unkA8->unk03 * e->unkA8->unk03;

        e->unk1C.bytes[3] = 2;
        if (e->unk3C < reach << 16 || e->unk64 != 0)
            e->unk44 = 256;
        e->unk64 = 0;
        return;
    }
    e->unk64 = 0;
    if (e->unk44 == 256)
        e->unk1C.bytes[3] = 0;
    range = (r->unk01 * r->unk01) << 16;
    if (e->unk1C.bytes[1] == 0) {
        if (range < e->unk3C) {
            if (e->unk0F == 64)
                e->unk1C.bytes[1] = 2;
            else
                e->unk1C.bytes[1] = 0;
        } else {
            if (abs((short)e->unk58) < 1024)
                n = 4096 - (e->unk3C << 12) / range;
            else
                n = (4096 - (e->unk3C << 12) / range) >> 1;
            if (rand() >> 3 <= n)
                e->unk1C.bytes[1] = 1;
        }
    } else {
        if (e->unk1C.bytes[1] == 2 && range >= e->unk3C)
            e->unk1C.bytes[1] = 1;
        if (e->unk1C.bytes[1] == 1 && range < e->unk3C)
            e->unk1C.bytes[1] = 0;
    }
    n = 0;
    if (e->unk54 == 0xffff || (e->unk0F == 64 && rand() >> 14))
        n = 1;
    if (n) {
        if (r->unk02 == 255) {
            e->unk54 = 1;
        } else if (r->unk02 == 1) {
            if (e->unk0F == 64)
                e->unk54 = 0;
        } else {
            n = (e->unk2C << 12) / r->unk32;
            if (n < 3072)
                n = 3072;
            if (rand() >> 7 < r->unk02 * n >> 12)
                e->unk54 = 1;
            else
                e->unk54 = 0;
        }
    }
    near = (r->unk14 * r->unk14) << 16;
    bearing = abs((short)e->unk58);
    alert = e->unk1C.bytes[1] != 0;
    state = (s32)e->unk3C < near ? alert | 2 : alert;
    if (bearing < 512)
        state |= 4;
    if (e->unk62 == 1)
        state &= 0xfffd;
    else if (e->unk62 == 2)
        state &= 0xfffb;
    e->unk62 = 0;
    if (!(state & 1)) {
        e->unk44 = 1;
    } else if (state & 2) {
        if (state & 4) {
            if (r->unk1C & 0x10) {
                if (e->unk54) {
                    if (rand() < 24576)
                        e->unk44 = 16;
                    else
                        e->unk44 = 1;
                } else if (!(r->unk1C & 8)) {
                    e->unk44 = 16;
                } else if (rand() < 4096) {
                    e->unk44 = 16;
                } else {
                    e->unk44 = 8;
                }
            } else if (r->unk1C & 8) {
                e->unk44 = 8;
            } else {
                e->unk44 = 1;
            }
        } else if (r->unk1C & 2) {
            if (!(r->unk1C & 8)) {
                e->unk44 = 2;
            } else if (e->unk54) {
                if (rand() < 4096)
                    e->unk44 = 8;
                else
                    e->unk44 = 2;
            } else if (rand() < 4096) {
                e->unk44 = 2;
            } else {
                e->unk44 = 8;
            }
        } else if (r->unk1C & 8) {
            e->unk44 = 8;
        } else {
            e->unk44 = 1;
        }
    } else if (state & 4) {
        if (e->unk54) {
            if (r->unk1C & 0x20) {
                if (!(r->unk1C & 4))
                    e->unk44 = 32;
                else if (rand() <= 16384)
                    e->unk44 = 4;
                else
                    e->unk44 = 32;
            } else if (r->unk1C & 4) {
                e->unk44 = 4;
            } else {
                e->unk44 = 1;
            }
        } else if (r->unk1C & 8) {
            e->unk44 = 8;
        } else {
            e->unk44 = 1;
        }
    } else if (r->unk1C & 2) {
        if (!(r->unk1C & 8))
            e->unk44 = 2;
        else if (e->unk54)
            e->unk44 = 2;
        else if (rand() < 16384)
            e->unk44 = 2;
        else
            e->unk44 = 8;
    } else if (r->unk1C & 8) {
        e->unk44 = 8;
    } else {
        e->unk44 = 1;
    }
    e->unk60 = state;
}
