/*
 * ST.EXE: table A entries (vis_table_a.entries) taking damage, the per-frame
 * pass over them, and loading them from an area file (WIP module; the damage
 * helpers 0x8004da2c..0x8004de24 lie just before it and may share its file).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/memory.h>
#include <st/menu.h>
#include <st/anim.h>
#include <st/player.h>
#include <st/funcs.h>
#include <st/vis.h>

/* Takes `power` damage (times three for a critical hit), doubled by luck or
 * kind 2; on death starts the record's 'P' script, on a heavy hit its '@'
 * script. Returns the damage dealt, or -1 for an entry in state 2. */
ADDRESS(0x8004def8, 0x3e0)
int creature_take_damage(e, kind, power, levels, crit)
    Creature *e;
    int kind;
    short power;
    u16 *levels;
    u8 crit;
{
    CreatureType *t;
    CreatureScript **p;
    CreatureScript *script = NULL;
    int dmg;
    int found;
    u8 index;
    int luck;
    int i;

    e->hit = 1;
    if (e->status.bytes[3] == 2)
        return -1;
    dmg = creature_calc_damage(e, 0, levels);
    if (crit)
        dmg *= 3;
    dmg = dmg * power / 5120;
    if (dmg == 0)
        return 0;
    t = &vis_table_a.types[e->type];
    if (viewer.status_flags & 0x200000)
        dmg = dmg * 3 / 2;
    luck = viewer.double_damage_chance;
    if (rand() * 100 / 32768 < luck) {
        sound_play(21, 128);
        dmg <<= 1;
    }
    i = 24;
    if (kind == 2)
        dmg <<= 1;
    found = 0;
    e->status.bytes[2] &= 0xca;
    index = 0;
    if (dmg >= e->hp) {
        if (e->status.bytes[3] != 3) {
            e->state = 5;
            creature_roll_drops(e);
            creature_award_kill_bonuses(e);
            game_progress.creature_bits_and_vs_data[5 + (t->kind >> 5)] |= 1 << (t->kind & 0x1f);
            if (e < &vis_table_a.entries[48]) {
                game_progress.area_kills[area_state.area]++;
                viewer.creatures_killed++;
            }
            e->hp = 0;
            e->status.bytes[2] |= 0x15;
            p = t->scripts;
            while (i--) {
                script = *p++;
                if (script == NULL)
                    break;
                if (script->command == 'P') {
                    found = 1;
                    break;
                }
                index++;
            }
            goto done;
        }
    } else if (e->status.bytes[3] != 3) {
        e->hp -= dmg;
    }
    e->status.bytes[2] |= 0x20;
    if (dmg >= e->hp * 15 / 100) {
        p = t->scripts;
        while (i--) {
            script = *p++;
            if (script == NULL)
                break;
            if (script->command == '@') {
                found = 1;
                break;
            }
            index++;
        }
    }
done:
    if (found) {
        e->script_index = index;
        e->script = script;
        e->command = script->command;
        e->script_state = 0;
        e->anim_frame = 0;
        e->anim_step = script->anim_step;
    }
    e->hurt_timer = 0xfff;
    if (viewer.state != 4) {
        if (viewer.hp_drain_percent)
            player_add_hp(dmg * viewer.hp_drain_percent / 100);
        if (viewer.mp_drain_percent)
            player_add_mp(dmg * viewer.mp_drain_percent / 100);
    }
    return (short)dmg;
}

/* Gives every entry of the first 32 left in the previous area (0xfd) that
 * area's id. */
ADDRESS(0x8004e2d8, 0x4c)
void creatures_bind_previous_area(void)
{
    Creature *e = vis_table_a.entries;
    int i = 32;

    while (i--) {
        if (e->zone == 0xfd)
            e->zone = vis_table_a.prev_zone;
        e++;
    }
}

/* The per-frame table A pass: on an area change frees the animation files
 * and parks the entries of other areas, then measures each entry against
 * the viewer, runs it and moves it. */
