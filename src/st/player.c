/*
 * ST.EXE: the player (WIP module, one translation unit): death and target
 * queries, status, wear and equipment, the per-frame player update and its
 * item, magic and area hooks, 0x8003bd14..0x800443ec. Retail keeps the
 * constants of the whole run before its switch tables (player's pool
 * 0x80011c7c, then player_8003e5e8's at 0x80011cb4, then both units'
 * tables from 0x80011cfc), which only one -G8 object produces.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <stdarg.h>
#include <kf/lib/null.h>
#include <RAND.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <INLINE_C.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/anim.h>
#include <st/data.h>
#include <st/effect.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/hud.h>
#include <st/light.h>
#include <st/menu.h>
#include <st/player.h>
#include <st/scratch.h>
#include <st/sound.h>
#include <st/vis.h>
#include <st/card.h>
#include <st/item.h>
#include <st/menu_main.h>

RODATA(0x80011c7c, 0x29c)
SDATA(0x8008802c, 0x10)
SDATA(0x80088040, 0x10)
SDATA(0x80088054, 0x10)

/* Kills the player (state 4). */
ADDRESS(0x8003bd14, 0x9c)
void player_kill(int unused)
{
    if (viewer.state != 4) {
        viewer.state = 4;
        viewer.hp = 0;
        play_death_sound();
        world_state.no_plane_test = 1;
        viewer.death_timer = 0;
        viewer.unk17C = 170;
        viewer.death_pitch_step = (-1024 - (viewer.pitch & 0x7ff)) / 24;
    }
}

ADDRESS(0x8003bdb0, 0x60)
void player_check_fall_death(void)
{
    if (viewer.vertical_speed >= 1152 && vis_lists.floor_dist >= 1600) {
        player_kill(0);
        viewer.fall_death = 1;
    }
}

ADDRESS(0x8003be10, 0xa4)
void player_update_submersion(void)
{
    if (vis_lists.surface_y != 0x80000) {
        if (vis_lists.surface_y < viewer.position.vy) {
            viewer.submerged = 1;
        } else {
            viewer.submerged = 0;
        }
    }
    if (vis_lists.acid_surface_y != 0x80000) {
        if (vis_lists.acid_surface_y < viewer.position.vy) {
            viewer.submerged_acid = 1;
        } else {
            viewer.submerged_acid = 0;
        }
    }
    if (area_state.loaded_area == 10 && viewer.block == 327) {
        viewer.submerged_acid = 0;
    }
}

/* The viewer's distance to `t` when it is within t's reach, else -1. */
ADDRESS(0x8003beb4, 0x14c)
int player_distance_in_reach(void *request)
{
    PlayerTarget *t = request;
    int dx;
    int dz;
    int dist;

    dx = viewer.position.vx - t->position.vx;
    if (dx >= -10240 && dx <= 10240) {
        dz = viewer.position.vz - t->position.vz;
        if (dz >= -10240 && dz <= 10240) {
            dist = SquareRoot0(dx * dx + dz * dz);
            if (dist - t->speed * rcos(t->heading - (short)ratan2(dz, dx)) / 4096 < t->radius + 256) {
                if (viewer.position.vy + 1600 < t->position.vy) {
                    if (viewer.position.vy + 1600 < t->position.vy - t->height) {
                        return -1;
                    }
                } else if (t->position.vy < viewer.position.vy) {
                    return -1;
                }
                return dist;
            }
        }
    }
    return -1;
}

ADDRESS(0x8003c000, 0x54)
int func_8003c000(int x, int y, int z, int range, int kind)
{
    return vector_distance_xz_within_rev((VECTOR *)&viewer.position.vx, x, y, z, range + 256, 1000, kind);
}

ADDRESS(0x8003c054, 0x8c)
void player_reset_state(void)
{
    viewer.bob_offset = 0;
    viewer.bob_phase = 0;
    viewer.unk2D2 = 0;
    viewer.unk239 = 0;
    viewer.vertical_speed = 0;
    viewer.state = 0;
    viewer.unk294 = 0;
    viewer.unk292 = 0;
    viewer.fall_pitch = 0;
    viewer.shake_roll = 0;
    viewer.shake_yaw = 0;
    viewer.shake_pitch = 0;
    viewer.roll_offset = 0;
    viewer.rot_offset.halves[1] = 0;
    viewer.rot_offset.halves[0] = 0;
    *(SVECTOR *)&viewer.pitch = viewer.view_rot;
    viewer.shot_magic[0] = 255;
    viewer.shot_magic[1] = 255;
    viewer.submerged_acid = 0;
    viewer.submerged = 0;
    vis_lists.floor_kinds = 0;
    viewer.shake_timer = -1;
}

ADDRESS(0x8003c0e0, 0x9c)
void player_reload_equipment(void)
{
    player_load_equipment_model(viewer.equipment[EQUIP_SLOT_RIGHT_HAND], 0, 0);
    player_load_equipment_model(viewer.equipment[EQUIP_SLOT_LEFT_HAND], 1, 0);
    player_load_equipment_model(viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE], 2, 0);
    player_load_equipment_model(viewer.equipment[EQUIP_SLOT_LEFT_RESERVE], 3, 0);
    player_load_magic(viewer.equipment[EQUIP_SLOT_RIGHT_MAGIC], 0);
    player_load_magic(viewer.equipment[EQUIP_SLOT_LEFT_MAGIC], 1);
    player_reset_state();
    player_recalc_stats();
    lighting.last_area = 255;
}

/*
 * Steps a status timer: the level climbs by one towards its peak, then the
 * peak clears and the level falls back to 0. Returns max scaled by the level
 * over `ramp` (max once the level reaches it), or -1 when the timer is idle;
 * the new values are stored only when `apply` is set.
 */
ADDRESS(0x8003c17c, 0xd0)
short player_step_status_timer(short *level, short *peak, int ramp, int max, int apply)
{
    short p;
    short l;
    short v;

    l = *level;
    p = 0;
    if (peak) {
        p = *peak;
    }
    if (l != 0 || p != 0) {
        if (l < p) {
            l++;
        } else {
            p = 0;
            l--;
        }
        if (l < ramp) {
            v = l * max / ramp;
        } else {
            v = max;
        }
        if (apply) {
            *level = l;
            if (peak) {
                *peak = p;
            }
        }
        return v;
    }
    return -1;
}

/* Steps the status timers and the damage fades once per frame. */
ADDRESS(0x8003c24c, 0x710)
void player_update_status(int apply)
{
    short level;

    if (viewer.red_fade != 0) {
        viewer.red_fade -= viewer.red_fade_step;
        if (viewer.red_fade < 0) {
            viewer.red_fade = 0;
        }
        if (viewer.red_fade >= 4096) {
            level = 4096;
        } else {
            level = viewer.red_fade;
        }
        fade_add_color(60, 0, 0, level, 1);
    }
    if (viewer.yellow_fade != 0) {
        viewer.yellow_fade -= viewer.yellow_fade_step;
        if (viewer.yellow_fade < 0) {
            viewer.yellow_fade = 0;
        }
        if (viewer.yellow_fade >= 4096) {
            level = 4096;
        } else {
            level = viewer.yellow_fade;
        }
        fade_add_color(60, 60, 0, level, 1);
    }

    level = player_step_status_timer(&viewer.unk190[0][0], &viewer.unk190[0][1], 20, 4096, apply);
    if (level != -1) {
        fade_add_color(0, 40, 0, level, 1);
        if (viewer.timers.words[0] == 0) {
            viewer.status_flags &= ~7;
        }
    }
    level = player_step_status_timer(&viewer.unk190[1][0], &viewer.unk190[1][1], 20, 4096, apply);
    if (level != -1) {
        if ((viewer.unk190[1][0] & 3) == 3) {
            level = 4096;
        } else {
            level -= 512;
        }
        fade_add_color(40, 40, 0, level, 1);
        player_stop_motion();
        vis_table_b.push_x = vis_table_b.push_y = vis_table_b.push_z = 0;
        vis_lists.floor_push_x = vis_lists.floor_push_y = vis_lists.floor_push_z = 0;
        viewer.push_x = viewer.push_y = viewer.push_z = vis_table_a.push_x = vis_table_a.push_y =
            vis_table_a.push_z = 0;
        if (viewer.timers.words[1] == 0) {
            viewer.status_flags &= ~0x18;
        }
    }
    level = player_step_status_timer(&viewer.unk190[2][0], &viewer.unk190[2][1], 0, 4096, apply);
    if (level != -1) {
        fade_add_color(20, 20, 20, level, 2);
        if (viewer.timers.words[2] == 0) {
            viewer.status_flags &= ~0x60;
            lighting.row_request |= 4;
        }
    }
    level = player_step_status_timer(&viewer.unk190[3][0], &viewer.unk190[3][1], 20, 4096, apply);
    if (level != -1) {
        if (viewer.timers.words[3] == 0) {
            viewer.status_flags &= ~0x80;
            player_compute_derived_stats(0);
            PLAYER_CLAMP_HP();
        }
    }
    level = player_step_status_timer(&viewer.unk190[4][0], &viewer.unk190[4][1], 20, 4096, apply);
    if (level != -1) {
        fade_add_color(40, 0, 20, level, 1);
        if (viewer.timers.words[4] == 0) {
            viewer.status_flags &= ~0x100;
            player_compute_derived_stats(0);
            PLAYER_CLAMP_HP();
        }
    }
    if (viewer.submerged) {
        fade_add_color(0, 0, 40, 4096, 1);
    }
    if (viewer.submerged_acid) {
        fade_add_color(40, 40, 0, 4096, 1);
    }
    level = player_step_status_timer(&viewer.unk190[5][0], &viewer.unk190[5][1], 0, 4096, apply);
    if (level != -1) {
        lighting.row_request |= 0x10;
        lighting.column_request |= 2;
        if (viewer.timers.words[5] == 0) {
            viewer.status_flags &= ~0x20000;
            viewer.status_flags &= ~0x10000;
        }
    }
    level = player_step_status_timer(&viewer.unk190[6][0], &viewer.unk190[6][1], 0, 4096, apply);
    if (level != -1) {
        if (viewer.timers.words[6] == 0) {
            viewer.status_flags &= ~0x40000;
        }
    }
    level = player_step_status_timer(&viewer.unk190[7][0], &viewer.unk190[7][1], 0, 4096, apply);
    if (level != -1) {
        if (viewer.timers.words[7] == 0) {
            viewer.status_flags &= ~0x80000;
        }
    }
    level = player_step_status_timer(&viewer.unk190[8][0], &viewer.unk190[8][1], 0, 4096, apply);
    if (level != -1) {
        if (viewer.timers.words[8] == 0) {
            viewer.status_flags &= ~0x100000;
        }
    }
    level = player_step_status_timer(&viewer.unk190[9][0], &viewer.unk190[9][1], 0, 4096, apply);
    if (level != -1) {
        if (viewer.timers.words[9] == 0) {
            viewer.status_flags &= ~0x200000;
        }
    }
    level = player_step_status_timer(&viewer.unk190[10][0], &viewer.unk190[10][1], 0, 4096, apply);
    if (level != -1) {
        lighting.force_first_column = 1;
        lighting.row_request |= 0x20;
        lighting.column_request |= 1;
        if (viewer.timers.words[10] == 0) {
            viewer.status_flags &= ~0x400000;
        }
    }
    level = player_step_status_timer(&viewer.unk190[11][0], &viewer.unk190[11][1], 0, 4096, apply);
    if (level != -1) {
        if (viewer.timers.words[11] == 0) {
            viewer.status_flags &= ~0x800000;
        }
    }
    level = player_step_status_timer(&viewer.unk190[12][0], &viewer.unk190[12][1], 0, 4096, apply);
    if (level != -1) {
        if (viewer.timers.words[12] == 0) {
            viewer.status_flags &= ~0x800;
        }
    }
}

ADDRESS(0x8003c95c, 0x188)
void area_load_set_blocking(int r, int g, int b, int arg3, int arg4, int arg5)
{
    cd_wait_idle();
    do {
        cd_wait_frame();
        area_load_update();
    } while (area_state.loading != 0);
    area_state.loaded_area = 99;
    area_state.loaded_world = 99;
    area_state.loaded_images = 99;
    area_state.loaded_sound_bank = 99;
    area_state.unk8 = 99;
    area_request_load(r, g, b, 255, 255, 127, 127, 127, 0, 0, 0);
    do {
        cd_wait_frame();
        area_load_update();
    } while (area_state.loading != 0);
    cd_wait_idle();
    VSync(2);
    player_reset_motion();
    lighting_load_defaults();
    fade_reset();
    lighting_build_matrices();
    fade_run_frames(130, 4096, 4096, 0);
    cd_wait_idle();
    area_state.entry_fade_start = cd_state.vsync_count;
    area_request_load(255, 255, 255, arg3, arg4, 127, 127, 127, 0, 0, 0);
    lighting_clear_requests();
}

ADDRESS(0x8003cae4, 0xa4)
void player_reset_stats(void)
{
    int i;

    viewer.sp_total = 0;
    i = 14;
    while (i--) {
        viewer.base_attributes[i] = 50;
        viewer.attributes[i] = 50;
        viewer.attribute_bonuses[i] = 0;
    }
    viewer.unk26 = 0;
    viewer.max_mp = 0;
    viewer.max_hp = 0;
    viewer.equipment[EQUIP_SLOT_QUICK_4] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_QUICK_2] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_QUICK_5] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_RIGHT_HAND] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_GLOVES] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_RIGHT_BRACELET] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_RIGHT_RING] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_RIGHT_MAGIC] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_QUICK_3] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_BOOTS] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_ARMOR] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_HELMET] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_AMULET] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_LEFT_MAGIC] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_QUICK_1] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_LEFT_RESERVE] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_LEFT_HAND] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_GLOVES_MIRROR] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_LEFT_BRACELET] = EQUIP_NONE;
    viewer.equipment[EQUIP_SLOT_LEFT_RING] = EQUIP_NONE;
}

/*
 * Totals the bonuses of the equipped items (equipment slots other than 4..6,
 * 11, 12 and 15) into the viewer's derived stats.
 */
ADDRESS(0x8003cb88, 0x354)
void player_sum_equipment_bonuses(void)
{
    int kind[4];
    int amount[3];
    int i;
    int j;
    int a1;
    int a2;
    int a3;
    int a4;
    int a5;
    int a6;
    int a7;
    int a8;
    u16 *slot;
    ItemType *item;

    slot = viewer.equipment;
    a1 = 0;
    a2 = 0;
    a3 = 0;
    a4 = 0;
    a5 = 0;
    a6 = 0;
    a7 = 0;
    a8 = 0;
    for (i = 0; i < 17; i++, slot++) {
        if (i >= 4 && i <= 6) {
            continue;
        }
        if (i == 11) {
            continue;
        }
        if (i == 12) {
            continue;
        }
        if (i == 15) {
            continue;
        }
        if (*slot == 0xffff) {
            continue;
        }
        item = &vis_table_c.types[*slot];
        kind[0] = item->kind0;
        kind[1] = item->kind1;
        kind[2] = item->kind2;
        amount[0] = item->amount0;
        amount[1] = item->amount1;
        amount[2] = item->amount2;
        for (j = 0; j < 3; j++) {
            switch (kind[j]) {
            case 1:
                a1 += amount[j];
                if (a1 > 100) {
                    a1 = 100;
                }
                break;
            case 2:
                a2 += amount[j];
                if (a2 > 100) {
                    a2 = 100;
                }
                break;
            case 3:
                a3 += amount[j];
                if (a3 > 20) {
                    a3 = 20;
                }
                break;
            case 4:
                if (a4 >= 0) {
                    if (amount[j] == 1) {
                        a4 = 1;
                    } else if (amount[j] == 2) {
                        a4 = -1;
                    }
                }
                break;
            case 5:
                a5 += amount[j];
                if (a5 > 100) {
                    a5 = 100;
                }
                break;
            case 6:
                a6 += amount[j];
                if (a6 > 100) {
                    a6 = 100;
                }
                break;
            case 7:
                a7 += amount[j];
                if (a7 > 100) {
                    a7 = 100;
                }
                break;
            case 8:
                a8 = 1;
                break;
            }
        }
    }
    if (a1) {
        viewer.hp_regen_interval = 100 / a1;
    } else {
        viewer.hp_regen_interval = 0;
    }
    if (a2) {
        viewer.mp_regen_interval = 100 / a2;
    } else {
        viewer.mp_regen_interval = 0;
    }
    viewer.double_damage_chance = a3 * 5;
    if (a4 < 0) {
        viewer.equipment_light = -1;
    } else if (a4 > 0) {
        viewer.equipment_light = 1;
    } else {
        viewer.equipment_light = 0;
    }
    viewer.status_time_scale = 100 - a5 * 80 / 100;
    viewer.hp_drain_percent = a6;
    viewer.mp_drain_percent = a7;
    viewer.blocks_magic = a8;
}

