/*
 * creature_run_script_command and creature_choose_action, the 11th and 12th of the 21 functions
 * every FDAT overlay starts with (WIP; shared fragment). Each overlay's unit
 * includes it after its handler table, whose second part is creature_run_script_command's
 * switch table; the claims are FDAT_002.BIN's addresses and the other
 * overlays bind by name.
 */

/* A hit window of a script: frame `f` is passed during this step. */
#define FDAT_PASSES(e, f) \
    ((f) != 0xffff && (short)(e)->anim_frame - (e)->anim_step < (f) && (short)(e)->anim_frame >= (f))

/*
 * Runs the movement or attack of the entry's current script command.
 * Command 32's two push windows share their locals: `inner` is the first
 * window's end and the second window's start, `outer` the other two bounds.
 */
ADDRESS(0x801ebec4, 0x7e0)
void creature_run_script_command(Creature *e)
{
    int speed = abs(e->anim_step);
    CreatureScript *s = e->script;
    int turn;
    int move;
    int push;
    int frame;
    int inner;
    int outer;

    creature_update_script_sound(e);
    switch (e->command) {
    case 0:
    case 1:
    case 2:
        e->anim = s->anim;
        if (e->command == 0)
            creature_turn_toward(e->target_heading, s->arg.s[0] * speed >> 12);
        else if (e->command == 1 && ((short)e->anim_frame & 0x7ff) < 1024)
            creature_turn_toward(e->target_heading, s->arg.s[0] * speed >> 10);
        break;
    case 22:
        e->anim = s->anim;
        creature_turn_toward(e->target_heading, s->arg.s[1] * speed >> 12);
        creature_set_velocity(e->facing + s->arg.u[2] - 1024, s->arg.s[0] * speed >> 12);
        break;
    case 16:
    case 17:
    case 18:
        e->anim = s->anim;
        if ((((short)e->anim_frame - s->arg.s[3] + 4096) & 0x7ff) < 1024)
            creature_set_velocity(e->facing + s->arg.u[2] - 1024, s->arg.s[0] * speed >> 11);
        else
            creature_turn_toward(e->target_heading, s->arg.s[1] * speed >> 11);
        break;
    case 19:
    case 20:
    case 21:
        e->anim = s->anim;
        if ((((short)e->anim_frame - s->arg.s[4] + 4096) & 0xfff) < s->arg.s[5]) {
            turn = s->arg.s[1] * speed / s->arg.s[5];
            move = s->arg.s[0] * speed / s->arg.s[5];
            creature_turn_toward(e->target_heading, turn);
            creature_set_velocity(e->facing + s->arg.u[3] - 1024, move);
        } else {
            creature_turn_toward(e->target_heading, s->arg.s[2] * speed / (4096 - s->arg.s[5]));
        }
        break;
    case 32:
        e->anim = s->anim;
        push = 0;
        if (e->contact_hit == 1)
            break;
        if (s->arg.u[11] != 0xffff && (frame = (short)e->anim_frame) >= (outer = s->arg.u[11])
            && (inner = s->arg.u[12]) >= frame) {
            push = s->arg.s[10] * speed / (inner - outer);
            creature_set_velocity(e->facing - 1024, push);
        }
        if (s->arg.u[14] != 0xffff && (frame = (short)e->anim_frame) >= (inner = s->arg.u[14])
            && frame <= (outer = s->arg.u[15])) {
            push = s->arg.s[13] * speed / (outer - inner);
            creature_set_velocity(e->facing + 1024, push);
        }
        if (s->arg.w[1] == -1 && s->arg.u[4] == 0xffff) {
            if (e->contact_hit == 0 && e->touched_player == 1 && push != 0
                && (short)e->player_bearing <= s->arg.u[1] && (short)e->player_bearing >= -s->arg.u[1]) {
                creature_hit_player(e, s);
                e->attack_miss = 0;
                e->contact_hit = 1;
                e->unkBE = 0;
                vis_table_a.push_x += push * rcos((short)e->player_heading - 1024) >> 11;
                vis_table_a.push_z += push * rsin((short)e->player_heading - 1024) >> 11;
            }
        } else if (FDAT_PASSES(e, s->arg.u[2]) || FDAT_PASSES(e, s->arg.u[3])
                   || FDAT_PASSES(e, s->arg.u[4])) {
            if (e->player_dist_xz >= e->radius + s->arg.u[0] + 768
                || e->position.vy - e->height >= viewer.position.vy + 1600 || viewer.position.vy >= e->position.vy)
                e->attack_miss = 1;
            else if ((short)e->player_bearing > s->arg.u[1] || (short)e->player_bearing < -s->arg.u[1])
                e->attack_miss = 2;
            else {
                creature_hit_player(e, s);
                e->attack_miss = 0;
            }
        }
        break;
    case 48:
        e->anim = s->anim;
        if (FDAT_PASSES(e, s->arg.u[2]) || FDAT_PASSES(e, s->arg.u[3])
            || FDAT_PASSES(e, s->arg.u[4])) {
            if (e->player_dist_xz < e->radius + s->arg.u[0] + 512) {
                if ((short)e->player_bearing <= s->arg.u[1] && (short)e->player_bearing >= -s->arg.u[1])
                    creature_spawn_attack_hits(e, s);
                else
                    e->attack_miss = 2;
            } else {
                e->attack_miss = 1;
            }
        }
        break;
    case 64:
        e->anim = s->anim;
        e->chain_script = NULL;
        break;
    case 80:
        e->anim = s->anim;
        break;
    }
    if (e->move_mode == 4)
        creature_steer_height(viewer.position.vy, 16);
}