ADDRESS(0x8004e324, 0x5c8)
void creatures_update(void)
{
    MapBlock *r = viewer.block_record;
    Creature *e;
    CreatureType *t;
    StAnimFile *file;
    StAnimFile *anim;
    int n;
    int dx;
    int dy;
    int dz;
    int d;
    u16 h;

    if (r != NULL && r->zone != 0xff) {
        u8 prev = vis_table_a.zone;

        vis_table_a.prev_zone = prev;
        vis_table_a.zone = r->zone;
        if (prev != vis_table_a.zone) {
            anim_slots_free_idle();
            anim_sets_free_idle();
            anim_slots_free_all();
            anim_sets_free_all();
            n = 16;
            while (n--) {
                anim_get_file(n + 128, &file);
                if (file)
                    heap_free(file);
            }
            creatures_bind_previous_area();
            n = 16;
            while (n--) {
                anim_get_file(n + 112, &file);
                if (file)
                    heap_free(file);
            }
            e = vis_table_a.entries;
            for (vis_table_a.current_index = 0; vis_table_a.current_index < 64; vis_table_a.current_index++, e++) {
                t = &vis_table_a.types[e->type];
                if (e->state == 1) {
                    if (vis_table_a.zone == 0xff)
                        break;
                    if (vis_table_a.zone != e->zone) {
                        if (e->zone == 0xff && e->player_dist_xz < 0x7000)
                            break;
                        vis_table_a.unk612C -= t->unk12;
                        vis_table_a.spawn_cost_total -= e->spawn_cost;
                        e->state = 0;
                        creature_return_home(e);
                    }
                }
            }
        }
    }
    vis_table_a.active_count = 0;
    for (e = vis_table_a.entries, vis_table_a.current_index = 0; vis_table_a.current_index < 64;
         e++, vis_table_a.current_index++) {
        if (e->spawn_chance != 0xff) {
            creature_set_current(e);
            dx = e->position.vx - viewer.position.vx;
            dy = e->position.vy - viewer.position.vy;
            dz = e->position.vz - viewer.position.vz;
            if (dx < -0x8000 || dx > 0x8000 || dy > 0x8000 || dy < -0x8000 || dz > 0x8000
                || dz < -0x8000) {
                e->player_dist_sq = 0x40000000;
                e->player_dist_xz = 0x8000;
            } else {
                e->player_dist_sq = dx * dx + dy * dy + dz * dz;
                e->player_dist_xz = SquareRoot0(dx * dx + dz * dz);
            }
            h = (ratan2(dz, dx) - 0x400) & 0xfff;
            e->player_heading = h;
            d = ((u16)e->facing - h) & 0xfff;
            e->player_bearing = d;
            if (d > 0x800)
                e->player_bearing = 0x1000 - d;
            e->velocity.vz = 0;
            e->velocity.vy = 0;
            e->velocity.vx = 0;
            if ((vis_table_a.frame & 3) == (e->zone & 3) || viewer.update_all_spawns)
                creature_update_spawn();
            if (e->state == 1) {
                if ((vis_table_a.frame & 3) == (vis_table_a.active_count & 3))
                    area_state.hooks->creature_pick_move();
                area_state.hooks->creature_step_script();
                if (e->fade != -1) {
                    anim_get_file(e->model_slot + 128, &anim);
                    if (anim) {
                        e->fade -= 128;
                        if (e->fade < 0) {
                            e->fade = -1;
                            e->status.bytes[2] &= 0xf2;
                        }
                    }
                }
                e->touched_player = 0;
                if (*(s32 *)&e->velocity != 0 || e->velocity.vz != 0 || e->vertical_speed != 0) {
                    creature_move(e);
                    if (!(viewer.status_flags & 0x40000)) {
                        e->position.vx += e->velocity.vx;
                        e->position.vy += e->velocity.vy;
                        e->position.vz += e->velocity.vz;
                    }
                }
                vis_table_a.active_count++;
            } else if (e->state == 5) {
                area_state.hooks->creature_step_script();
            }
        }
        e->unk23 = 0;
    }
    vis_table_a.zone_history[3] = vis_table_a.zone_history[2];
    vis_table_a.zone_history[2] = vis_table_a.zone_history[1];
    vis_table_a.zone_history[1] = vis_table_a.zone_history[0];
    vis_table_a.zone_history[0] = vis_table_a.zone;
    vis_table_a.frame++;
    vis_table_a.push_x >>= 1;
    vis_table_a.push_y >>= 1;
    vis_table_a.push_z >>= 1;
    if (abs(vis_table_a.push_x) < 4)
        vis_table_a.push_x = 0;
    if (abs(vis_table_a.push_y) < 4)
        vis_table_a.push_y = 0;
    if (abs(vis_table_a.push_z) < 4)
        vis_table_a.push_z = 0;
    creature_set_current(NULL);
    if (DAT_801cd0e4)
        player_warp_back();
}