ADDRESS(0x8003cedc, 0x4c)
void player_recalc_stats(void)
{
    player_sum_equipment_bonuses();
    player_compute_derived_stats(0);
    PLAYER_CLAMP_HP();
}

ADDRESS(0x8003cf28, 0x6c)
void player_add_hp(int delta)
{
    int hp;

    hp = viewer.hp + delta;
    if (hp <= 0) {
        hp = 0;
        player_kill(hp);
    } else if (viewer.max_hp < hp) {
        hp = viewer.max_hp;
    }
    viewer.hp = hp;
}

ADDRESS(0x8003cf94, 0x48)
void player_add_mp(int delta)
{
    int mp;

    mp = viewer.mp + delta;
    if (mp <= 0) {
        mp = 0;
    } else if (viewer.max_mp < mp) {
        mp = viewer.max_mp;
    }
    viewer.mp = mp;
}

ADDRESS(0x8003cfdc, 0x54)
void player_check_item_durability(int id)
{
    int v;

    v = vis_table_c.types[id].durability;
    if (v != 0) {
        if (v < 4) {
            viewer.status_flags |= 0x1000000;
        }
    }
}

/* Clears the durability-warning flag, then sets it again if any worn item
 * (weapons and armour slots) is nearly broken. */
ADDRESS(0x8003d030, 0xd4)
void player_update_durability_warning(void)
{
    u16 *slots[10] = {
        &viewer.equipment[EQUIP_SLOT_RIGHT_RING], &viewer.equipment[EQUIP_SLOT_GLOVES],
            &viewer.equipment[EQUIP_SLOT_RIGHT_HAND],  &viewer.equipment[EQUIP_SLOT_HELMET],
        &viewer.equipment[EQUIP_SLOT_ARMOR], &viewer.equipment[EQUIP_SLOT_BOOTS],
            &viewer.equipment[EQUIP_SLOT_LEFT_RING], &viewer.equipment[EQUIP_SLOT_LEFT_HAND],
    };
    u16 **p;

    p = slots;
    viewer.status_flags &= ~0x1000000;
    for (; *p; p++) {
        if (**p != 0xffff) {
            player_check_item_durability(**p);
        }
    }
}

/* Takes `amount` durability from the item in equipment slot `slot`; a
 * broken item is unequipped (except in slots 0 and 13) and its model
 * dropped. */
ADDRESS(0x8003d104, 0x184)
void player_wear_item(int amount, u16 *slot)
{
    int left;

    if (*slot == 0xffff) {
        return;
    }
    left = vis_table_c.types[*slot].durability;
    if (left == 0) {
        return;
    }
    left -= amount;
    if (left <= 0) {
        sound_play(4, 128);
        vis_table_c.types[*slot].durability = 0;
        if (slot != &viewer.equipment[EQUIP_SLOT_RIGHT_RING]
            && slot != &viewer.equipment[EQUIP_SLOT_LEFT_RING]) {
            *slot = 0xffff;
        }
        if (slot == &viewer.equipment[EQUIP_SLOT_RIGHT_HAND]) {
            if (viewer.slot_model_ids[0] >= 31 && viewer.slot_model_ids[0] <= 34) {
                player_load_equipment_model(0xffff, 1, 0);
            }
            player_load_equipment_model(0xffff, 0, 0);
        } else if (slot == &viewer.equipment[EQUIP_SLOT_LEFT_HAND]) {
            player_load_equipment_model(0xffff, 1, 0);
        } else if (slot == &viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE]) {
            player_load_equipment_model(0xffff, 2, 0);
        } else if (slot == &viewer.equipment[EQUIP_SLOT_LEFT_RESERVE]) {
            player_load_equipment_model(0xffff, 3, 0);
        }
        player_sum_equipment_bonuses();
        player_compute_derived_stats(0);
        PLAYER_CLAMP_HP();
    } else {
        vis_table_c.types[*slot].durability = left;
    }
    player_update_durability_warning();
}

/* Wears the shield (slot 16) while it is raised, and the armour. */
ADDRESS(0x8003d288, 0x30c)
void player_wear_armour(int damage, u16 flags)
{
    u16 *armour[4] = {&viewer.equipment[EQUIP_SLOT_HELMET], &viewer.equipment[EQUIP_SLOT_ARMOR],
        &viewer.equipment[EQUIP_SLOT_BOOTS], &viewer.equipment[EQUIP_SLOT_GLOVES]};
    int wear;
    int n;
    int shield;
    int pick;
    int i;
    u8 a;
    u8 b;

    if (viewer.shield_raised != 0 && viewer.equipment[EQUIP_SLOT_LEFT_HAND] != EQUIP_NONE) {
        if (damage * 20 < viewer.max_hp) {
            viewer.shield_wear++;
        } else {
            n = damage * 10 / viewer.max_hp + 1;
            if (n <= 0) {
                return;
            }
            viewer.shield_wear += n * 2;
            viewer.armour_wear += n;
        }
        a = viewer.shield_wear;
        b = viewer.armour_wear;
        shield = a / 5;
        wear = b / 5;
        viewer.shield_wear = a - shield * 5;
        viewer.armour_wear = b - wear * 5;
        if ((flags & 0x400) && !(viewer.status_flags & 0x800000)) {
            shield <<= 1;
            wear <<= 1;
        }
        if (flags & 0x1000) {
            shield *= 5;
            wear *= 5;
        }
        player_wear_item(shield, &viewer.equipment[EQUIP_SLOT_LEFT_HAND]);
        pick = -1;
    } else {
        if (damage * 20 < viewer.max_hp) {
            return;
        }
        n = damage * 10 / viewer.max_hp + 1;
        if (n <= 0) {
            return;
        }
        viewer.armour_wear += n * 2;
        wear = viewer.armour_wear / 5;
        viewer.armour_wear -= wear * 5;
        if ((flags & 0x400) && !(viewer.status_flags & 0x800000)) {
            wear <<= 1;
        }
        if (flags & 0x1000) {
            wear *= 5;
        }
        pick = rand() * 5 >> 15;
        if (pick >= 5) {
            pick = 4;
        }
    }
    i = 3;
    do {
        if (*armour[i] != 0xffff) {
            if (i == pick)
                player_wear_item(wear * 2, armour[i]);
            else
                player_wear_item(wear, armour[i]);
        }
    } while (--i != -1);
}

/* The screen shake offsets player_update_shake adds to the view angles. */
DATA(0x800881d0, 0x4, ".sbss")
static short shake[2];

/*
 * The area module's overlay pointers (st.area_load sets both from
 * overlay_area_start) and the unreferenced word before them: retail allocates
 * the three as one small COMMON batch right after shake, in hash-bucket
 * order. They are this object's: another object's small data would start
 * eight-aligned, and st.area_load reads DAT_800881dc with lui pairs, so it
 * does not request it (its tentative DAT_800881d8 merges into this one).
 */
DATA(0x800881d4, 0x4, ".sbss")
s32 DAT_800881d4;
DATA(0x800881d8, 0x4, ".sbss")
u8 *DAT_800881d8;
DATA(0x800881dc, 0x4, ".sbss")
u8 *DAT_800881dc[1];

/* Steps the screen shake started by player_take_damage. */
ADDRESS(0x8003d594, 0x1f4)
void player_update_shake(void)
{
    if (viewer.shake_timer < 0) {
        return;
    }
    if (viewer.shake_timer % 3 == 0) {
        shake[0] = (rand() - 16384) >> ((30 - viewer.shake_timer) / 5 + 5);
        shake[1] = (rand() - 16384) >> ((30 - viewer.shake_timer) / 5 + 5);
    } else {
        shake[0] >>= 1;
        shake[1] >>= 1;
    }
    if (shake[0] < -2 || shake[0] > 2) {
        viewer.shake_pitch += shake[0];
    }
    if (shake[1] < -2 || shake[1] > 2) {
        viewer.shake_roll += shake[1];
    }
    if (viewer.shake_pitch > 2048 && viewer.shake_pitch < 4032) {
        viewer.shake_pitch = 4032;
    }
    if (viewer.shake_pitch > 64 && viewer.shake_pitch < 2048) {
        viewer.shake_pitch = 64;
    }
    if (viewer.shake_roll > 2048 && viewer.shake_roll < 4032) {
        viewer.shake_roll = 4032;
    }
    if (viewer.shake_roll > 64 && viewer.shake_roll < 2048) {
        viewer.shake_roll = 64;
    }
    viewer.shake_timer--;
}

/* Takes `damage` hit points, pushes the viewer away from `from` and starts
 * the red fade and the screen shake. */
ADDRESS(0x8003d788, 0x458)
void player_take_damage(int damage, int flags, long *from)
{
    SVECTOR dir;
    SVECTOR push;
    int hp;
    int scaled;
    int reach;
    int amount;
    int random;
    int shake_amount;
    short time;

    if (viewer.state == 4 || damage == 0) {
        return;
    }
    player_wear_armour(damage, flags);
    hp = viewer.hp - damage;
    if (hp <= 0) {
        hp = 0;
    }
    viewer.hp = hp;
    PLAYER_CLAMP_HP();
    scaled = (damage << 12) / viewer.max_hp;
    if (scaled > 950) {
        scaled = 950;
    }
    reach = scaled >> 2;
    if (from) {
        vector_to_angles(viewer.position.vx - from[0], viewer.position.vy - from[1], viewer.position.vz - from[2], &dir);
        if (reach < 80) {
            reach = 80;
        }
        if (reach > 450) {
            reach = 450;
        }
        vector_from_angles(&dir, &push);
        vector_scale(reach, &push);
        if (push.vy > 200) {
            push.vy = 200;
        } else if (push.vy < -200) {
            push.vy = -200;
        }
    } else {
        push.vx = push.vy = push.vz = 0;
    }
    time = (1300 - (scaled >> 1)) >> 2;
    if (time < 71) {
        time = 70;
    }
    if (viewer.impact_resistance) {
        viewer.shake_timer = (damage * 5000 / viewer.max_hp + 500) / viewer.impact_resistance;
    } else {
        viewer.shake_timer = 30;
    }
    if (viewer.shake_timer < 0) {
        viewer.shake_timer = -1;
    }
    if (viewer.shake_timer > 30) {
        viewer.shake_timer = 30;
    }
    pad_vibrate(damage * 255 / viewer.max_hp);
    amount = damage * 255 / viewer.max_hp + 32;
    if (amount > 128) {
        amount = 128;
    }
    random = rand();
    shake_amount = amount - 16384;
    shake_amount += random;
    viewer.hand_rot_z = shake_amount >> 9;
    viewer.hand_rot_x += amount + (rand() >> 9);
    if (hp == 0) {
        player_kill((int)&push);
        viewer.red_fade = 3500;
        viewer.red_fade_step = time;
    } else if (from == NULL) {
        viewer.red_fade = 3500;
        viewer.red_fade_step = time;
    } else {
        switch ((dir.vy - viewer.yaw + 256) >> 9 & 7) {
        case 0:
            dir.vx = -scaled;
            dir.vz = 0;
            break;
        case 1:
            dir.vx = -scaled;
            dir.vz = scaled;
            break;
        case 2:
            dir.vx = 0;
            dir.vz = scaled;
            break;
        case 3:
            dir.vx = scaled;
            dir.vz = scaled;
            break;
        case 4:
            dir.vx = scaled;
            dir.vz = 0;
            break;
        case 5:
            dir.vx = scaled;
            dir.vz = -scaled;
            break;
        case 6:
            dir.vx = 0;
            dir.vz = -scaled;
            break;
        case 7:
            dir.vx = -scaled;
            dir.vz = -scaled;
            break;
        }
        dir.vy = 0;
        player_enter_hurt_state(&push, &dir, time, damage);
    }
}

/*
 * Applies a hit: rolls the status effect against the viewer's resistance,
 * caps the status times, then turns the three physical and six elemental
 * damage values into hit points through the defence values.
 */
