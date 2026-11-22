/*
 * ST.EXE: item counts and the examine action (WIP module,
 * 0x8005529c..0x80055990): count tests, taking and adding one, and what
 * the player's examine/use reaches at a position: a table C item, a table
 * A entry to talk to, or the table B objects in reach (doors, images,
 * transitions).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/funcs.h>
#include <st/player.h>
#include <st/pad.h>
#include <st/xa.h>
#include <st/save.h>
#include <st/item.h>

ADDRESS(0x8005529c, 0x20)
int item_is_held(int id)
{
    return inventory.counts[id].count != 0;
}

/* Takes one of item `id`; nonzero when there was none. */
ADDRESS(0x800552bc, 0x44)
int item_take(int id)
{
    if (inventory.counts[id].count != 0) {
        inventory.counts[id].count--;
        return 0;
    }
    return 1;
}

/* Adds one of item `id`; nonzero when 99 are already held. */
ADDRESS(0x80055300, 0x74)
int item_add(int id)
{
    if (inventory.counts[id].count < 99) {
        inventory.counts[id].count++;
        area_state.hooks->item_added(id);
        return 0;
    }
    return 1;
}

/* The examine action at `pos` facing `rot` (vx: yaw, vy: pitch). */
ADDRESS(0x80055374, 0x61c)
void player_examine(long *pos, SVECTOR *rot)
{
    VECTOR p;
    CollisionRequest req;
    int i;
    int first_result;
    int up;
    int down;
    int kind;
    u16 r;
    int n;
    int d;
    GroundItem *c;
    Creature *a;
    AreaObject *b;
    AreaObjectType *def;

    p.vx = pos[0];
    p.vy = pos[1] + 1600;
    p.vz = pos[2];
    game_progress.item_use_result = 0;
    req.tests = 0;
    req.position = p;
    req.radius = 1056;
    req.velocity.vx = 0;
    req.velocity.vy = 0;
    req.velocity.vz = 0;
    req.speed = 0;
    req.height = 1600;
    req.heading = 0;
    i = ground_item_find_in_reach((CollisionState *)&req, 1);
    first_result = i;
    if (i != -1) {
        c = &vis_table_c.entries[i];
        c->prev_state = c->state;
        c->state = 3;
        return;
    }
    /* CollisionState starts with the request fields creature_find_in_reach reads. */
    i = creature_find_in_reach((CollisionState *)&req, 1);
    if (i != first_result) {
        a = &vis_table_a.entries[i];
        up = ratan2(a->position.vy - a->height - viewer.position.vy, a->player_dist_xz);
        down = ratan2(a->position.vy - (a->height >> 1) - viewer.position.vy, a->player_dist_xz);
        if (up > 2048)
            up -= 2048;
        if (down > 2048)
            up -= 2048;
        if (up > 96)
            up = 96;
        if (down < 160)
            down = 160;
        if (angle_is_after((short)down, rot->vx) && angle_is_after(rot->vx, (short)up)) {
            if (angle_within(rot->vy, a->rotation.vy, 512))
                creature_talk(a, 0, 0);
            return;
        }
    }
    i = 0;
    req.radius = 256;
    vis_lists.position = p;
    vis_lists.radius = 256;
    vis_lists.height = 1600;
    vis_lists.velocity.vx = 0;
    vis_lists.velocity.vy = 0;
    vis_lists.velocity.vz = 0;
    vis_lists.speed = 0;
    vis_lists.heading = 0;
    vis_lists.tests = 0;
    for (; (i = object_find_usable(i, &vis_lists)) != -1; i++) {
        b = &vis_table_b.entries[i];
        def = &vis_table_b.types[b->type];
        game_progress.item_use_result = 1;
        b->examined = 1;
        kind = def->kind;
        switch (kind) {
        case 254:
            if (b->state == 0)
                b->state = 1;
            break;
        /* An empty case above 226: retail's compare tree splits its four
         * upper cases at 226, which takes a fifth node on that side. */
        case 255:
            break;
        case 0:
        case 1:
        case 50:
        case 51:
            if (b->params.door.lock == 251) {
                r = (short)b->player_bearing > 2048 ? b->player_bearing - 4096 : b->player_bearing;
                /* The offset window wraps at the halfword boundary. */
                if ((u16)(r + 1023) < 2047) {
                    play_unlock_sound();
                    b->params.door.lock = 255;
                }
            } else if (b->params.door.lock == 252) {
                r = (short)b->player_bearing > 2048 ? b->player_bearing - 4096 : b->player_bearing;
                if ((u16)(r + 1024) >= 2049) {
                    play_unlock_sound();
                    b->params.door.lock = 255;
                }
            }
            if (b->params.door.lock == 255) {
                if (b->state == 0)
                    b->state = 1;
                if (b->state == 7)
                    b->state = 8;
            } else if (b->params.door.lock < 0xf0 || b->params.door.lock > 0xf3) {
                play_locked_sound();
            }
            break;
        case 52:
            if (b->params.door.lock == 255) {
                if (b->state == 0) {
                    b->state = 1;
                    b->anim = 0;
                }
                if (b->state == 255 && b->unk49 != 252) {
                    b->state = 3;
                    b->anim = 1;
                }
            }
            break;
        case 225:
            d = (u16)(b->rotation.vy - b->player_heading) & 0xfff;
            if (d < 512 || d > 3584)
                talk_show_message(b->unk48);
            break;
        case 226:
            viewer.hp = viewer.max_hp;
            sound_play(8, 128);
            break;
        case 14:
            if (area_state.loading != 1 && area_state.entry_fade_start == -1) {
                lighting.paused = 1;
                cd_wait_idle();
                area_save_snapshot(area_state.loaded_area);
                play_confirm_sound();
                anim_free_all_for_menu();
                pad_clear_input();
                pad_vibrate(0);
                b = vis_table_b.entries;
                n = 240;
                while (n--) {
                    if (b->kind == 236 && b->unk54 != -1) {
                        sound_stop(b->unk54, b->params.snd.sound + 1108);
                        b->unk54 = -1;
                    }
                    b++;
                }
                card_screen_open(1, 1);
                card_screen_run(1);
                lighting.paused = 2;
                pad_clear_input();
                card_screen_close();
            }
            break;
        }
    }
    area_state.hooks->examine(&p, rot);
}