/* Turns the script offsets of the loaded records into pointers. */
ADDRESS(0x8004e8ec, 0x74)
void creature_types_link_scripts(void)
{
    CreatureType *t = vis_table_a.types;
    CreatureScript **p;
    int i;
    int j;

    for (i = 0; i < 16; i++, t++) {
        if (t->unk00 == 0xff)
            return;
        p = t->scripts;
        for (j = 0; j < 24; j++, p++) {
            if ((s32)*p == -1)
                *p = NULL;
            else
                *p = (CreatureScript *)((u8 *)&vis_table_a.types[16] + (s32)*p);
        }
    }
}

/* Loads the 64 table A entries from an area file. */
ADDRESS(0x8004e960, 0x3f4)
void creatures_load(CreatureFileEntry *src)
{
    u8 cnt[4];
    u8 map[16];
    u32 ids[3];
    Creature *e;
    CreatureType *t;
    ItemType *c;
    u16 i;
    u8 k;
    int j;

    vis_table_a.push_z = 0;
    vis_table_a.push_y = 0;
    vis_table_a.push_x = 0;
    fill_words((u32 *)cnt, 0, 1);
    fill_words((u32 *)map, -1, 4);
    e = vis_table_a.entries;
    i = 64;
    while (i--) {
        e->spawn_chance = src->spawn_chance;
        e->state = 0;
        if (e->spawn_chance == 0xff)
            goto next;
        e->type = src->type;
        t = &vis_table_a.types[e->type];
        k = src->type;
        if (map[k] == 0xff) {
            map[k] = e->vram_tag = cnt[k >> 2];
            k >>= 2;
            cnt[k] = (cnt[k] + t->vram_tag_count) & 3;
        } else {
            e->vram_tag = map[k];
        }
        e->home_block = src->home_block;
        e->region = vis_records[e->home_block].region;
        e->zone = vis_records[e->home_block].zone;
        e->unk09 = t->vram_tag_count;
        e->model_slot = e->type;
        e->visibility = 0;
        e->drop_item0 = ids[0] = src->drop_items[0];
        e->drop_item1 = ids[1] = src->drop_items[1];
        e->drop_item2 = ids[2] = src->drop_items[2];
        e->drop_chance0 = src->drop_chance0;
        e->drop_chance1 = src->drop_chance1;
        e->drop_chance2 = src->drop_chance2;
        e->home_facing = src->home_facing & 0xfff;
        e->home_offset_x = src->home_offset_x;
        e->home_offset_y = src->home_offset_y;
        e->home_offset_z = src->home_offset_z;
        creature_copy_type_stats(e);
        e->anim = 0;
        e->anim_frame = 0;
        e->unk1B = 0;
        e->chain_script = 0;
        e->spawn_cost = src->spawn_cost;
        e->move_mode = t->move_mode;
        e->command = 2;
        e->vertical_speed = 0;
        e->script_state = 0xff;
        e->script = NULL;
        e->voice = -1;
        e->depth_bias = t->depth_bias;
        e->unk0A = t->kind;
        e->radius = t->radius;
        e->height = t->height;
        e->collision_tests = 178;
        if (63 - i >= 56)
            e->status.bytes[3] = 1;
        else
            e->status.bytes[3] = 0;
        e->status.bytes[0] = 0;
        e->hit = 0;
        creature_return_home(e);
        e->spawn_y = e->position.vy;
        for (j = 0; j < 3; j++) {
            if (ids[j] != 0xffff) {
                c = &vis_table_c.types[ids[j]];
                if (vis_table_c.model_slots[c->model] == 0xff)
                    vis_table_c.model_slots[c->model] = vis_table_c.next_model_slot++;
            }
        }
        if (e->flags & 0x10) {
            if (t->unk00 == 2) {
                if (e->home_offset_y == -1)
                    e->home_offset_y = t->max_hp;
                if (e->home_offset_z == -1)
                    e->home_offset_z = t->unk08;
            } else {
                t->unk00 = 1;
                e->home_offset_z = 0;
            }
        }
    next:
        src++;
        e++;
    }
    vis_table_a.unk6130 = 0;
    vis_table_a.unk612C = 0;
    vis_table_a.spawn_cost_total = 0;
}
