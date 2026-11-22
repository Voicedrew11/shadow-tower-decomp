/*
 * FDAT_282.BIN: its handlers after fdat_801ed4a0 (WIP module): the step of
 * table A entries 19 and 48..50, and the two door scenes.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/xa.h>
#include <st/fdat.h>

void func_801edba4(int id);
void func_801ede88(void);

/*
 * Steps table A entries 19 and 48..50: 48 and 49 open their doors when the
 * viewer stands on the trigger records, 50 opens the gate, 19 counts its
 * hits; then runs the script like fdat_801ed6e4.
 */
ADDRESS(0x801ed7a4, 0x274)
void func_801ed7a4(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    Vis212 *a48 = &vis_table_a.entries[48];
    Vis212 *a50 = &vis_table_a.entries[50];
    Vis212 *a19 = &vis_table_a.entries[19];
    Vis212 *a49 = &vis_table_a.entries[49];

    if (e == a48) {
        if (viewer.unk178 == 0x55 || viewer.unk178 == 0x29) {
            func_801edba4(0x55);
            creature_despawn(e);
            return;
        }
        if (viewer.unk178 == 0x3b)
            e->unk8E = 0x800;
        else if (viewer.unk178 == 0x13d)
            e->unk8E = 0;
    }
    if (e == a50 && viewer.unk178 == 0x65) {
        func_801ede88();
        creature_despawn(e);
        creature_despawn(a48);
        creature_despawn(a49);
        return;
    }
    if (e == a19) {
        if (e->unk64 != 0) {
            e->unk1C.bytes[3] = 0;
            DAT_801cba08.unk1448[r->unk09]++;
        }
        if (DAT_801cba08.unk1448[r->unk09] == 4)
            e->unk70 &= ~0x2000000;
    }
    if (e == a49 && viewer.unk178 == 0x61) {
        func_801edba4(0x61);
        creature_despawn(e);
        return;
    }
    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e == a48 || e == a49)
            fdat_801eb7b4(e, 0);
        else if (e == a50)
            fdat_801eb7b4(e, 4);
        else if (e == a19 && DAT_801cba08.unk1448[r->unk09] == 3)
            fdat_801eb7b4(e, 0);
        else {
            e->unk1A = 0xff;
            fdat_801ed4a0();
        }
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}

ADDRESS(0x801eda18, 0x8)
void func_801eda18(void)
{
}

/*
 * Opens door 26 of table B: raises it to 0xfff, dropping item 2 on the way.
 */
ADDRESS(0x801eda20, 0x184)
void func_801eda20(void)
{
    int step;
    VECTOR pos;
    SVECTOR rot;
    Vis88 *door = &vis_table_b.entries[26];
    Vis48 *item = &vis_table_c.entries[2];

    viewer.unk218 = 5;
    rot.vy = -1024;
    rot.vx = 0;
    pos.vx = door->unk24.vx - 3072;
    pos.vz = door->unk24.vz;
    fdat_801ecfc4(&pos, &rot);
    step = 28;
    do {
        if (door->unk10 < 0xf00 && door->unk10 + step >= 0xf00)
            object_play_sound(door, door->unk0a * 3 + 208);
        door->unk10 += step;
        if (door->unk10 >= 0x1000)
            door->unk10 = 0xfff;
        if (door->unk10 > 0x400 && item->unk22 == 4)
            item->unk22 = 2;
        if (door->unk10 > 0xf00 && item->unk22 == 2)
            item->unk22 = 1;
        ground_items_update();
        world_run_frame(NULL, NULL, 0);
    } while (door->unk10 != 0xfff);
    door->unk0C = 0xff;
    viewer.unk218 = 0;
}

/*
 * The door scene of table A entry 48 (`id` 0x55, door 2) or 49 (door 1):
 * plays the entry's opening script, then raises the door and swaps its
 * records.
 */
