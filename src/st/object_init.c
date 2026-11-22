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
void object_play_sound(AreaObject *e, int sound)
{
    if (sound_state.sounds[sound].slot != -1)
        sound_play_at(sound, &e->position, 127, 0);
}

/* Frees every table B entry. */
ADDRESS(0x8004eda8, 0x58)
void objects_clear(void)
{
    AreaObject *e = vis_table_b.entries;
    u16 i = 240;

    while (i--) {
        e->type = 0xffff;
        e->kind = OBJECT_KIND_NONE;
        ((u32 *)&e->anim_slot)[2] = 0;
        ((u32 *)&e->anim_slot)[1] = 0;
        ((u32 *)&e->anim_slot)[0] = 0;
        e++;
    }
    vis_table_b.unk727A = 0;
    vis_table_b.unk7278 = 0;
    vis_table_b.unk7276 = 0;
}

/* Resets an entry to the neutral state: unit scale, no rotation, no kind. */
ADDRESS(0x8004ee00, 0x48)
void object_reset(AreaObject *e)
{
    e->anim = 0x80;
    e->scale.vz = 0x1000;
    e->scale.vy = 0x1000;
    e->scale.vx = 0x1000;
    e->anim_frame = 0;
    e->draw_flags = 0;
    e->unk01 = 0;
    e->rotation.vz = 0;
    e->rotation.vx = 0;
    e->rotation.vy = 0;
    e->kind = OBJECT_KIND_NONE;
    e->depth_bias = 0;
    e->unk06 = 0xff;
    e->light = 0;
}

/* Mode 0 parks table C entry `index` (state 4) unless it is free; mode 1
 * wakes a parked one (state 2). */
ADDRESS(0x8004ee48, 0x74)
void ground_item_set_hidden(index, mode)
    u8 index;
    u8 mode;
{
    GroundItem *c;

    if (index == 0xff)
        return;
    c = &vis_table_c.entries[index];
    switch (mode) {
    case 0:
        if (c->state)
            c->state = GROUND_ITEM_HIDDEN;
        break;
    case 1:
        if (c->state == GROUND_ITEM_HIDDEN)
            c->state = GROUND_ITEM_RESTING;
        break;
    }
}