ADDRESS(0x8003dbe0, 0xa08)
void player_hit(u16 *damage, u16 scale, u16 effect, long *from)
{
    u16 bonus[6];
    int physical;
    int elemental;
    u16 *defence;
    u16 *extra;
    int resist;
    int base;
    int limit300;
    int limit75;
    int limit600;
    int limit0;
    u16 attack;
    u16 magic;
    ItemType *shield;
    int k;
    int d;
    int v;
    int i;

    elemental = physical = 0;
    defence = viewer.element_defence;
    memset(bonus, 0, sizeof(bonus));
    extra = bonus;
    if (effect != 0) {
        switch (effect) {
        case 8:
        case 16:
        case 24:
        case 32:
        case 64:
        case 96:
        case 128:
        case 256:
            resist = (u16)(viewer.physical_base / 20) + viewer.attributes[13] * 3 / 80 + viewer.element_defence[5] * 60 / 255;
            if (101 - resist < rand() * 100 / 32768) {
                goto limits;
            }
            break;
        case 7:
            resist = (u16)(viewer.physical_base / 20) + viewer.attributes[12] * 3 / 80 + viewer.element_defence[2] * 60 / 255;
            if (101 - resist < rand() * 100 / 32768) {
                goto limits;
            }
            break;
        }
        switch (effect) {
        case 128:
            if (!(viewer.status_flags & 0x80)) {
                viewer.unk190[3][1] = 300;
                viewer.unk190[3][0] = 0;
                viewer.status_flags |= 0x80;
                player_compute_derived_stats(0);
                PLAYER_CLAMP_HP();
            } else {
                viewer.unk190[3][0] = 20;
                viewer.unk190[3][1] = 300;
            }
            break;
        case 8:
        case 16:
        case 24:
            if (!(viewer.status_flags & 0x18)) {
                viewer.unk190[1][1] = 75;
                viewer.unk190[1][0] = 0;
                viewer.status_flags |= 8;
            } else if (viewer.status_flags & 8) {
                viewer.unk190[1][1] = 75;
                viewer.unk190[1][0] = 0;
                viewer.status_flags |= 0x10;
            } else {
                viewer.unk190[1][0] = 20;
                viewer.unk190[1][1] = 75;
            }
            break;
        case 32:
        case 64:
        case 96:
            if (!(viewer.status_flags & 0x60)) {
                viewer.unk190[2][1] = 600;
                viewer.unk190[2][0] = 0;
                viewer.status_flags |= 0x60;
            } else {
                viewer.unk190[2][0] = 20;
                viewer.unk190[2][1] = 600;
            }
            break;
        case 512:
            player_add_hp(-(rand() / 256));
            sound_play(9, 128);
            break;
        case 256:
            if (!(viewer.status_flags & 0x100)) {
                viewer.unk190[4][1] = 6000;
                viewer.unk190[4][0] = 0;
                viewer.status_flags |= 0x100;
                player_compute_derived_stats(0);
                PLAYER_CLAMP_HP();
            } else {
                viewer.unk190[4][0] = 20;
                viewer.unk190[4][1] = 6000;
            }
            break;
        case 1:
        case 2:
        case 4:
        case 7:
            if (!(viewer.status_flags & 0x100000)) {
                if (viewer.status_flags & 7) {
                    if (viewer.unk190[0][0] >= 220 && viewer.unk190[0][1] != 0) {
                        viewer.unk190[0][0] = 20;
                        viewer.unk190[0][1] = 600;
                        if (viewer.status_flags & 2) {
                            viewer.status_flags |= 4;
                        }
                        if (viewer.status_flags & 1) {
                            viewer.status_flags |= 2;
                        }
                    }
                } else {
                    viewer.unk190[0][1] = 600;
                    viewer.unk190[0][0] = 0;
                    viewer.status_flags |= 1;
                }
            }
            break;
        case 1024:
            viewer.status_flags |= 0x400;
            if (viewer.frame_count % 40 == 0) {
                sound_play(7, 128);
            }
            viewer.yellow_fade = 3500;
            viewer.yellow_fade_step = 128;
            break;
        case 16384:
            viewer.shield_raised = 0;
            viewer.shield_lowering = 1;
            break;
        }
    limits:
        limit300 = 300;
        limit75 = 75;
        limit600 = 600;
        base = (112 - viewer.status_recovery) * 50 / 59 + 50;
        limit300 = base * limit300 / 100;
        limit75 = base * limit75 / 100;
        limit600 = base * limit600 / 100;
        limit300 = limit300 * viewer.status_time_scale / 100;
        limit75 = limit75 * viewer.status_time_scale / 100;
        limit600 = limit600 * viewer.status_time_scale / 100;
        limit0 = limit600;
        if (limit300 < viewer.unk190[3][1]) {
            viewer.unk190[3][1] = limit300;
        }
        if (limit75 < viewer.unk190[1][1]) {
            viewer.unk190[1][1] = limit75;
        }
        if (limit600 < viewer.unk190[2][1]) {
            viewer.unk190[2][1] = limit600;
        }
        if (limit0 < viewer.unk190[0][1]) {
            viewer.unk190[0][1] = limit0;
        }
    }
    if (viewer.shield_raised != 0 && viewer.equipment[EQUIP_SLOT_LEFT_HAND] != EQUIP_NONE) {
        attack = viewer.armour_defence + viewer.shield_defence;
        shield = &vis_table_c.types[viewer.equipment[EQUIP_SLOT_LEFT_HAND]];
        magic = viewer.magic_defence + viewer.shield_magic_defence;
        if (shield->kind2 >= 9) {
            bonus[shield->kind2 - 9] = shield->amount2 >> 1;
        }
    } else {
        attack = viewer.armour_defence;
        magic = viewer.magic_defence;
    }
    k = attack ? attack : 1;
    for (i = 0; i < 3; i++, damage++) {
        if (*damage) {
            d = *damage;
            physical += d * 32500 / (k * (650 - d));
        }
    }
    for (i = 0; i < 6; i++, damage++, defence++, extra++) {
        if (*damage == 0) {
            continue;
        }
        d = *damage;
        if ((viewer.status_flags & 0x80000) && i == 0) {
            d >>= 2;
        }
        if ((viewer.status_flags & 0x800000) && i == 3) {
            d >>= 2;
        }
        if ((viewer.status_flags & 0x100000) && i == 2) {
            continue;
        }
        k = magic + *defence + *extra;
        if (k == 0) {
            k = 1;
        }
        v = d * 32500 / (k * (650 - d));
        if (i != 2 && v == 0) {
            v = 1;
        }
        elemental += v;
    }
    player_take_damage((physical + elemental) * scale >> 12, effect, from);
}

/* Hits the viewer with `damage` when it is within `range` of `pos`, scaled
 * down with the distance. */
ADDRESS(0x8003e5e8, 0x14c)
void player_hit_in_radius(u16 *damage, u16 effect, long *pos, int min, int range, int kind)
{
    int d;

    if (kind == 0x8000) {
        d = vector_sphere_distance((VECTOR *)pos, range, (VECTOR *)&viewer.position.vx, 256, 1000);
    } else if (kind == 0x8001) {
        if (viewer.position.vy > pos[1]) {
            d = -9999999;
        } else {
            d = vector_sphere_distance((VECTOR *)pos, range, (VECTOR *)&viewer.position.vx, 256, 1000);
        }
    } else {
        d = player_distance_within(pos[0], pos[1], pos[2], range, kind);
        if (d == -1) {
            d = -9999999;
        }
    }
    if (d >= min) {
        player_hit(damage, 4096 - (d << 12) / range, effect, pos);
    }
}

/* The camera position and angles. */
ADDRESS(0x8003e734, 0x60)
void player_get_camera(VECTOR *pos, SVECTOR *rot)
{
    pos->vx = viewer.position.vx;
    pos->vz = viewer.position.vz;
    pos->vy = viewer.position.vy + viewer.bob_offset + viewer.unk2D2 - 1600;
    rot->vx = viewer.pitch;
    rot->vy = viewer.yaw;
    rot->vz = viewer.roll;
}

/* Clears the status timers and restores the hit and magic points. */
ADDRESS(0x8003e794, 0xac)
void player_recover_all(void)
{
    viewer.unk190[0][0] = viewer.unk190[0][1] = viewer.unk190[1][0] = viewer.unk190[1][1] =
        viewer.unk190[2][0] = viewer.unk190[2][1] = viewer.unk190[3][0] = viewer.unk190[3][1] =
            viewer.unk190[4][0] = viewer.unk190[4][1] = viewer.unk190[5][0] =
                viewer.unk190[5][1] = viewer.unk190[6][0] = viewer.unk190[6][1] =
                    viewer.unk190[7][0] = viewer.unk190[7][1] = viewer.unk190[8][0] =
                        viewer.unk190[8][1] = viewer.unk190[9][0] = viewer.unk190[9][1] =
                            viewer.unk190[10][0] = viewer.unk190[10][1] =
                                viewer.unk190[11][0] = viewer.unk190[11][1] =
                                    viewer.unk190[12][0] = viewer.unk190[12][1] = 0;
    viewer.fall_death = 0;
    viewer.hp = viewer.hud_hp = viewer.max_hp;
    viewer.mp = viewer.hud_mp = viewer.max_mp;
    viewer.status_flags = 0;
    player_reset_state();
}

ADDRESS(0x8003e840, 0x34)
void hud_clear_damage_numbers(void)
{
    int i;

    for (i = 0; i < 5; i++) {
        viewer.damage_number_timers[i] = 0;
        viewer.damage_numbers[i] = 0;
    }
}

/* Resets the player for a new game. */
ADDRESS(0x8003e874, 0x1ac)
void player_reset_new_game(void)
{
    player_reset_stats();
    viewer.footstep_timer = 2048;
    viewer.play_seconds = 0;
    viewer.red_fade = viewer.red_fade_step = viewer.yellow_fade = viewer.yellow_fade_step = 0;
    viewer.hud_redraw = 2;
    viewer.quick_item_index = 0;
    viewer.unk1E4 = 0;
    viewer.motion_stopped = 0;
    viewer.shield_wear = viewer.armour_wear = viewer.weapon_wear = 0;
    viewer.magic_defs[0] = viewer.magic_defs[1] = 0;
    player_load_magic(0xffff, 0);
    player_load_magic(0xffff, 1);
    viewer.slot_model_info[0] = viewer.slot_model_info[1] = viewer.slot_model_info[2] = viewer.slot_model_info[3] = 0;
    player_load_equipment_model(0xffff, 0, 0);
    player_load_equipment_model(0xffff, 1, 0);
    player_load_equipment_model(0xffff, 2, 0);
    player_load_equipment_model(0xffff, 3, 0);
    viewer.slot_sounds[0] = 24;
    viewer.slot_sounds[1] = 25;
    viewer.slot_sounds[2] = 26;
    viewer.slot_sounds[3] = 27;
    viewer.equipment[EQUIP_SLOT_RIGHT_HAND] = 0;
    player_load_equipment_model(0, 0, 0);
    item_counts_clear();
    viewer.items_found = 0;
    viewer.play_frames = 0;
    viewer.play_seconds = 0;
    viewer.creatures_killed = 0;
    player_recalc_stats();
    viewer.view_rot.vx = 0;
    viewer.view_rot.vy = 3072;
    viewer.view_rot.vz = 0;
    viewer.position.vx = 0x3d000;
    viewer.position.vy = 0x405c0;
    viewer.position.vz = 0x3f000;
    viewer.push_x = viewer.push_y = viewer.push_z = 0;
    player_recover_all();
    equip_load_item_slot_pictures();
    viewer.save_x = 61;
    viewer.save_y = 64;
    viewer.save_z = 63;
    viewer.save_area = 0;
    viewer.save_dx = 0;
    viewer.save_dy = 1472;
    viewer.save_dz = 0;
    viewer.save_yaw = 3072;
    hud_clear_damage_numbers();
}

ADDRESS(0x8003ea20, 0xb0)
void player_init(void)
{
    viewer.magic_buffers[0] = magic_model_buffers[0];
    viewer.magic_buffers[1] = magic_model_buffers[1];
    viewer.slot_model_files[0] = equipment_model_buffers[0];
    viewer.slot_model_files[1] = equipment_model_buffers[1];
    viewer.slot_model_files[2] = equipment_model_buffers[2];
    viewer.slot_model_files[3] = equipment_model_buffers[3];
    player_reset_new_game();
    game_settings.sfx_volume = 63;
    game_settings.bgm_volume = 63;
    game_settings.show_status = 1;
    game_settings.walk_bob = 1;
    game_settings.subtitles = 1;
    game_settings.vibration = 1;
    game_settings.repeat_interval = 5;
    game_settings.repeat_delay = 30;
    game_settings.combo_window = 20;
    viewer.update_all_spawns = 0;
}

ADDRESS(0x8003ead0, 0x28)
void player_stop_motion(void)
{
    viewer.walk_speed_max = 0;
    viewer.yaw_speed_max = 0;
    viewer.yaw_speed = 0;
    viewer.pitch_speed = 0;
    viewer.walk_speed = 0;
    viewer.motion_stopped = 1;
}

ADDRESS(0x8003eaf8, 0x3c)
void player_reset_motion(void)
{
    viewer.frame_count = 0;
    viewer.update_all_spawns = 1;
    viewer.unk239 = 0;
    viewer.vertical_speed = 0;
    viewer.state = 0;
    player_stop_motion();
}

/* player_distance_within's distance to `pos` (height ignored) when the viewer is
 * within `limit` of facing `angle`, else -1. */
ADDRESS(0x8003eb34, 0xc8)
int player_distance_in_arc(VECTOR *pos, int angle, int range, int limit)
{
    int d;
    short a;

    d = player_distance_within(pos->vx, 0xffff, pos->vz, range, 0);
    if (d != -1) {
        a = (angle_from_xz(viewer.position.vx - pos->vx, viewer.position.vz - pos->vz) - angle) & 0xfff;
        if (a > 2048) {
            a = 4096 - a;
        }
        if (a > limit) {
            d = -1;
        }
    }
    return d;
}

ADDRESS(0x8003ebfc, 0x50)
int player_distance_within(int x, int y, int z, int range, int kind)
{
    return vector_distance_xz_within_rev((VECTOR *)&viewer.position.vx, x, y, z, range, 1000, kind);
}

ADDRESS(0x8003ec4c, 0x10)
void func_8003ec4c(volatile u8 *p)
{
    p[16] = 1;
}

/*
 * Uploads the TIM images in `data` (until a block without the TIM id): each
 * CLUT block at `clut` and each pixel block at `pix`, offset from the
 * TIM's own position unless `absolute`.
 */
ADDRESS(0x8003ec5c, 0x1dc)
void tim_load_images_at(void *data, DVECTOR *pix, DVECTOR *clut, int absolute)
{
    typedef struct {
        u_long byte_count;
        u_long position;
        u_long dimensions;
    } TimBlockWords;
    union {
        TimBlock header;
        TimBlockWords words;
    } block;
    typedef char HeaderWordSize[sizeof(u_long) == 4 ? 1 : -1];
    typedef char HeaderSize[sizeof(TimBlock) == 12 ? 1 : -1];
    typedef char HeaderLengthAt0[offsetof(TimBlock, len) == 0 ? 1 : -1];
    typedef char HeaderRectAt4[offsetof(TimBlock, rect) == 4 ? 1 : -1];
    typedef char HeaderRectSize[sizeof(RECT) == 8 ? 1 : -1];
    typedef char HeaderWordsSize[sizeof(TimBlockWords) == 12 ? 1 : -1];
    typedef char HeaderPositionAt4[offsetof(TimBlockWords, position) == 4 ? 1 : -1];
    typedef char HeaderDimensionsAt8[offsetof(TimBlockWords, dimensions) == 8 ? 1 : -1];
    typedef char HeaderUnionSize[sizeof(block) == 12 ? 1 : -1];
    RECT rect;
    DVECTOR zero_clut;
    DVECTOR zero_pix;
    DVECTOR *cp;
    DVECTOR *pp;
    u_long *p;
    u_long position_word;
    u_long length_word;
    u_long dimensions_word;

    memset(&zero_clut, 0, sizeof(zero_clut));
    memset(&zero_pix, 0, sizeof(zero_pix));
    cp = &zero_clut;
    pp = &zero_pix;
    if (clut) {
        cp = clut;
    }
    p = data;
    if (pix) {
        pp = pix;
    }
next:
    if (*p++ == 0x10) {
        if (*p++ & 8) {
            position_word = p[1];
            length_word = p[0];
            block.words.position = position_word;
            dimensions_word = p[2];
            p += 3;
            block.words.byte_count = length_word;
            block.words.dimensions = dimensions_word;
            if (!absolute) {
                rect.x = block.header.rect.x + cp->vx;
                rect.y = block.header.rect.y + cp->vy;
            } else {
                rect.x = cp->vx;
                rect.y = cp->vy;
            }
            rect.w = block.header.rect.w;
            rect.h = block.header.rect.h;
            LoadImage(&rect, p);
            p = (u_long *)((u8 *)p + (block.header.len - 12));
        }
        position_word = p[1];
        length_word = p[0];
        block.words.position = position_word;
        dimensions_word = p[2];
        p += 3;
        block.words.byte_count = length_word;
        block.words.dimensions = dimensions_word;
        if (!absolute) {
            rect.x = block.header.rect.x + pp->vx;
            rect.y = block.header.rect.y + pp->vy;
        } else {
            rect.x = pp->vx;
            rect.y = pp->vy;
        }
        rect.w = block.header.rect.w;
        rect.h = block.header.rect.h;
        LoadImage(&rect, p);
        p = (u_long *)((u8 *)p + (block.header.len - 12));
        goto next;
    }
}

/* Loads item `id`'s parts and textures into item slot `slot` (0xffff:
 * empty), freeing the slot's old effects. */
