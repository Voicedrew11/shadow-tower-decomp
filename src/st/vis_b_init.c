/*
 * ST.EXE: playing a sound at a table B entry, clearing table B
 * (vis_table_b.entries) and resetting one entry, and a table C entry's
 * state switch (WIP module; the area-file loader objects_load after it is
 * not reconstructed).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/funcs.h>

/* Plays sound `sound` at the entry's position when the area has it. */
ADDRESS(0x8004ed54, 0x54)
void object_play_sound(Vis88 *e, int sound)
{
    if (DAT_801dd1c0.sounds[sound].slot != -1)
        func_80058220(sound, &e->unk24, 127, 0);
}

/* Frees every table B entry. */
ADDRESS(0x8004eda8, 0x58)
void objects_clear(void)
{
    Vis88 *e = vis_table_b.entries;
    u16 i = 240;

    while (i--) {
        e->unk0a = 0xffff;
        e->unk05 = 0xff;
        ((u32 *)e->unk44)[2] = 0;
        ((u32 *)e->unk44)[1] = 0;
        ((u32 *)e->unk44)[0] = 0;
        e++;
    }
    vis_table_b.unk727A = 0;
    vis_table_b.unk7278 = 0;
    vis_table_b.unk7276 = 0;
}

/* Resets an entry to the neutral state: unit scale, no rotation, no kind. */
ADDRESS(0x8004ee00, 0x48)
void object_reset(Vis88 *e)
{
    e->unk02 = 0x80;
    e->unk3C.vz = 0x1000;
    e->unk3C.vy = 0x1000;
    e->unk3C.vx = 0x1000;
    e->unk10 = 0;
    e->unk03 = 0;
    e->unk01 = 0;
    e->unk38 = 0;
    e->unk34 = 0;
    e->unk36 = 0;
    e->unk05 = 0xff;
    e->unk14 = 0;
    e->unk06 = 0xff;
    e->unk16 = 0;
}

/* Mode 0 parks table C entry `index` (state 4) unless it is free; mode 1
 * wakes a parked one (state 2). */
ADDRESS(0x8004ee48, 0x74)
void ground_item_set_hidden(index, mode)
    u8 index;
    u8 mode;
{
    Vis48 *c;

    if (index == 0xff)
        return;
    c = &vis_table_c.entries[index];
    switch (mode) {
    case 0:
        if (c->unk22)
            c->unk22 = 4;
        break;
    case 1:
        if (c->unk22 == 4)
            c->unk22 = 2;
        break;
    }
}