ADDRESS(0x801edba4, 0x2e4)
void func_801edba4(int id)
{
    Vis212 *e;
    Vis88 *door;
    Vis192 *r;
    u16 step;
    u16 n;
    u8 t;

    if (id == 0x55) {
        e = &vis_table_a.entries[48];
        door = &vis_table_b.entries[2];
    } else {
        e = &vis_table_a.entries[49];
        door = &vis_table_b.entries[1];
    }
    r = &vis_table_a.unk3500[e->unk02];
    if (id == 0x55)
        DAT_801cba08.unk1448[r->unk09] = 0;
    else
        DAT_801cba08.unk1448[r->unk09] = 1;
    step = door->unk48 * 4;
    r->unk60[0]->unk00 = 0x70;
    r->unk60[1]->unk00 = 0x71;
    r->unk60[2]->unk00 = 0x72;
    r->unk60[5]->unk00 = 0x80;
    r->unk60[6]->unk00 = 0x80;
    r->unk60[7]->unk00 = 0x80;
    fdat_801ed388(e, 0, 0);
    DAT_801cba08.unk1448[r->unk09] = 3;
    fdat_801eb7b4(e, 3);
    do {
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        if ((short)e->unk2A + e->unkB0 >= 0x800 && (short)e->unk2A < 0x800) {
            viewer.unk190[4][0] = 0;
            viewer.unk190[4][1] = 6000;
            viewer.unk2C |= 0x100;
        }
        world_run_frame(NULL, NULL, 0);
    } while ((short)e->unk2A != 0xfff);
    for (;;) {
        if (door->unk10 < 1 && door->unk10 + step >= 1)
            object_play_sound(door, door->unk0a * 3 + 208);
        door->unk10 += step;
        if (door->unk10 + step >= 0x1000)
            break;
        world_run_frame(NULL, NULL, 0);
    }
    door->unk10 = 0xfff;
    door->unk00 = 0;
    door->unk0C = 0xff;
    n = door->unk4A.door.unk4E;
    while (n--) {
        t = vis_records[door->unk4A.door.unk4C + n].part;
        vis_records[door->unk4A.door.unk4C + n].part = vis_records[door->unk4A.door.unk4A + n].part;
        vis_records[door->unk4A.door.unk4A + n].part = t;
    }
}

/*
 * The gate scene of table A entry 50: swaps door 3's records, raises it and
 * plays the entry's script to its end.
 */
ADDRESS(0x801ede88, 0x228)
void func_801ede88(void)
{
    int raising_step;
    Vis212 *e = &vis_table_a.entries[50];
    Vis192 *r = &vis_table_a.unk3500[e->unk02];
    Vis88 *door = &vis_table_b.entries[3];
    u16 step = door->unk48 * 4;
    u16 n;
    u8 t;

    r->unk60[0]->unk00 = 0x80;
    r->unk60[1]->unk00 = 0x80;
    r->unk60[2]->unk00 = 0x80;
    r->unk60[5]->unk00 = 0x70;
    r->unk60[6]->unk00 = 0x71;
    r->unk60[7]->unk00 = 0x72;
    DAT_801cba08.unk1448[r->unk09] = 2;
    fdat_801ed388(e, 0, 0);
    n = door->unk4A.door.unk4E;
    while (n--) {
        t = vis_records[door->unk4A.door.unk4C + n].part;
        vis_records[door->unk4A.door.unk4C + n].part = vis_records[door->unk4A.door.unk4A + n].part;
        vis_records[door->unk4A.door.unk4A + n].part = t;
    }
    /* The look-ahead test uses the promoted halfword step. */
    raising_step = step;
    for (;;) {
        door->unk10 += step;
        if (door->unk10 + raising_step >= 0x1000)
            goto gate_raised;
        world_run_frame(NULL, NULL, 0);
    }
gate_raised:
    door->unk10 = 0xfff;
    door->unk00 = 0;
    door->unk0C = 0xff;
    fdat_801eb7b4(e, 8);
    do {
        e->unk2A += e->unkB0;
        if ((short)e->unk2A >= 0x1000)
            e->unk2A = 0xfff;
        world_run_frame(NULL, NULL, 0);
    } while ((short)e->unk2A != 0xfff);
    DAT_801cba08.unk1448[r->unk09] = 3;
}