ADDRESS(0x8003ee38, 0x2f4)
void player_load_magic(u16 id, u8 slot)
{
    u8 item = 255;
    DVECTOR pix[2] = {{0, 0}, {64, 0}};
    DVECTOR clut[2] = {{0, 0}, {0, 6}};
    StEffect *rec;
    int i;
    u8 *p;
    u16 n;
    u16 part;
    u_long size;

    if (id != 0xffff) {
        item = id;
    }
    if (viewer.magic_ids[slot] != 255) {
        rec = effects.slots;
        i = 128;
        while (i--) {
            int side = rec->flags;

            if (side != 255) {
                side &= 0x20;
                if ((side != 0) == slot) {
                    if (rec->kind < 44) {
                        rec->flags = 255;
                    }
                    if (rec->kind >= 170 && rec->kind < 180) {
                        rec->flags = 255;
                    }
                }
            }
            rec++;
        }
    }
    viewer.magic_ids[slot] = item;
    if (item != 255) {
        viewer.magic_defs[slot] = &effects.defs[item];
        cd_read_member(1, item * 4 + 740, (u_long *)viewer.magic_buffers[slot]);
        p = viewer.magic_buffers[slot];
        n = *(u16 *)p;
        p += 4;
        part = slot * 8 + 96;
        while (n--) {
            size = *(u_long *)p;
            if (size >= 5) {
                world_state.model_files[part] = p;
                model_offset_textures((Model *)(p + ((u_long *)p)[2]), &pix[slot], &clut[slot], 0);
            }
            p += size;
            part++;
        }
        cd_read_member(1, item * 4 + 741, cd_file_buffer);
        tim_load_images_at(cd_file_buffer, &pix[slot], &clut[slot], 0);
        DrawSync(0);
        sound_bank_load(1, item * 4 + 742, slot + 6);
    }
    viewer.magic_gauge_delay = 30;
    viewer.magic_gauge = 0;
    viewer.cast_gauge = 0;
    viewer.cast_request[slot] = -1;
    viewer.unk128[slot] = 0;
    viewer.shot_magic[slot] = 255;
    cd_wait_idle();
}

/* Loads equipment `id`'s model and textures into model slot `slot` (0xffff:
 * empty); a two-handed model (31..34) also fills the paired slot. */
ADDRESS(0x8003f12c, 0x3c0)
void player_load_equipment_model(u16 id, u8 slot, u8 reload)
{
    DVECTOR pix[4] = {{0, 0}, {64, 0}, {0, 128}, {64, 128}};
    DVECTOR clut[4] = {{0, 0}, {0, 2}, {0, 4}, {0, 6}};
    ScratchPad *sp;
    ItemType *item;
    u8 model;
    u8 cur;
    u8 paired;
    u8 other;
    u8 pair;

    paired = 0;
    other = 0;
    sp = SCRATCHPAD;
    if (id != 0xffff) {
        item = &vis_table_c.types[id];
        model = item->model;
    } else {
        model = 255;
    }
    viewer.weapon_gauge_delay = 15;
    viewer.weapon_gauge = 0;
    viewer.swing_gauge = 0;
    if (model != 255 && (cur = viewer.slot_model_ids[slot]) == model && reload) {
        cd_read_member(1, cur * 4 + 485, cd_file_buffer);
        tim_load_images_at(cd_file_buffer, &pix[slot], &clut[slot], 0);
        DrawSync(0);
    }
    viewer.slot_model_ids[slot] = model;
    if (model >= 31 && model <= 34) {
        paired = 1;
        other = model + 29;
    }
    if (model != 255) {
        viewer.slot_model_info[slot] = &equipment_model_placements[model];
        if (paired) {
            pair = slot ^ 1;
            viewer.slot_model_info[pair] = &equipment_model_placements[other];
            cd_read_member(1, model * 4 + 484, viewer.slot_model_files[slot]);
            cd_read_member(1, other * 4 + 484, viewer.slot_model_files[pair]);
            anim_register(slot + 32, viewer.slot_model_files[slot]);
            model_offset_textures((Model *)sp->model, &pix[slot], &clut[slot], 0);
            anim_register(pair + 32, viewer.slot_model_files[pair]);
        } else {
            cd_read_member(1, model * 4 + 484, viewer.slot_model_files[slot]);
            anim_register(slot + 32, viewer.slot_model_files[slot]);
            model_offset_textures((Model *)sp->model, &pix[slot], &clut[slot], 1);
        }
        cd_read_member(1, model * 4 + 485, cd_file_buffer);
        tim_load_images_at(cd_file_buffer, &pix[slot], &clut[slot], 0);
        DrawSync(0);
        sound_bank_load(1, model * 4 + 486, viewer.slot_sounds[slot] - 22);
    }
    if (slot < 2) {
        viewer.action_timers.halves[slot] = -1;
    }
    viewer.slot_anims[slot] = 0;
    viewer.action_recoil = 0;
    viewer.shield_lowering = 0;
    viewer.hand_rot_x = 0;
    viewer.hand_rot_y = 0;
    viewer.hand_rot_z = 0;
    viewer.shield_raised = 0;
    cd_wait_idle();
}

/* Starts model slot `slot`'s action animation when it is idle. */
ADDRESS(0x8003f4ec, 0x184)
void player_start_hand_action(u8 value, u8 slot)
{
    if (slot == 0) {
        if (viewer.action_timers.halves[0] != -1) {
            return;
        }
        if (viewer.action_timers.halves[1] >= 2048 && !viewer.shield_lowering) {
            return;
        }
        if (viewer.slot_model_ids[0] == 255) {
            return;
        }
        if (viewer.unk190[1][0] != 0) {
            return;
        }
        viewer.slot_anim_ids[0] = value;
        viewer.action_timers.halves[0] = 0;
        viewer.action_started = 1;
        pad_state.pressed &= ~pad_state.buttons[8];
        viewer.action_timers.halves[2] = viewer.slot_model_info[0]->hit_time;
        viewer.action_sound_time[0] = viewer.slot_model_info[0]->sound_time;
        viewer.weapon_gauge_delay = 15;
    } else {
        if (viewer.action_timers.halves[slot] != -1 || viewer.action_timers.halves[0] != -1) {
            return;
        }
        if (viewer.cast_request[slot] != -1 || viewer.cast_request[0] != -1) {
            return;
        }
        if (viewer.slot_model_ids[slot] == 255) {
            return;
        }
        if (viewer.unk190[1][0] != 0) {
            return;
        }
        pad_state.pressed &= ~pad_state.buttons[9];
        viewer.slot_anim_ids[slot] = value;
        viewer.action_timers.halves[slot] = 0;
        viewer.action_started = 1;
        viewer.action_timers.halves[slot + 2] = viewer.slot_model_info[slot]->hit_time;
        viewer.action_sound_time[slot] = viewer.slot_model_info[slot]->sound_time;
    }
}

/* Starts item slot `slot`'s use when both model slots are idle. */
ADDRESS(0x8003f670, 0x94)
void player_start_cast(u8 unused, u8 slot)
{
    Viewer *v;

    v = &viewer;
    if (*(s32 *)v->cast_request == -1 && v->action_timers.word == -1 && v->magic_ids[slot] != 255 &&
        !(v->status_flags & 0x10) && (!v->blocks_magic || v->unk190[12][0] != 0)) {
        v->cast_request[slot] = 0;
        v->cast_gauge = v->magic_gauge;
        v->magic_gauge = 0;
        v->magic_gauge_delay = 30;
    }
}

/*
 * Places an equipment effect: `pos` gets slot `slot`'s offset turned by the
 * viewer's matrix, and `dir` a `length`-long direction along `rot` (or the
 * viewer's facing when `rot` is null).
 */
ADDRESS(0x8003f704, 0x20c)
int player_get_launch_vector(int length, VECTOR *pos, SVECTOR *dir, SVECTOR *rot, int slot)
{
    if (pos) {
        gte_SetRotMatrix(&viewer.camera_matrix);
        gte_ldv0(&viewer.shot_offsets[slot]);
        gte_rtv0();
        gte_stlvnl(pos);
        pos->vx += viewer.position.vx;
        pos->vy += viewer.position.vy;
        pos->vz += viewer.position.vz;
    }
    if (rot) {
        if (dir) {
            MATRIX m;
            SVECTOR v;

            memset(&v, 0, sizeof(v));
            v.vz = 4096;
            RotMatrixZYX(rot, &m);
            gte_SetRotMatrix(&m);
            gte_ldv0(&v);
            gte_rtv0();
            gte_stsv(dir);
            vector_scale(length, dir);
        }
    } else if (dir) {
        SVECTOR v;

        memset(&v, 0, sizeof(v));
        v.vz = 4096;
        gte_SetRotMatrix(&viewer.camera_matrix);
        gte_ldv0(&v);
        gte_rtv0();
        gte_stsv(dir);
        vector_scale(length, dir);
    }
    return 0;
}

/* The scratchpad model matrix (rotation and translation). */
#define PLAYER_MATRIX(sp) ((MATRIX *)&(sp)->part_pos)
/* Fires equipment slot `slot`'s effect of kind `kind`. va_start names
 * `kind`, so kind lives in its home slot: retail reads its low byte back
 * (lbu 128(sp)) for each effect request. */
ADDRESS(0x8003f910, 0x714)
void player_fire_magic_shot(int kind, int slot, ...)
{
    VECTOR pos;
    VECTOR pos2;
    SVECTOR rot;
    SVECTOR dir;
    SVECTOR aim;
    ScratchPad *sp;
    int count;
    int speed;
    int length;
    int turn;
    int i;
    int side;
    va_list ap;

    va_start(ap, kind);
    side = slot << 5;
    sp = SCRATCHPAD;
    count = viewer.shots_left[slot] - 1;
    switch (kind) {
    case 0:
        speed = 4096;
        goto shoot;
    case 10:
        speed = 2048;
    shoot:
        rot.vx = viewer.camera_rot.vx;
        rot.vy = viewer.camera_rot.vy;
        rot.vz = viewer.camera_rot.vz;
        player_get_launch_vector(700, &pos, &dir, &rot, slot);
        effect_spawn(10, side | 0x42, kind, &pos, &dir, speed, &viewer.pitch);
        break;
    case 7:
        i = 2;
        while (i--) {
            length = i * 32 - 16;
            aim.vx = viewer.camera_rot.vx - 512;
            aim.vy = viewer.camera_rot.vy + length;
            aim.vz = viewer.camera_rot.vz;
            player_get_launch_vector(700, &pos, &dir, &aim, slot);
            effect_spawn(10, side | 0x42, kind, &pos, &dir, 1024, &viewer.pitch);
            viewer.shot_offsets[slot].vx += 1024;
        }
        break;
    case 1:
        length = 256;
        turn = 0;
        speed = 1024;
        goto spray;
    case 3:
        length = 384;
        turn = count * 32 - 80;
        speed = 512;
        goto spray;
    case 9:
        length = 256;
        turn = 128 - rand() / 128;
        speed = 768;
        goto spray;
    case 6:
        length = 512;
        turn = 0;
        speed = 3072;
        goto spray;
    case 4:
        length = 384;
        turn = 0;
        speed = 8192;
        goto spray;
    case 12:
        length = 384;
        turn = 0;
        speed = 2048;
        goto spray;
    case 13:
        length = 320;
        turn = 0;
        speed = 8192;
        goto spray;
    case 15:
        length = 768;
        turn = 0;
        speed = 1024;
        goto spray;
    case 16:
        length = 320;
        turn = 0;
        speed = 1024;
    spray:
        rot.vx = viewer.camera_rot.vx;
        rot.vy = viewer.camera_rot.vy + turn;
        rot.vz = viewer.camera_rot.vz;
        player_get_launch_vector(length, &pos, &dir, &rot, slot);
        rot.vx = viewer.camera_rot.vx;
        rot.vy = -viewer.camera_rot.vy;
        rot.vz = -viewer.camera_rot.vz;
        effect_spawn(10, side | 0x42, kind, &pos, &dir, speed, &rot, count);
        break;
    case 44:
    case 45:
    case 46:
    case 47:
        anim_get_vertex(32, viewer.slot_anim_ids[0], viewer.slot_model_info[0]->hit_time, viewer.slot_model_info[0]->shot_vertex, &dir);
        sp->model_t[0] = viewer.slot_model_info[slot]->pos_x;
        sp->model_t[1] = viewer.slot_model_info[slot]->pos_y;
        sp->model_t[2] = viewer.slot_model_info[slot]->pos_z;
        RotMatrix(&viewer.slot_model_info[slot]->rotation, PLAYER_MATRIX(sp));
        gte_SetRotMatrix(PLAYER_MATRIX(sp));
        gte_SetTransMatrix(PLAYER_MATRIX(sp));
        gte_ldv0(&dir);
        gte_rt();
        gte_stlvnl(&pos);
        dir.vx = pos.vx;
        dir.vy = pos.vy;
        dir.vz = pos.vz;
        rot = viewer.camera_rot;
        sp->model_t[0] = viewer.position.vx;
        sp->model_t[1] = viewer.position.vy;
        sp->model_t[2] = viewer.position.vz;
        RotMatrixZYX(&rot, PLAYER_MATRIX(sp));
        gte_SetRotMatrix(PLAYER_MATRIX(sp));
        gte_SetTransMatrix(PLAYER_MATRIX(sp));
        gte_ldv0(&dir);
        gte_rt();
        gte_stlvnl(&pos);
        anim_get_vertex(32, viewer.slot_anim_ids[0], viewer.slot_model_info[0]->hit_time, viewer.slot_model_info[0]->rotation.pad, &dir);
        sp->model_t[0] = viewer.slot_model_info[slot]->pos_x;
        sp->model_t[1] = viewer.slot_model_info[slot]->pos_y;
        sp->model_t[2] = viewer.slot_model_info[slot]->pos_z;
        RotMatrix(&viewer.slot_model_info[slot]->rotation, PLAYER_MATRIX(sp));
        gte_SetRotMatrix(PLAYER_MATRIX(sp));
        gte_SetTransMatrix(PLAYER_MATRIX(sp));
        gte_ldv0(&dir);
        gte_rt();
        gte_stlvnl(&pos2);
        dir.vx = pos2.vx;
        dir.vy = pos2.vy;
        dir.vz = pos2.vz;
        rot = viewer.camera_rot;
        sp->model_t[0] = viewer.position.vx;
        sp->model_t[1] = viewer.position.vy;
        sp->model_t[2] = viewer.position.vz;
        RotMatrixZYX(&rot, PLAYER_MATRIX(sp));
        gte_SetRotMatrix(PLAYER_MATRIX(sp));
        gte_SetTransMatrix(PLAYER_MATRIX(sp));
        gte_ldv0(&dir);
        gte_rt();
        gte_stlvnl(&pos2);
        vector_to_angles_ratan2(pos.vx - pos2.vx, pos.vy - pos2.vy, pos.vz - pos2.vz, &rot);
        vector_from_angles_zyx(&rot, &dir);
        if (kind == 44) {
            vector_scale(viewer.swing_gauge * 768 / 5120, &dir);
        } else {
            vector_scale(viewer.swing_gauge * 1024 / 5120, &dir);
        }
        pos.vx = pos2.vx;
        pos.vy = pos2.vy + viewer.bob_offset + viewer.unk2D2;
        pos.vz = pos2.vz;
        effect_spawn(10, side | 0x42, kind, &pos, &dir, &rot, viewer.swing_gauge);
        break;
    }
}