/*
 * Updates the current entry's alert state (unk1D) and picks its next action
 * (unk44) from the viewer's range and bearing (unk60 keeps the range bits).
 */
ADDRESS(0x801ec6a4, 0x528)
void creature_choose_action(void)
{
    Creature *e = vis_table_a.current;
    CreatureType *r = vis_table_a.current_type;
    u32 range;
    int near;
    int bearing;
    int alert;
    int n;
    u16 state;

    creature_clear_contact_hit(e);
    if (e->action == 128) {
        int reach = e->script->wake_range * e->script->wake_range;

        e->status.bytes[3] = 2;
        if (e->player_dist_sq < reach << 16 || e->hit != 0)
            e->action = 256;
        e->hit = 0;
        return;
    }
    e->hit = 0;
    if (e->action == 256)
        e->status.bytes[3] = 0;
    range = (r->alert_range * r->alert_range) << 16;
    if (e->status.bytes[1] == 0) {
        if (range < e->player_dist_sq) {
            if (e->command == 64)
                e->status.bytes[1] = 2;
            else
                e->status.bytes[1] = 0;
        } else {
            if (abs((short)e->player_bearing) < 1024)
                n = 4096 - (e->player_dist_sq << 12) / range;
            else
                n = (4096 - (e->player_dist_sq << 12) / range) >> 1;
            if (rand() >> 3 <= n)
                e->status.bytes[1] = 1;
        }
    } else {
        if (e->status.bytes[1] == 2 && range >= e->player_dist_sq)
            e->status.bytes[1] = 1;
        if (e->status.bytes[1] == 1 && range < e->player_dist_sq)
            e->status.bytes[1] = 0;
    }
    n = 0;
    if (e->aggressive == 0xffff || (e->command == 64 && rand() >> 14))
        n = 1;
    if (n) {
        if (r->aggression == 255) {
            e->aggressive = 1;
        } else if (r->aggression == 1) {
            if (e->command == 64)
                e->aggressive = 0;
        } else {
            n = (e->hp << 12) / r->max_hp;
            if (n < 3072)
                n = 3072;
            if (rand() >> 7 < r->aggression * n >> 12)
                e->aggressive = 1;
            else
                e->aggressive = 0;
        }
    }
    near = (r->near_range * r->near_range) << 16;
    bearing = abs((short)e->player_bearing);
    alert = e->status.bytes[1] != 0;
    state = (s32)e->player_dist_sq < near ? alert | 2 : alert;
    if (bearing < 512)
        state |= 4;
    if (e->attack_miss == 1)
        state &= 0xfffd;
    else if (e->attack_miss == 2)
        state &= 0xfffb;
    e->attack_miss = 0;
    if (!(state & 1)) {
        e->action = 1;
    } else if (state & 2) {
        if (state & 4) {
            if (r->actions & 0x10) {
                if (e->aggressive) {
                    if (rand() < 24576)
                        e->action = 16;
                    else
                        e->action = 1;
                } else if (!(r->actions & 8)) {
                    e->action = 16;
                } else if (rand() < 4096) {
                    e->action = 16;
                } else {
                    e->action = 8;
                }
            } else if (r->actions & 8) {
                e->action = 8;
            } else {
                e->action = 1;
            }
        } else if (r->actions & 2) {
            if (!(r->actions & 8)) {
                e->action = 2;
            } else if (e->aggressive) {
                if (rand() < 4096)
                    e->action = 8;
                else
                    e->action = 2;
            } else if (rand() < 4096) {
                e->action = 2;
            } else {
                e->action = 8;
            }
        } else if (r->actions & 8) {
            e->action = 8;
        } else {
            e->action = 1;
        }
    } else if (state & 4) {
        if (e->aggressive) {
            if (r->actions & 0x20) {
                if (!(r->actions & 4))
                    e->action = 32;
                else if (rand() <= 16384)
                    e->action = 4;
                else
                    e->action = 32;
            } else if (r->actions & 4) {
                e->action = 4;
            } else {
                e->action = 1;
            }
        } else if (r->actions & 8) {
            e->action = 8;
        } else {
            e->action = 1;
        }
    } else if (r->actions & 2) {
        if (!(r->actions & 8))
            e->action = 2;
        else if (e->aggressive)
            e->action = 2;
        else if (rand() < 16384)
            e->action = 2;
        else
            e->action = 8;
    } else if (r->actions & 8) {
        e->action = 8;
    } else {
        e->action = 1;
    }
    e->range_bits = state;
}
