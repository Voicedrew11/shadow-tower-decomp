/*
 * ST.EXE: table A entry drops and checks (WIP module, 0x8004d75c..0x8004def8;
 * its caller creature_take_damage opens the next band and may share its file).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/funcs.h>
#include <st/data.h>

ADDRESS(0x8004d75c, 0x8)
int creature_sound_note(Creature *e)
{
    return 0;
}

/* Plays the entry's sound from its current list entry at its position. */
ADDRESS(0x8004d764, 0x98)
void creature_play_script_sound(Creature *e)
{
    u8 *p = (u8 *)e->script;

    if (e->flags & 0x800000)
        sound_play_at_range(p[5] + 48, &e->position, 127, 24576, 30720, creature_sound_note(e));
    else
        sound_play_at(p[5] + 48, &e->position, 127, creature_sound_note(e));
}

ADDRESS(0x8004d7fc, 0x8)
void func_8004d7fc(void)
{
}

/* Drops item `id` where the entry stands: a slot of table C's group 48
 * (kind 30) or group 32, the oldest slot giving way when the group is full. */
ADDRESS(0x8004d804, 0x228)
void creature_drop_item(Creature *e, int id)
{
    ItemType *rec = &vis_table_c.types[id];
    GroundItem *slot;

    if (rec->category == 30) {
        if (vis_table_c.ground_item_counts[3] == 16) {
            int n;

            slot = &vis_table_c.entries[48];
            n = 15;
            while (n--) {
                slot[0] = slot[1];
                slot++;
            }
        } else {
            slot = &vis_table_c.entries[48 + vis_table_c.ground_item_counts[3]];
            vis_table_c.ground_item_counts[3]++;
        }
    } else if (vis_table_c.ground_item_counts[2] == 16) {
        int n;

        slot = &vis_table_c.entries[32];
        n = 15;
        while (n--) {
            slot[0] = slot[1];
            slot++;
        }
    } else {
        slot = &vis_table_c.entries[32 + vis_table_c.ground_item_counts[2]];
        vis_table_c.ground_item_counts[2]++;
    }
    slot->item = id;
    slot->block = e->block;
    slot->anim_slot.slot = NULL;
    slot->position = e->position;
    slot->position.vy -= e->height >> 1;
    /* One 8-byte copy of the rotation (rot_x..unk92) over the item's (rot_x..unk1E). */
    slot->rotation = e->rotation;
    slot->unk20 = rec->flags;
    slot->state = GROUND_ITEM_FALLING;
    slot->prev_state = 1;
    slot->draw_flags = 0;
    slot->player_dist_xz = e->player_dist_xz;
    slot->vertical_speed = 0;
    slot->bounces = 0;
    slot->area = area_state.area;
    vis_table_c.ground_item_counts[0]++;
    vis_table_c.placed_bits[id / 32] |= 1 << (id % 32);
}

/* Drops one of the entry's three items, each by its percent chance when
 * item_is_uncollected allows it. */
ADDRESS(0x8004da2c, 0xe0)
void creature_roll_drops(Creature *e)
{
    int r = rand() * 100 >> 15;

    if (item_is_uncollected(e->drop_item0) && r < e->drop_chance0)
        creature_drop_item(e, e->drop_item0);
    else if (item_is_uncollected(e->drop_item1) && r < e->drop_chance1)
        creature_drop_item(e, e->drop_item1);
    else if (item_is_uncollected(e->drop_item2) && r < e->drop_chance2)
        creature_drop_item(e, e->drop_item2);
}

/* Adds the entry's record bonuses to the viewer's 14 figures and total. */
ADDRESS(0x8004db0c, 0x90)
void creature_award_kill_bonuses(Creature *e)
{
    CreatureType *r = &vis_table_a.types[e->type];
    int i;

    for (i = 0; i < 14; i++) {
        viewer.base_attributes[i] += r->kill_bonus[i];
        viewer.sp_total += r->kill_bonus[i];
    }
    player_compute_derived_stats(0);
}

/* Whether the viewer looks at the entry within reach: 0 no, 1 yes, 2 dead
 * centre and close. */
ADDRESS(0x8004db9c, 0x288)
int creature_test_player_aim(Creature *e, short reach)
{
    short yaw;
    short pitch;
    short dy;
    short dy2;
    short lo;
    short hi;
    short half;
    int ay;

    yaw = (viewer.rotation.vy - e->player_heading) & 0xfff;
    pitch = viewer.rotation.vx & 0xfff;
    if (reach + e->radius < e->player_dist_xz)
        return 0;
    dy = e->position.vy - viewer.position.vy;
    dy2 = e->position.vy - e->height - viewer.position.vy;
    if (dy > 0 && dy2 > 0) {
        lo = ratan2(dy, e->player_dist_xz - e->radius);
        hi = ratan2(dy2, e->player_dist_xz + e->radius);
    } else if (dy < 0 && dy2 < 0) {
        lo = ratan2(dy, e->player_dist_xz + e->radius);
        hi = ratan2(dy2, e->player_dist_xz - e->radius);
    } else {
        lo = ratan2(dy, e->player_dist_xz);
        hi = ratan2(dy2, e->player_dist_xz);
    }
    half = ratan2(e->radius, e->player_dist_xz);
    if (yaw > 2048)
        yaw -= 4096;
    if (pitch > 2048)
        pitch -= 4096;
    if (lo > 2048)
        lo = 4096 - lo;
    if (hi > 2048)
        hi = 4096 - hi;
    if (pitch < hi || lo < pitch)
        return 0;
    ay = abs(yaw);
    if (ay < half && e->player_dist_xz < e->radius + reach) {
        if (pitch < hi >> 1 || lo >> 1 < pitch)
            return 1;
        if (ay < half >> 1 && e->player_dist_xz < (e->radius >> 1) + reach)
            return 2;
        return 1;
    }
    return 0;
}

/* Sums the record's weights over the 9 levels (arg 0 only). */
ADDRESS(0x8004de24, 0xd4)
int creature_calc_damage(Creature *e, int arg, u16 *levels)
{
    CreatureType *r = &vis_table_a.types[e->type];
    u16 *w;
    int l;
    int m;
    int sum;
    int i;

    sum = 0;
    if (arg == 0) {
        w = r->resistances;
        for (i = 0; i < 9; i++, levels++, w++) {
            l = *levels;
            if (l != 0) {
                m = *w ?: 1;
                sum += l * 50000 / (m * (1000 - l));
            }
        }
    }
    return sum;
}