/* Per hand: starts the queued magic, or recovers the second bar. */
ADDRESS(0x80040024, 0x288)
void player_update_cast(u8 hand)
{
    ItemType *item;
    int active;
    int rate = 36;

    if (viewer.cast_request[hand] != -1) {
        if (hand) {
            if (viewer.equipment[EQUIP_SLOT_LEFT_RING] != EQUIP_NONE) {
                item = &vis_table_c.types[viewer.equipment[EQUIP_SLOT_LEFT_RING]];
                if (item->durability)
                    player_cast_magic(viewer.magic_ids[hand], hand);
            }
        } else {
            if (viewer.equipment[EQUIP_SLOT_RIGHT_RING] != EQUIP_NONE) {
                item = &vis_table_c.types[viewer.equipment[EQUIP_SLOT_RIGHT_RING]];
                if (item->durability)
                    player_cast_magic(viewer.magic_ids[0], 0);
            }
        }
        viewer.cast_request[hand] = -1;
        return;
    }
    active = 0;
    if (viewer.equipment[EQUIP_SLOT_RIGHT_RING] != EQUIP_NONE) {
        item = &vis_table_c.types[viewer.equipment[EQUIP_SLOT_RIGHT_RING]];
        if (item->durability)
            active = viewer.equipment[EQUIP_SLOT_RIGHT_MAGIC] != EQUIP_NONE;
    }
    if (viewer.equipment[EQUIP_SLOT_LEFT_RING] != EQUIP_NONE) {
        item = &vis_table_c.types[viewer.equipment[EQUIP_SLOT_LEFT_RING]];
        if (item->durability && viewer.equipment[EQUIP_SLOT_LEFT_MAGIC] != EQUIP_NONE)
            active = 1;
    }
    if (!viewer.blocks_magic || viewer.unk190[12][0] != 0) {
        if (!active)
            return;
        if (viewer.magic_gauge_delay == 0) {
            viewer.magic_gauge += (viewer.attributes[8] / (viewer.attributes[8] + 1000) * rate + 36) << 1;
            if (viewer.magic_gauge > 5120)
                viewer.magic_gauge = 5120;
        } else {
            viewer.magic_gauge_delay--;
        }
    } else {
        if (viewer.magic_gauge - 128 > 0)
            viewer.magic_gauge -= 128;
        else
            viewer.magic_gauge = 0;
    }
}

/* Steps hand `hand`'s swing or cast for its held item kind: the swing
 * timer viewer.action_timers.halves[hand] against its hit window (action_timers.halves[hand + 2]) and
 * its end (action_sound_time[hand]), the weapon gauge viewer.weapon_gauge, the hit search
 * over vis_table_a's entries and the item wear.
 *
 * The step and the gauge gain are loaded and then scaled in place
 * (`x = base; x += x * skill / n`): retail multiplies by the register
 * that holds 96 instead of shifting, and keeps the raw argument in a0.
 * In case 0..30 retail keeps the step's add ahead of the pad address
 * and the limit's load behind it, where the scheduler would otherwise
 * sink the add into the load gap: a loop note between them (a
 * statement macro's do/while (0)) and the pad pointer taken before the
 * limit reproduce that; the other two cases have neither.
 * The unused locals stand for stack retail reserves and never touches:
 * 24 bytes after sv, 32 after v and 8 after p. */
ADDRESS(0x800402ac, 0xc58)
void player_update_hand_action(u8 hand)
{
    EquipmentModel *rec;
    SVECTOR sv;
    VECTOR unused1;
    SVECTOR unused2;
    VECTOR v;
    MATRIX unused3;
    CollisionRequest req;
    VECTOR p;
    SVECTOR unused4;
    Creature *e;
    Creature *hit;
    int d;
    int i;
    int r;
    int hr;
    u32 best;
    short dmg;
    int reach;
    int inc;
    int kind;
    CreatureType *def;

    rec = viewer.slot_model_info[hand];
    switch (viewer.slot_model_ids[hand]) {
    case 36 ... 54: {
        int cost;
        if (viewer.hand_rot_x != 0) {
            viewer.hand_rot_x >>= 1;
        }
        if (viewer.hand_rot_z != 0) {
            viewer.hand_rot_z >>= 1;
        }
        if (viewer.hand_rot_y != 0) {
            viewer.hand_rot_y -= viewer.hand_rot_y / 20;
        }
        if (viewer.action_timers.halves[hand] == -1) {
            break;
        }
        cost = rec->action_step;
        cost += cost * viewer.attributes[1] / 1600;
        if (viewer.action_timers.halves[hand ^ 1] != -1) {
            viewer.shield_lowering = 1;
        }
        if (viewer.shield_lowering) {
            viewer.action_timers.halves[hand] -= cost;
        } else if ((!(pad_state.held & pad_state.buttons[8]) && hand == 0) ||
                   (!(pad_state.held & pad_state.buttons[9]) && hand != 0)) {
            viewer.action_timers.halves[hand] -= cost;
            viewer.shield_raised = 0;
        } else if (viewer.action_timers.halves[hand + 2] - cost < viewer.action_timers.halves[hand] &&
                   viewer.action_timers.halves[hand] <= viewer.action_timers.halves[hand + 2]) {
            viewer.shield_raised = 1;
            sv.vx = 0;
            sv.vy = 0;
            sv.vz = rec->reach << 1;
            ApplyMatrix(&viewer.camera_matrix, &sv, &v);
            v.vy = v.vy * (3600 - rec->reach) / 1800;
            v.vx /= 4;
            v.vy /= 4;
            v.vz /= 4;
            d = 0xffff;
            i = 4;
            p.vx = viewer.position.vx;
            p.vy = viewer.position.vy;
            p.vz = viewer.position.vz;
            while (i--) {
                p.vx += v.vx;
                p.vy += v.vy;
                p.vz += v.vz;
                req.position = p;
                req.radius = 1024;
                req.height = 1024;
                r = creature_find_in_reach((CollisionState *)&req, 4);
                if (r != -1) {
                    d = viewer.yaw - (short)vis_table_a.entries[r].player_heading;
                    if (d > 2048) {
                        d -= 4096;
                    }
                    if (d < -2048) {
                        d += 4096;
                    }
                }
            }
            if (d != 0xffff) {
                viewer.hand_rot_y += (d - viewer.hand_rot_y) / 4;
            }
        } else if (viewer.action_started) {
            viewer.action_started = 0;
        } else {
            viewer.action_timers.halves[hand] += cost;
        }
        if ((u16)viewer.action_timers.halves[hand] > 4095) {
            viewer.action_timers.halves[hand] = -1;
            viewer.shield_lowering = 0;
            viewer.hand_rot_x = 0;
            viewer.hand_rot_y = 0;
            viewer.hand_rot_z = 0;
        }
        break;
    }
    case 31 ... 34: {
        int cost;
        if (viewer.action_timers.halves[hand] != -1) {
            cost = rec->action_step;
            cost += cost * viewer.attributes[1] / 1600;
            if (viewer.bow_released) {
                if (viewer.action_sound_time[hand] - cost >= viewer.action_timers.halves[hand]) {
                    viewer.action_timers.halves[hand] -= cost;
                } else {
                    viewer.action_timers.halves[hand] += cost;
                    if (viewer.action_timers.halves[hand] > viewer.action_timers.halves[hand + 2] &&
                        viewer.action_timers.halves[hand] <= viewer.action_timers.halves[hand + 2] + cost) {
                        kind = viewer.slot_model_ids[hand] + 13;
                        sound_play((viewer.slot_sounds[hand] - 24) / 2 + 22, 128);
                        viewer.action_timers.halves[hand] -= cost;
                        player_cast_magic(kind, hand);
                        viewer.action_timers.halves[hand] += cost;
                    }
                }
            } else if ((!(pad_state.held & pad_state.buttons[8]) && hand == 0) ||
                       (!(pad_state.held & pad_state.buttons[9]) && hand != 0)) {
                viewer.bow_released = 1;
                viewer.swing_gauge = viewer.weapon_gauge;
                viewer.weapon_gauge = 0;
            } else if (viewer.action_timers.halves[hand + 2] - cost < viewer.action_timers.halves[hand] &&
                       viewer.action_timers.halves[hand] <= viewer.action_timers.halves[hand + 2]) {
                inc = 96;
                inc += viewer.attributes[1] * inc / 800;
                viewer.weapon_gauge += inc * 2;
                if (viewer.weapon_gauge > 5120) {
                    viewer.weapon_gauge = 5120;
                }
            } else if (viewer.action_timers.halves[hand] == 0 && viewer.action_started) {
                viewer.action_started = 0;
                viewer.bow_released = 0;
            } else {
                {
                int gain = 96;
                gain += viewer.attributes[1] * gain / 800;
                viewer.weapon_gauge += gain;
                }
                if (viewer.weapon_gauge > 5120) {
                    viewer.weapon_gauge = 5120;
                }
                viewer.action_timers.halves[hand] += cost;
                if (viewer.action_timers.halves[hand] > viewer.action_sound_time[hand] - cost &&
                    viewer.action_timers.halves[hand] <= viewer.action_sound_time[hand]) {
                    sound_play(viewer.slot_sounds[hand], 128);
                    player_wear_item(1, &viewer.equipment[EQUIP_SLOT_RIGHT_HAND]);
                }
            }
            if ((u16)viewer.action_timers.halves[hand] > 4095) {
                viewer.action_timers.halves[hand] = -1;
                viewer.action_recoil = 0;
                viewer.bow_released = 0;
            }
            viewer.action_timers.halves[hand ^ 1] = viewer.action_timers.halves[hand];
        } else {
            viewer.weapon_gauge_delay = 0;
        }
        break;
    }
    case 0 ... 30: {
        int cost;
        int max;
        PadState *pad;
        if (viewer.action_timers.halves[hand] != -1) {
            do {
                cost = rec->action_step;
                cost += cost * viewer.attributes[1] / 1600;
            } while (0);
            pad = &pad_state;
            max = rec->sound_time;
            if (pad->pressed & pad->buttons[8]) {
                pad->pressed &= ~pad->buttons[8];
            }
            if (viewer.action_recoil && !(viewer.action_timers.halves[hand + 2] - cost < viewer.action_timers.halves[hand])) {
                viewer.action_timers.halves[hand] -= cost;
            } else if (viewer.action_started) {
                viewer.swing_gauge = viewer.weapon_gauge;
                viewer.weapon_gauge = 0;
                viewer.action_started = 0;
            } else {
                viewer.action_timers.halves[hand] += cost;
            }
            if (viewer.action_sound_time[hand] - cost < viewer.action_timers.halves[hand] &&
                viewer.action_timers.halves[hand] <= viewer.action_sound_time[hand]) {
                sound_play(viewer.slot_sounds[hand], 128);
                if (viewer.action_sound_time[hand] >= max) {
                    viewer.action_sound_time[hand] = 5000;
                } else {
                    viewer.action_sound_time[hand] = viewer.action_sound_time[hand];
                }
            }
            if (viewer.slot_model_ids[hand] < 36 || viewer.slot_model_ids[hand] > 254) {
                if (viewer.action_timers.halves[hand + 2] - cost < viewer.action_timers.halves[hand] &&
                    viewer.action_timers.halves[hand] <= viewer.action_timers.halves[hand + 2]) {
                    int side;
                    e = vis_table_a.entries;
                    best = -1;
                    hit = NULL;
                    hr = 0;
                    dmg = 0;
                    reach = viewer.slot_model_info[hand]->reach;
                    for (vis_table_a.current_index = 0; vis_table_a.current_index < 64; vis_table_a.current_index++, e++) {
                        if (e->spawn_chance != 255) {
                            creature_set_current(e);
                            if (e->state == 1 && e->status.bytes[3] != 1 && e->hp != 0 &&
                                (side = creature_test_player_aim(e, reach)) != 0 && e->player_dist_sq < best) {
                                best = e->player_dist_sq;
                                hit = e;
                                hr = side;
                            }
                        }
                    }
                    if (hit != NULL) {
                        dmg = creature_take_damage(hit, hr, viewer.swing_gauge, viewer.weapon_attack, 0);
                        hud_queue_damage_number(dmg);
                    }
                    if (hr != 0 && dmg != -1) {
                        def = &vis_table_a.types[hit->type];
                        if (dmg * 4 < def->max_hp) {
                            viewer.weapon_wear += 3;
                        } else if (dmg * 2 < def->max_hp) {
                            viewer.weapon_wear += 2;
                        } else if (dmg < def->max_hp) {
                            viewer.weapon_wear += 1;
                        }
                        if (viewer.weapon_wear >= 7) {
                            viewer.weapon_wear -= 7;
                            player_wear_item(1, &viewer.equipment[EQUIP_SLOT_RIGHT_HAND]);
                        }
                    }
                }
                if ((u16)viewer.action_timers.halves[hand] > 4095) {
                    viewer.action_timers.halves[hand] = -1;
                    viewer.action_recoil = 0;
                }
            }
        } else {
            viewer.action_recoil = 0;
            if (!(pad_state.held & pad_state.buttons[8])) {
                if (viewer.weapon_gauge_delay == 0) {
                    inc = 96;
                    inc += viewer.attributes[1] / (viewer.attributes[1] + 1000) * inc;
                    if (viewer.equipment[EQUIP_SLOT_LEFT_HAND] != EQUIP_NONE
                        && viewer.action_timers.halves[1] >= 0) {
                        inc >>= 1;
                    }
                    viewer.weapon_gauge += inc * 2;
                    if (viewer.weapon_gauge > 5120) {
                        viewer.weapon_gauge = 5120;
                    }
                } else {
                    viewer.weapon_gauge_delay--;
                }
            }
        }
        break;
    }
    case 255: /* no item: an empty out-of-range label; with it GCC dispatches
               * by the compare tree retail has instead of a jump table */
        break;
    }
}

/* Starts hand `hand` casting magic `magic` if it is idle and the magic
 * points cover the cost: per-magic cast timers and launch offsets, then
 * the hand's item wears unless the magic is 44..47. */
ADDRESS(0x80040f04, 0x24c)
void player_cast_magic(u8 magic, u8 hand)
{
    StMagic *rec;
    int cost;
    SVECTOR unused;

    if (viewer.shot_magic[hand] != 255 || magic == 255) {
        return;
    }
    rec = (StMagic *)&effects.defs[magic];
    cost = rec->mp_cost;
    if (viewer.mp < cost) {
        return;
    }
    viewer.mp -= cost;
    viewer.shot_magic[hand] = magic;
    switch (magic) {
    case 0:
    case 10:
        viewer.shot_countdown[hand] = 1;
        viewer.shots_left[hand] = 1;
        viewer.shot_interval[hand] = 1;
        viewer.shot_offsets[hand].vx = 0;
        viewer.shot_offsets[hand].vy = 50;
        viewer.shot_offsets[hand].vz = 0;
        break;
    case 7:
        viewer.shot_countdown[hand] = 1;
        viewer.shots_left[hand] = 1;
        viewer.shot_interval[hand] = 1;
        viewer.shot_offsets[hand].vx = -512;
        viewer.shot_offsets[hand].vy = -1024;
        viewer.shot_offsets[hand].vz = 0;
        break;
    case 3:
        viewer.shots_left[hand] = 4;
        viewer.shot_interval[hand] = 8;
        viewer.shot_offsets[hand].vx = 0;
        viewer.shot_offsets[hand].vy = 200;
        viewer.shot_offsets[hand].vz = 0;
        goto fire;
    case 9:
        viewer.shots_left[hand] = 3;
        viewer.shot_interval[hand] = 4;
        viewer.shot_offsets[hand].vx = 0;
        viewer.shot_offsets[hand].vy = 200;
        viewer.shot_offsets[hand].vz = 0;
        goto fire;
    case 12:
        viewer.shots_left[hand] = 2;
        viewer.shot_interval[hand] = 4;
        viewer.shot_offsets[hand].vx = 0;
        viewer.shot_offsets[hand].vy = 200;
        viewer.shot_offsets[hand].vz = 0;
        goto fire;
    case 1:
    case 4:
    case 6:
    case 13:
    case 15:
    case 16:
        viewer.shots_left[hand] = 1;
        viewer.shot_interval[hand] = 1;
        viewer.shot_offsets[hand].vx = 0;
        viewer.shot_offsets[hand].vy = 200;
        viewer.shot_offsets[hand].vz = 0;
    fire:
        viewer.shot_countdown[hand] = 1;
        break;
    case 44:
    case 45:
    case 46:
    case 47:
        viewer.shot_countdown[hand] = 1;
        viewer.shots_left[hand] = 1;
        viewer.shot_interval[hand] = 1;
        break;
    }
    viewer.magic_defs[hand] = rec;
    switch (magic) {
    case 44:
    case 45:
    case 46:
    case 47:
        break;
    default:
        if (hand) {
            player_wear_item(1, &viewer.equipment[EQUIP_SLOT_LEFT_RING]);
        } else {
            player_wear_item(1, &viewer.equipment[EQUIP_SLOT_RIGHT_RING]);
        }
    }
}

/* Moves the viewer `speed` units along heading `angle`, sliding along the
 * walls collision_query reports (at most three tries), and keeps the motion
 * in viewer.velocity.
 */
ADDRESS(0x80041150, 0x4dc)
void player_move(int angle, int speed)
{
    VECTOR vel;
    VECTOR d;
    SVECTOR n;
    int hits;
    int tries;
    int slid;
    int slid_a;
    int slid_b;
    int ang;
    int dy;
    int initial_z;
    Viewer *source;

    slid_a = 0;
    slid_b = 0;
    slid = 0;
    tries = 0;
    vel.vx = rcos(angle) * speed / 4096;
    initial_z = rsin(angle) * speed / 4096;
    source = &viewer;
    vis_lists.position = source->position;
    vel.vz = initial_z;
    vis_lists.radius = 256;
    vis_lists.height = 1600;
    vel.vy = source->vertical_speed;
    vis_lists.tests = 1074;
loop:
    viewer.velocity.vz = vel.vz + viewer.push_z;
    viewer.velocity.vx = vel.vx + viewer.push_x;
    viewer.velocity.vy = vel.vy;
    vis_lists.velocity = viewer.velocity;
    vis_lists.heading = ratan2(viewer.velocity.vz, viewer.velocity.vx);
    vis_lists.speed = SquareRoot0(vel.vx * vel.vx + vel.vz * vel.vz);
    vis_lists.floor_push_x = 0;
    vis_lists.floor_push_y = 0;
    vis_lists.floor_push_z = 0;
    hits = collision_query(&vis_lists);
    if (hits == 0) {
    move:
        viewer.position.vx += vel.vx;
        viewer.position.vy += vel.vy;
        viewer.position.vz += vel.vz;
        goto done;
    }
    if (tries++ >= 3) {
        goto done;
    }
    if (hits & 1) {
        if (!(hits & ~1)) {
            goto move;
        }
    }
    if (hits & 2) {
        vel.vx = (1 - (vis_lists.wall_dist - 256)) * vis_lists.wall_normal_x / 4096;
        vel.vz = (1 - (vis_lists.wall_dist - 256)) * vis_lists.wall_normal_z / 4096;
        if (slid_a) {
            goto move;
        }
        slid_a = 1;
        ang = ratan2(vis_lists.wall_normal_z, vis_lists.wall_normal_x);
        vis_lists.hit_normal_heading = (short)ang;
        hits = speed * rcos(angle - (ang + 1024)) / 4096;
        vis_lists.slide_speed = hits;
    } else if (hits & 8) {
        vel.vx = (1 - (vis_lists.low_wall_dist - 256)) * vis_lists.low_wall_normal_x / 4096;
        vel.vz = (1 - (vis_lists.low_wall_dist - 256)) * vis_lists.low_wall_normal_z / 4096;
        if (slid_b) {
            goto move;
        }
        slid_b = 1;
        ang = ratan2(vis_lists.low_wall_normal_z, vis_lists.low_wall_normal_x);
        vis_lists.hit_normal_heading = (short)ang;
        hits = speed * rcos(angle - (ang + 1024)) / 4096;
        vis_lists.slide_speed = hits;
    } else if (hits & 0x50) {
        collision_locate_hit_body((VECTOR *)&viewer.position.vx);
        d.vx = viewer.position.vx - vis_lists.hit_body_position.vx;
        d.vy = 0;
        d.vz = viewer.position.vz - vis_lists.hit_body_position.vz;
        VectorNormalS(&d, &n);
        dy = vis_lists.hit_body_dist - vis_lists.hit_body_radius - 256;
        if (hits & 0x10) {
            dy = vis_lists.hit_body_dist - vis_lists.hit_body_radius - 768;
        }
        dy = 1 - dy;
        vel.vx = dy * n.vx / 4096;
        vel.vz = dy * n.vz / 4096;
        if (slid) {
            goto done;
        }
        slid = 1;
        ang = ratan2(d.vz, d.vx);
        vis_lists.hit_normal_heading = ang;
        hits = speed * rcos(angle - (ang + 1024)) / 4096;
        vis_lists.slide_speed = hits;
    } else {
        goto done;
    }
    if (hits != 0) {
        vel.vx += rcos(ang + 1024) * hits / 4096;
        vel.vz += rsin(ang + 1024) * hits / 4096;
    }
    goto loop;
done:
    viewer.push_x += vis_lists.floor_push_x / 4;
    viewer.push_z += vis_lists.floor_push_z / 4;
    viewer.velocity.vx = vel.vx;
    viewer.velocity.vy = vel.vy;
    viewer.velocity.vz = vel.vz;
}

/* The coasting vertical offset player_update_motion halves each frame. */
DATA(0x8008803c, 0x2, ".sdata")
static s16 vertical_speed_pitch = 0;

/* Coasts the viewer on its push offsets while it is not walking (sliding
 * along walls as player_move does), then steps the vertical speed
 * against the floor, the head bob and the footsteps, and records the
 * visible record the viewer stands in.
 */
ADDRESS(0x8004162c, 0x720)
void player_update_motion(void)
{
    VECTOR vel;
    int hits;
    int push;
    int slid_a;
    int slid_b;
    int ang;
    int speed;
    short heading;
    int d;
    int s;

    memset(&vel, 0, sizeof(vel));
    slid_b = 0;
    slid_a = 0;
    if (viewer.walk_speed == 0 || viewer.motion_stopped != 0) {
        int initial_x = viewer.push_x;
        int initial_z = viewer.push_z;
        vis_lists.position = viewer.position;
        vis_lists.radius = 256;
        vis_lists.height = 1600;
        vis_lists.tests = 1074;
        vel.vx = initial_x;
        vel.vy = viewer.vertical_speed;
        vel.vz = initial_z;
        speed = SquareRoot0(initial_x * initial_x + initial_z * initial_z);
        heading = ratan2(vel.vz, vel.vx);
    loop:
        vis_lists.heading = 9999;
        vis_lists.velocity.vx = vel.vx;
        vis_lists.velocity.vy = vel.vy;
        vis_lists.velocity.vz = vel.vz;
        vis_lists.speed = SquareRoot0(vel.vx * vel.vx + vel.vz * vel.vz);
        vis_lists.floor_push_x = 0;
        vis_lists.floor_push_y = 0;
        vis_lists.floor_push_z = 0;
        hits = collision_query(&vis_lists);
        if (hits == 0) {
        move:
            viewer.position.vx += vel.vx;
            viewer.position.vy += vel.vy;
            viewer.position.vz += vel.vz;
            goto done;
        }
        if (hits & 1) {
            viewer.position.vx += vel.vx;
            viewer.position.vy += vel.vy;
            viewer.position.vz += vel.vz;
        }
        if (hits & 2) {
            vel.vx = (1 - (vis_lists.wall_dist - 256)) * vis_lists.wall_normal_x / 4096;
            vel.vz = (1 - (vis_lists.wall_dist - 256)) * vis_lists.wall_normal_z / 4096;
            if (slid_a) {
                goto move;
            }
            slid_a = 1;
            ang = ratan2(vis_lists.wall_normal_z, vis_lists.wall_normal_x);
            vis_lists.hit_normal_heading = ang;
            push = speed * rcos(heading - (ang + 1024)) / 4096;
            vis_lists.slide_speed = push;
        } else if (hits & 8) {
            vel.vx = (1 - (vis_lists.low_wall_dist - 256)) * vis_lists.low_wall_normal_x / 4096;
            vel.vz = (1 - (vis_lists.low_wall_dist - 256)) * vis_lists.low_wall_normal_z / 4096;
            if (slid_b) {
                goto move;
            }
            slid_b = 1;
            ang = ratan2(vis_lists.low_wall_normal_z, vis_lists.low_wall_normal_x);
            vis_lists.hit_normal_heading = ang;
            push = speed * rcos(heading - (ang + 1024)) / 4096;
            vis_lists.slide_speed = push;
        } else {
            goto done;
        }
        if (push != 0) {
            vel.vx += rcos(ang + 1024) * push / 4096;
            vel.vz += rsin(ang + 1024) * push / 4096;
        }
        goto loop;
    done:
        viewer.push_x += vis_lists.floor_push_x / 4;
        viewer.push_z += vis_lists.floor_push_z / 4;
        viewer.velocity.vx = vel.vx;
        viewer.velocity.vy = vel.vy;
        viewer.velocity.vz = vel.vz;
    }
    player_check_fall_death();
    d = vis_lists.next_floor_dist - 1600;
    if (d < 0) {
        if (viewer.vertical_speed > 0) {
            play_landing_sound(viewer.vertical_speed);
            viewer.vertical_speed >>= 2;
        } else if (viewer.vertical_speed < 0) {
            if (d >= -1024) {
                viewer.walk_speed_max = 0;
            }
            viewer.vertical_speed -= 48;
            if (vis_lists.next_floor_dist - viewer.vertical_speed > 1600) {
                viewer.vertical_speed = vis_lists.next_floor_dist - 1600;
            }
        } else if (d >= -31) {
            viewer.vertical_speed -= d;
        } else {
            if (d >= -1024) {
                viewer.walk_speed_max = 0;
            }
            viewer.vertical_speed -= 16;
        }
    } else if (d > 0) {
        if (viewer.vertical_speed > 0) {
            if (d < 1025) {
                viewer.walk_speed_max = 0;
            }
            if ((vis_lists.hits & ~15) == 0) {
                viewer.vertical_speed += 48;
            }
        } else if (viewer.vertical_speed < 0) {
            viewer.vertical_speed /= 2;
        } else if (d < 32) {
            viewer.vertical_speed += d;
        } else {
            if (d < 1025) {
                viewer.walk_speed_max = 0;
            }
            viewer.vertical_speed += 16;
        }
    } else {
        viewer.unk239 = 0;
        viewer.vertical_speed = 0;
    }
    if (viewer.vertical_speed > 128) {
        vertical_speed_pitch = (viewer.vertical_speed - 128) / 2;
    } else if (viewer.vertical_speed < -128) {
        vertical_speed_pitch = (viewer.vertical_speed + 128) / 2;
    } else {
        vertical_speed_pitch /= 2;
    }
    viewer.fall_pitch = vertical_speed_pitch;
    if (viewer.unk239 == 0) {
        if (game_settings.walk_bob) {
            viewer.bob_phase = (viewer.bob_phase + viewer.walk_speed) & 0xfff;
            s = rsin(viewer.bob_phase) >> 5;
            if (s < 0) {
                s = -s;
            }
            viewer.bob_offset = s - (s >> 2);
        } else {
            viewer.bob_offset = 0;
        }
        if (viewer.unk239 == 0 && viewer.vertical_speed < 256) {
            if (viewer.walk_speed < viewer.walk_speed_max >> 2) {
                viewer.footstep_timer -= viewer.walk_speed * 3;
            } else if (viewer.walk_speed < viewer.walk_speed_max >> 1) {
                viewer.footstep_timer -= viewer.walk_speed * 3 / 2;
            } else {
                viewer.footstep_timer -= viewer.walk_speed;
            }
            if (viewer.footstep_timer < 0) {
                viewer.footstep_timer = 2048;
                play_footstep_sound(viewer.walk_speed);
            }
        }
    }
    player_update_submersion();
    if (vis_lists.hit_block != -1) {
        viewer.prev_block_record = viewer.block_record;
        viewer.block_record = &vis_records[vis_lists.hit_block];
        viewer.block = vis_lists.hit_block;
    }
}

ADDRESS(0x80041d4c, 0x354)
void player_update_look(void)
{
    if (pad_state.held & pad_state.buttons[2]) {
        viewer.yaw_speed += viewer.yaw_speed_max >> 2;
        if (viewer.yaw_speed_max < viewer.yaw_speed)
            viewer.yaw_speed = viewer.yaw_speed_max;
    } else if (pad_state.held & pad_state.buttons[3]) {
        viewer.yaw_speed -= viewer.yaw_speed_max >> 2;
        if (viewer.yaw_speed < -viewer.yaw_speed_max)
            viewer.yaw_speed = -viewer.yaw_speed_max;
    } else if (viewer.yaw_speed > 0) {
        viewer.yaw_speed -= viewer.yaw_speed_max >> 2;
        if (viewer.yaw_speed < 0)
            viewer.yaw_speed = 0;
    } else if (viewer.yaw_speed < 0) {
        viewer.yaw_speed += viewer.yaw_speed_max >> 2;
        if (viewer.yaw_speed > 0)
            viewer.yaw_speed = 0;
    }
    viewer.view_rot.vy = (viewer.view_rot.vy + viewer.yaw_speed) & 0xfff;
    if (pad_state.held & pad_state.buttons[6]) {
        if (pad_state.held & pad_state.buttons[7]) {
            if (viewer.view_rot.vx > 2048) {
                int limit = (4224 - viewer.view_rot.vx) >> 2;

                viewer.pitch_speed += 8;
                if (limit < viewer.pitch_speed) {
                    viewer.pitch_speed = limit;
                    if (limit == 0)
                        viewer.pitch_speed = 4224 - viewer.view_rot.vx;
                }
            } else if (viewer.view_rot.vx < 2048) {
                int limit = -((viewer.view_rot.vx - 128) >> 2);

                viewer.pitch_speed -= 8;
                if (viewer.pitch_speed < limit) {
                    viewer.pitch_speed = limit;
                    if (limit == 0) {
                        s16 offset = viewer.view_rot.vx - 128;

                        viewer.pitch_speed = -offset;
                    }
                }
            }
        } else {
            viewer.pitch_speed -= 3;
            if (viewer.pitch_speed < -32)
                viewer.pitch_speed = -32;
        }
    } else if (pad_state.held & pad_state.buttons[7]) {
        viewer.pitch_speed += 3;
        if (viewer.pitch_speed > 32)
            viewer.pitch_speed = 32;
    } else if (viewer.pitch_speed > 0) {
        viewer.pitch_speed -= 3;
        if (viewer.pitch_speed < 0)
            viewer.pitch_speed = 0;
    } else if (viewer.pitch_speed < 0) {
        viewer.pitch_speed += 3;
        if (viewer.pitch_speed > 0)
            viewer.pitch_speed = 0;
    }
    if (viewer.pitch_speed > 0) {
        viewer.view_rot.vx = (viewer.view_rot.vx + viewer.pitch_speed) & 0xfff;
        if (angle_signed_greater(viewer.view_rot.vx, 700))
            viewer.view_rot.vx = 700;
    } else if (viewer.pitch_speed < 0) {
        viewer.view_rot.vx = (viewer.view_rot.vx + viewer.pitch_speed) & 0xfff;
        if (angle_signed_greater(3396, viewer.view_rot.vx))
            viewer.view_rot.vx = 3396;
    }
}

/* Walks the viewer from the held buttons: accelerates the walk velocity
 * (viewer.walk_velocity) towards viewer.walk_speed_max or lets it decay, turns it by the
 * yaw and moves the viewer along it (player_move); eases the push
 * offsets viewer.push_x..push_z back and adds the area's drift. */
ADDRESS(0x800420a0, 0x580)
void player_update_walk(void)
{
    SVECTOR sv;
    VECTOR v;
    VECTOR sq;
    MATRIX m;
    SVECTOR rot;
    int ang;

    if (pad_state.held & pad_state.buttons[0]) {
        v.vz = viewer.walk_velocity.vz + (viewer.walk_speed_max >> 2);
        if (v.vz <= viewer.walk_speed_max) {
            viewer.walk_velocity.vz = v.vz;
        } else {
            viewer.walk_velocity.vz = viewer.walk_speed_max;
        }
    } else if (pad_state.held & pad_state.buttons[1]) {
        v.vz = viewer.walk_velocity.vz - (viewer.walk_speed_max >> 2);
        if (v.vz >= -viewer.walk_speed_max) {
            viewer.walk_velocity.vz = v.vz;
        } else {
            viewer.walk_velocity.vz = -viewer.walk_speed_max;
        }
    } else if (viewer.walk_velocity.vz > 0) {
        viewer.walk_velocity.vz -= viewer.walk_speed_max >> 3;
        if (viewer.walk_velocity.vz < 0) {
            viewer.walk_velocity.vz = 0;
        }
    } else if (viewer.walk_velocity.vz < 0) {
        viewer.walk_velocity.vz += viewer.walk_speed_max >> 3;
        if (viewer.walk_velocity.vz > 0) {
            viewer.walk_velocity.vz = 0;
        }
    }
    if ((pad_state.held & pad_state.buttons[5]) && !(pad_state.held & pad_state.buttons[4])) {
        v.vx = viewer.walk_velocity.vx + (viewer.walk_speed_max >> 2);
        if (v.vx <= viewer.walk_speed_max) {
            viewer.walk_velocity.vx = v.vx;
        } else {
            viewer.walk_velocity.vx = viewer.walk_speed_max;
        }
    } else if ((pad_state.held & pad_state.buttons[4]) && !(pad_state.held & pad_state.buttons[5])) {
        v.vx = viewer.walk_velocity.vx - (viewer.walk_speed_max >> 2);
        if (v.vx >= -viewer.walk_speed_max) {
            viewer.walk_velocity.vx = v.vx;
        } else {
            viewer.walk_velocity.vx = -viewer.walk_speed_max;
        }
    } else if (viewer.walk_velocity.vx > 0) {
        viewer.walk_velocity.vx -= viewer.walk_speed_max >> 3;
        if (viewer.walk_velocity.vx < 0) {
            viewer.walk_velocity.vx = 0;
        }
    } else if (viewer.walk_velocity.vx < 0) {
        viewer.walk_velocity.vx += viewer.walk_speed_max >> 3;
        if (viewer.walk_velocity.vx > 0) {
            viewer.walk_velocity.vx = 0;
        }
    }
    gte_ldlvl(&viewer.walk_velocity);
    gte_sqr0();
    gte_stlvnl(&sq);
    viewer.walk_speed = SquareRoot0(sq.vx + sq.vz);
    if (viewer.walk_speed == 0) {
        v.vz = 0;
        v.vx = 0;
    } else {
        if (viewer.walk_velocity.vx < 0) {
            sq.vx = -sq.vx;
        }
        if (viewer.walk_velocity.vz < 0) {
            sq.vz = -sq.vz;
        }
        v.vx = sq.vx / viewer.walk_speed;
        v.vz = sq.vz / viewer.walk_speed;
    }
    rot.vx = 0;
    rot.vz = 0;
    rot.vy = -viewer.view_rot.vy;
    RotMatrix(&rot, &m);
    sv.vx = v.vx;
    sv.vy = v.vy;
    sv.vz = v.vz;
    gte_SetRotMatrix(&m);
    gte_ldv0(&sv);
    gte_rtv0();
    gte_stlvnl(&v);
    ang = ratan2(v.vz, v.vx);
    gte_ldlvl(&v);
    gte_sqr0();
    gte_stlvnl(&sq);
    viewer.walk_speed = SquareRoot0(sq.vx + sq.vz);
    viewer.push_x -= viewer.push_x / 4;
    viewer.push_y -= viewer.push_y / 4;
    viewer.push_z -= viewer.push_z / 4;
    if (viewer.push_x > -4 && viewer.push_x < 4) {
        viewer.push_x = 0;
    }
    if (viewer.push_y > -4 && viewer.push_y < 4) {
        viewer.push_y = 0;
    }
    if (viewer.push_z > -4 && viewer.push_z < 4) {
        viewer.push_z = 0;
    }
    viewer.push_x += vis_table_b.push_x / 4;
    viewer.push_z += vis_table_b.push_z / 4;
    viewer.push_x += vis_table_a.push_x / 2;
    viewer.push_z += vis_table_a.push_z / 2;
    if (viewer.walk_speed != 0 && viewer.motion_stopped == 0) {
        player_move(ang, viewer.walk_speed);
    }
}

/* Swaps hand `a`'s item with slot `b`, twice for a two-handed item. */
ADDRESS(0x80042620, 0x2e0)
void player_swap_hand_items(u8 a, u8 b)
{
    int n = 1;
    SVECTOR unused0 = {0, 128, 0, -128};
    SVECTOR unused1 = {0, 4, 0, -4};
    u16 id;
    EquipmentModel *model_a;
    StAnimFile *block_a;
    u8 model_id_a;
    u8 sound_a;

    if ((viewer.equipment[EQUIP_SLOT_RIGHT_HAND] != EQUIP_NONE
        && vis_table_c.types[viewer.equipment[EQUIP_SLOT_RIGHT_HAND]].category == 9) ||
        (viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE] != EQUIP_NONE
            && vis_table_c.types[viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE]].category == 9))
        n = 2;
    do {
        if (a == 0) {
            id = viewer.equipment[EQUIP_SLOT_RIGHT_HAND];
            viewer.equipment[EQUIP_SLOT_RIGHT_HAND] = viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE];
            viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE] = id;
        } else {
            id = viewer.equipment[EQUIP_SLOT_LEFT_HAND];
            viewer.equipment[EQUIP_SLOT_LEFT_HAND] = viewer.equipment[EQUIP_SLOT_LEFT_RESERVE];
            viewer.equipment[EQUIP_SLOT_LEFT_RESERVE] = id;
        }
        model_a = viewer.slot_model_info[a];
        viewer.slot_model_info[a] = viewer.slot_model_info[b];
        viewer.slot_model_info[b] = model_a;
        block_a = viewer.slot_model_files[a];
        viewer.slot_model_files[a] = viewer.slot_model_files[b];
        viewer.slot_model_files[b] = block_a;
        model_id_a = viewer.slot_model_ids[a];
        viewer.slot_model_ids[a] = viewer.slot_model_ids[b];
        viewer.slot_model_ids[b] = model_id_a;
        if (viewer.slot_model_ids[a] != 0xff)
            anim_register(a + 32, viewer.slot_model_files[a]);
        if (viewer.slot_model_ids[b] != 0xff)
            anim_register(b + 32, viewer.slot_model_files[b]);
        if (viewer.slot_anims[a])
            ((StAnimSlot *)viewer.slot_anims[a])->model_id = 0xffff;
        if (viewer.slot_anims[b])
            ((StAnimSlot *)viewer.slot_anims[b])->model_id = 0xffff;
        sound_a = viewer.slot_sounds[a];
        viewer.slot_sounds[a] = viewer.slot_sounds[b];
        viewer.slot_sounds[b] = sound_a;
        if (a == 0) {
            viewer.weapon_gauge = 0;
            a = 1;
            b = 3;
        } else {
            a = 0;
            b = 2;
        }
    } while (--n);
    if (viewer.slot_model_ids[0] >= 31 && viewer.slot_model_ids[0] < 35)
        anim_register(33, viewer.slot_model_files[1]);
    if (viewer.slot_model_ids[2] >= 31 && viewer.slot_model_ids[2] < 35)
        anim_register(35, viewer.slot_model_files[3]);
    viewer.slot_anims[0] = 0;
    viewer.slot_anims[1] = 0;
    viewer.slot_anims[2] = 0;
    viewer.slot_anims[3] = 0;
    player_recalc_stats();
    viewer.hud_hp = 0xffff;
}

/*
 * Consumes the frame's item request, then steps the queued magic.
 * The paired timer word must be -1 before either equipment request runs.
 */
ADDRESS(0x80042900, 0x29c)
void player_update_hands(void)
{
    if (pad_state.combo == 2) {
        pad_state.combo = 0;
        if (viewer.action_timers.word == -1 && viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE] != EQUIP_NONE) {
            player_swap_hand_items(0, 2);
            player_update_durability_warning();
        }
    }
    if (pad_state.combo == 3) {
        pad_state.combo = 0;
        if (viewer.action_timers.word == -1 && viewer.equipment[EQUIP_SLOT_LEFT_RESERVE] != EQUIP_NONE) {
            player_swap_hand_items(1, 3);
            player_update_durability_warning();
        }
    }
    if (pad_state.combo == 4) {
        pad_state.combo = 0;
        if (viewer.magic_gauge >= 0x1400)
            player_start_cast(0, 0);
    }
    if (pad_state.combo == 5) {
        pad_state.combo = 0;
        if (viewer.magic_gauge >= 0x1400)
            player_start_cast(0, 1);
    }
    if (viewer.shot_magic[0] != 0xff && --viewer.shot_countdown[0] == 0) {
        player_fire_magic_shot(viewer.shot_magic[0], 0);
        if (--viewer.shots_left[0] == 0)
            viewer.shot_magic[0] = 0xff;
        else
            viewer.shot_countdown[0] = viewer.shot_interval[0];
    }
    if (viewer.shot_magic[1] != 0xff && --viewer.shot_countdown[1] == 0) {
        player_fire_magic_shot(viewer.shot_magic[1], 1);
        if (--viewer.shots_left[1] == 0)
            viewer.shot_magic[1] = 0xff;
        else
            viewer.shot_countdown[1] = viewer.shot_interval[1];
    }
    if (viewer.state != 1) {
        if (viewer.weapon_gauge_delay == 0 && (pad_state.pressed & pad_state.buttons[8]))
            player_start_hand_action(0, 0);
        if (pad_state.pressed & pad_state.buttons[9])
            player_start_hand_action(0, 1);
    }
}

ADDRESS(0x80042b9c, 0x74)
void player_handle_examine(void)
{
    u8 mode = viewer.state;

    if (mode != 4 && (pad_state.pressed & pad_state.buttons[10]) && !pad_state.combo_timer) {
        pad_state.pressed &= ~pad_state.buttons[10];
        if (mode == 0)
            player_examine(&viewer.position.vx, &viewer.view_rot);
    }
}

ADDRESS(0x80042c10, 0x34)
void player_enter_normal_state(void)
{
    viewer.state = 0;
    viewer.rot_offset.halves[0] = viewer.rot_offset.halves[1] = viewer.roll_offset = 0;
    viewer.shake_pitch = viewer.shake_yaw = viewer.shake_roll = 0;
    viewer.fall_pitch = viewer.unk292 = viewer.unk294 = 0;
}

DATA(0x80088050, 0x4, ".sdata")
static s32 guard_sound_frame = 0;

ADDRESS(0x80042c44, 0xa8)
void player_enter_hurt_state(SVECTOR *arg0, SVECTOR *arg1, s16 arg2, int arg3)
{
    viewer.state = 16;
    viewer.red_fade = 3500;
    viewer.red_fade_step = arg2;
    if (viewer.max_hp < arg3 * 10)
        viewer.action_recoil = 1;
    if (viewer.equipment[EQUIP_SLOT_LEFT_HAND] != EQUIP_NONE && viewer.shield_raised
        && guard_sound_frame != viewer.frame_count) {
        sound_play(viewer.slot_sounds[1], 128);
        guard_sound_frame = viewer.frame_count;
    }
}

ADDRESS(0x80042cec, 0x10)
void player_enter_frozen_state(void)
{
    viewer.state = 2;
}

/* Frees the animation slots and objects with two state bytes held. */
ADDRESS(0x80042cfc, 0x78)
void anim_free_all_for_menu(void)
{
    u8 *flags = (u8 *)&game_settings;
    u8 a = flags[3];
    u8 b = flags[5];

    flags[3] = 0;
    world_render_frame(0, 0);
    anim_slots_free_idle();
    anim_sets_free_idle();
    anim_slots_free_all();
    anim_sets_free_all();
    flags[3] = a;
    flags[5] = b;
}

/* The per-frame player update: the status timers, opening the pause menu
 * and the map, the walk and stamina limits, the state machine of
 * viewer.state (walking, using an item, dying, falling into a pit), the
 * floor damage and the status damage over time, the view angles, the
 * swings and the hit/magic point recovery. */
ADDRESS(0x80042d74, 0x1678)
void player_update(void)
{
    RECT r;
    StAnimFile *file;
    StEffect *fx;
    int n;
    int i;
    int k;
    int d;

    player_update_status(1);
    if (viewer.state == 0 || viewer.state == 4 || viewer.state == 16 || viewer.state == 1) {
        if (((pad_state.pressed & 1) || pad_state.combo == 1) && viewer.action_timers.word == -1 &&
            viewer.state != 1 && viewer.state != 4 && !(viewer.status_flags & 0x10) &&
            area_state.loading == 0 && vis_table_a.object_sequence_state == 0 && area_state.entry_fade_start == -1) {
            AreaObject *entry;
            int left;

            i = 16;
            lighting.paused = 1;
            play_confirm_sound();
            anim_free_all_for_menu();
            pad_clear_input();
            pad_vibrate(0);
            entry = vis_table_b.entries;
            left = 240;
            while (left--) {
                if (entry->kind == 236 && entry->unk54 != -1) {
                    sound_stop(entry->unk54, entry->params.snd.sound + 1108);
                    entry->unk54 = -1;
                }
                entry++;
            }
            viewer.free_anims_after_menu = 0;
            viewer.save_warp_request = 0;
            viewer.card_screen_done = 0;
            menu_main_run();
            if (game_settings.show_status == 0) {
                hud_clear_damage_numbers();
            }
            if (viewer.card_screen_done) {
                hud_clear_damage_numbers();
                pad_clear_input();
                pad_state.disabled = 1;
                fade_reset();
                viewer.red_fade = viewer.red_fade_step = viewer.yellow_fade = viewer.yellow_fade_step = 0;
                fade_run_frames(2, 4096, 4096, 0);
                fade_run_frames(2, 4096, 4096, 0);
                cd_wait_idle();
                player_reload_equipment();
                vis_table_a.zone = 254;
                vis_table_a.zone_history[0] = vis_table_a.zone_history[1] = vis_table_a.zone_history[2] =
                    vis_table_a.zone_history[3] = 255;
                pad_vibrate(0);
                fx = effects.slots;
                k = 128;
                while (k--) {
                    fx->flags = 255;
                    fx++;
                }
                area_load_set_blocking(area_state.loaded_area, area_state.loaded_area, area_state.loaded_area, area_state.loaded_sound_bank,
                              area_state.unk8, 255);
                cd_wait_idle();
                player_stop_motion();
            } else {
                if (viewer.free_anims_after_menu) {
                    while (i--) {
                        anim_get_file(i + 128, &file);
                        if (file) {
                            heap_free(file);
                        }
                    }
                    i = 128;
                    while (i--) {
                        anim_get_file(i + 588, &file);
                        if (file) {
                            heap_free(file);
                        }
                    }
                }
                player_sum_equipment_bonuses();
                player_update_durability_warning();
                {
                    world_state.draw[0].clip.w = 0;
                    world_state.draw[0].clip.h = 0;
                    world_state.draw[1].clip.w = 0;
                    world_state.draw[1].clip.h = 0;
                    r.x = 0;
                    r.y = world_state.frame ? 0 : 240;
                    r.w = 320;
                    r.h = 240;
                    MoveImage(&r, 0, world_state.frame * 240);
                    DrawSync(0);
                    player_update_status(0);
                    world_render_frame(0, 0);
                    player_update_status(0);
                    cd_wait_idle();
                    world_state.draw[0].clip.w = 320;
                    world_state.draw[0].clip.h = 240;
                    world_state.draw[1].clip.w = 320;
                    world_state.draw[1].clip.h = 240;
                }
            }
            pad_clear_input();
            lighting.paused = 2;
        }
        if (viewer.save_warp_request) {
            player_warp_to_save_point();
        }
        if ((pad_state.pressed & 8) && viewer.state != 4 && area_state.loading == 0 &&
            vis_table_a.object_sequence_state == 0 && area_state.entry_fade_start == -1) {
            lighting.paused = 1;
            play_confirm_sound();
            pad_state.pressed &= ~8;
            pad_clear_input();
            pad_vibrate(0);
            pause_screen_run();
            pad_clear_input();
            lighting.paused = 2;
        }
        viewer.walk_speed_max = 192;
        viewer.yaw_speed_max = 32;
        if (!(pad_state.held & (pad_state.buttons[0] | pad_state.buttons[1]))) {
            viewer.yaw_speed_max = 40;
        }
        if (viewer.unk190[3][0] != 0) {
            viewer.walk_speed_max = 96;
        }
        if (viewer.unk190[1][0] != 0) {
            viewer.walk_speed_max = 0;
            if (viewer.status_flags & 0x10) {
                viewer.yaw_speed_max = 0;
            }
            if (viewer.unk190[1][0] % 20 == 0) {
                sound_play(6, 128);
            }
        }
        if (viewer.equip_weight > viewer.equip_weight_limit) {
            int limit = viewer.walk_speed_max - (viewer.equip_weight - viewer.equip_weight_limit) / 5;

            if (limit < 0) {
                viewer.walk_speed_max = 0;
            } else {
                viewer.walk_speed_max = limit;
            }
        }
        switch (viewer.state) {
        case 0:
            player_update_hands();
            if (area_state.loading != 1 && vis_table_a.object_sequence_state == 0) {
                player_update_look();
                player_update_walk();
                player_update_motion();
                viewer.motion_stopped = 0;
                if ((pad_state.pressed & pad_state.buttons[11]) && pad_state.combo_timer == 0 &&
                    (viewer.equipment[EQUIP_SLOT_QUICK_1] != EQUIP_NONE
                        || viewer.equipment[EQUIP_SLOT_QUICK_2] != EQUIP_NONE ||
                     viewer.equipment[EQUIP_SLOT_QUICK_3] != EQUIP_NONE
                         || viewer.equipment[EQUIP_SLOT_QUICK_4] != EQUIP_NONE ||
                     viewer.equipment[EQUIP_SLOT_QUICK_5] != EQUIP_NONE)) {
                    play_confirm_sound();
                    pad_state.pressed &= pad_state.buttons[11];
                    viewer.state = 1;
                    pad_clear_input();
                }
            }
            player_handle_examine();
            break;
        case 1:
            player_update_hands();
            if (area_state.loading == 1 || vis_table_a.object_sequence_state) {
                goto status;
            }
            player_update_look();
            player_update_walk();
            player_update_motion();
            viewer.motion_stopped = 0;
            if (pad_state.pressed & pad_state.buttons[8]) {
                play_cursor_sound();
                pad_state.pressed &= ~pad_state.buttons[8];
                if (++viewer.quick_item_index >= 5) {
                    viewer.quick_item_index = 0;
                }
            } else if (pad_state.pressed & pad_state.buttons[9]) {
                play_cursor_sound();
                pad_state.pressed &= ~pad_state.buttons[9];
                if (--viewer.quick_item_index < 0) {
                    viewer.quick_item_index = 4;
                }
            } else {
                if ((pad_state.pressed & pad_state.buttons[11]) && pad_state.combo_timer == 0) {
                    play_cancel_sound();
                    viewer.state = 0;
                    pad_state.pressed &= ~pad_state.buttons[11];
                    pad_state.pressed &= ~pad_state.buttons[10];
                } else if ((pad_state.pressed & pad_state.buttons[10]) && pad_state.combo_timer == 0) {
                    int ids[5] = {viewer.equipment[EQUIP_SLOT_QUICK_1],
                        viewer.equipment[EQUIP_SLOT_QUICK_2], viewer.equipment[EQUIP_SLOT_QUICK_3],
                                  viewer.equipment[EQUIP_SLOT_QUICK_4], viewer.equipment[EQUIP_SLOT_QUICK_5]};

                    viewer.state = 0;
                    pad_state.pressed &= ~pad_state.buttons[8];
                    pad_state.pressed &= ~pad_state.buttons[11];
                    pad_state.pressed &= ~pad_state.buttons[9];
                    pad_state.pressed &= ~pad_state.buttons[10];
                    if (ids[viewer.quick_item_index] != 0xffff) {
                        viewer.save_warp_request = 0;
                        item_use(ids[viewer.quick_item_index]);
                        if (DAT_801cd0e0 == 1) {
                            play_confirm_sound();
                            if (ids[viewer.quick_item_index] == 288) {
                                player_compute_derived_stats(0);
                            }
                            if (inventory.counts[ids[viewer.quick_item_index]].count == 0) {
                                RECT icon = {896, 256, 24, 24};

                                switch (viewer.quick_item_index) {
                                case 0:
                                    viewer.equipment[EQUIP_SLOT_QUICK_1] = EQUIP_NONE;
                                    break;
                                case 1:
                                    viewer.equipment[EQUIP_SLOT_QUICK_2] = EQUIP_NONE;
                                    break;
                                case 2:
                                    viewer.equipment[EQUIP_SLOT_QUICK_3] = EQUIP_NONE;
                                    break;
                                case 3:
                                    viewer.equipment[EQUIP_SLOT_QUICK_4] = EQUIP_NONE;
                                    break;
                                case 4:
                                    viewer.equipment[EQUIP_SLOT_QUICK_5] = EQUIP_NONE;
                                    break;
                                }
                                icon.y = viewer.quick_item_index * 24 + 256;
                                ClearImage(&icon, 0, 0, 0);
                            }
                        } else if (DAT_801cd0e0 == 2) {
                            play_refuse_sound();
                        }
                    } else {
                        play_refuse_sound();
                    }
                } else {
                    goto status;
                }
                pad_clear_input();
            }
        status:
            player_handle_examine();
            break;
        case 4:
            if (viewer.death_timer < 17) {
                fade_add_color(255, 255, 255, viewer.death_timer << 8, 2);
                viewer.rot_offset.halves[0] += viewer.death_pitch_step;
                if (viewer.death_timer == 16) {
                    area_state.hooks->player_died();
                    fade_add_color(255, 255, 255, viewer.death_timer << 8, 2);
                }
            } else {
                fade_add_color(255, 255, 255, 4096, 2);
                if (viewer.death_timer == 20) {
                    RECT clear = {0, 0, 320, 480};

                    anim_slots_free_idle();
                    anim_sets_free_idle();
                    anim_slots_free_all();
                    anim_sets_free_all();
                    fx = effects.slots;
                    k = 128;
                    while (k--) {
                        fx->flags = 255;
                        fx++;
                    }
                    for (;;) {
                        if (menu_game_over_run() == 0) {
                            AreaObject *b;

                            b = vis_table_b.entries;
                            fade_reset();
                            viewer.red_fade = viewer.red_fade_step = viewer.yellow_fade = viewer.yellow_fade_step = 0;
                            fade_run_frames(2, 4096, 4096, 256);
                            n = 240;
                            while (n--) {
                                if (b->kind == 236 && b->unk54 != -1) {
                                    sound_stop(b->unk54, b->params.snd.sound + 1108);
                                    b->unk54 = -1;
                                }
                                b++;
                            }
                            game_exit_request = 2;
                            world_state.no_plane_test = 0;
                            break;
                        }
                        hud_clear_damage_numbers();
                        ClearImage(&clear, 0, 0, 0);
                        DrawSync(0);
                        card_screen_open(0, 1);
                        if (card_screen_run(1) == 1) {
                            AreaObject *b;

                            b = vis_table_b.entries;
                            n = 240;
                            while (n--) {
                                if (b->kind == 236 && b->unk54 != -1) {
                                    sound_stop(b->unk54, b->params.snd.sound + 1108);
                                    b->unk54 = -1;
                                }
                                b++;
                            }
                            pad_clear_input();
                            card_screen_close();
                            fade_reset();
                            viewer.red_fade = viewer.red_fade_step = viewer.yellow_fade = viewer.yellow_fade_step = 0;
                            fade_run_frames(2, 0, 4096, 256);
                            cd_wait_idle();
                            player_reload_equipment();
                            vis_table_a.zone = 254;
                            vis_table_a.zone_history[0] = vis_table_a.zone_history[1] =
                                vis_table_a.zone_history[2] = vis_table_a.zone_history[3] = 255;
                            pad_vibrate(0);
                            area_load_set_blocking(area_state.loaded_area, area_state.loaded_area, area_state.loaded_area,
                                          area_state.loaded_sound_bank, area_state.unk8, 255);
                            cd_wait_idle();
                            player_stop_motion();
                            break;
                        }
                        pad_clear_input();
                        card_screen_close();
                        ClearImage(&clear, 0, 0, 0);
                        DrawSync(0);
                    }
                }
            }
            viewer.death_timer++;
            break;
        case 16:
            player_update_hands();
            player_update_look();
            player_update_walk();
            if (viewer.rot_offset.word == 0 && viewer.roll_offset == 0) {
                player_enter_normal_state();
            }
            player_update_motion();
            viewer.motion_stopped = 0;
            player_handle_examine();
            break;
        case 5:
        case 6:
            player_stop_motion();
            vis_table_b.push_x = vis_table_b.push_y = vis_table_b.push_z = 0;
            vis_lists.floor_push_x = vis_lists.floor_push_y = vis_lists.floor_push_z = 0;
            vis_table_a.push_x = vis_table_a.push_y = vis_table_a.push_z = 0;
            viewer.push_x = viewer.push_y = viewer.push_z = 0;
            break;
        }
        {
            u16 hit[9];
            long from[4];
            u16 hit2[9];
            long from2[4];
            u16 hit3[9];
            long from3[4];

            if ((vis_lists.floor_kinds & 0x10) && vis_lists.floor_dist < 1601) {
                memset(hit, 0, sizeof(hit));
                memset(from, 0, sizeof(from));
                player_hit(hit, 4096, 7, from);
            }
            if ((vis_lists.floor_kinds & 0x20) && vis_lists.floor_dist < 1601) {
                memset(hit2, 0, sizeof(hit2));
                memset(from2, 0, sizeof(from2));
                player_hit(hit2, 4096, 1024, from2);
            }
            if ((vis_lists.floor_kinds & 8) && vis_lists.floor_dist < 1601) {
                memset(hit3, 0, sizeof(hit3));
                hit3[3] = 150;
                memset(from3, 0, sizeof(from3));
                player_hit(hit3, 4096, 0, from3);
            }
            if (viewer.unk190[0][0] != 0) {
                d = 0;
                if (viewer.status_flags & 4) {
                    d = -256;
                } else if (viewer.status_flags & 2) {
                    d = -64;
                } else if (viewer.status_flags & 1) {
                    d = -16;
                }
                if (viewer.unk190[0][0] % 20 == 0) {
                    player_add_hp(d);
                    sound_play(5, 128);
                }
            }
            if (viewer.unk190[4][0] != 0 && viewer.unk190[4][0] % 20 == 0) {
                player_add_hp(-1);
                player_add_mp(-1);
            }
            if (viewer.equip_weight > viewer.equip_weight_limit && viewer.frame_count % 20 == 0) {
                player_add_hp(viewer.equip_weight_limit / 10 - viewer.equip_weight / 10);
            }
            if ((viewer.status_flags & 0x400) || viewer.submerged_acid) {
                if (viewer.frame_count % 20 == 0) {
                    int heal = (viewer.element_defence[3] - 256) / 4;

                    if (viewer.status_flags & 0x800000) {
                        player_add_hp(heal / 2);
                    } else {
                        player_add_hp(heal);
                    }
                }
                if (viewer.frame_count % 60 == 0) {
                    u16 *slots[9] = {&viewer.equipment[EQUIP_SLOT_HELMET],
                        &viewer.equipment[EQUIP_SLOT_ARMOR], &viewer.equipment[EQUIP_SLOT_BOOTS],
                                     &viewer.equipment[EQUIP_SLOT_RIGHT_HAND],
                                         &viewer.equipment[EQUIP_SLOT_LEFT_HAND],
                                             &viewer.equipment[EQUIP_SLOT_GLOVES],
                                     &viewer.equipment[EQUIP_SLOT_RIGHT_RING],
                                         &viewer.equipment[EQUIP_SLOT_LEFT_RING], NULL};

                    if (viewer.submerged_acid) {
                        int j;

                        if (viewer.status_flags & 0x800000) {
                            for (j = 0; j < 8; j++) {
                                if (*slots[j] != 0xffff) {
                                    player_wear_item(1, slots[j]);
                                }
                            }
                        } else {
                            for (j = 0; j < 8; j++) {
                                if (*slots[j] != 0xffff) {
                                    player_wear_item(2, slots[j]);
                                }
                            }
                        }
                    } else if (viewer.status_flags & 0x800000) {
                        if (*slots[2] != 0xffff) {
                            player_wear_item(1, slots[2]);
                        }
                    } else if (*slots[2] != 0xffff) {
                        player_wear_item(2, slots[2]);
                    }
                }
                viewer.status_flags &= ~0x400;
            }
            player_update_shake();
            viewer.pitch = viewer.view_rot.vx + viewer.rot_offset.halves[0] + viewer.shake_pitch + viewer.fall_pitch;
            viewer.yaw = viewer.view_rot.vy + viewer.rot_offset.halves[1] + viewer.shake_yaw + viewer.unk292;
            viewer.roll = viewer.view_rot.vz + viewer.roll_offset + viewer.shake_roll + viewer.unk294;
            viewer.shake_pitch = viewer.shake_yaw = viewer.shake_roll = 0;
            viewer.camera_rot.vx = 4096 - viewer.pitch;
            viewer.camera_rot.vy = 4096 - viewer.yaw;
            viewer.camera_rot.vz = 4096 - viewer.roll;
            RotMatrixZYX(&viewer.camera_rot, &viewer.camera_matrix);
            vis_lists.floor_push_x /= 2;
            vis_lists.floor_push_y /= 2;
            vis_lists.floor_push_z /= 2;
            if (vis_lists.floor_push_x >= -4 && vis_lists.floor_push_x < 4) {
                vis_lists.floor_push_x = 0;
            }
            if (vis_lists.floor_push_y >= -4 && vis_lists.floor_push_y < 4) {
                vis_lists.floor_push_y = 0;
            }
            if (vis_lists.floor_push_z >= -4 && vis_lists.floor_push_z < 4) {
                vis_lists.floor_push_z = 0;
            }
            player_update_hand_action(0);
            player_update_hand_action(1);
            player_update_cast(0);
            player_update_cast(1);
            if (viewer.state != 4) {
                if (viewer.hp * 2 < viewer.max_hp && viewer.hp_regen_interval && viewer.frame_count % viewer.hp_regen_interval == 0) {
                    player_add_hp(1);
                }
                if (viewer.mp * 2 < viewer.max_mp && viewer.mp_regen_interval && viewer.frame_count % viewer.mp_regen_interval == 0) {
                    player_add_mp(1);
                }
            }
            viewer.frame_count++;
        }
    }
}
