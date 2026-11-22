/*
 * FDAT_212.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), then its map handlers through the
 * shared creature_pick_move. One object, because creature_pick_move's switch table
 * is 8-aligned after creature_run_script_command's in the same read-only run.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/anim.h>
#include <st/sound.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/xa.h>
#include <st/vis.h>
#include <st/fdat.h>
#include <LIBGPU.H>
#include <st/menu.h>
#include <st/light.h>
#include <st/bank.h>

RODATA(0x801eb560, 0x24c)

void map_restore_progress();
void map_update_events();
void map_use_key_on_object();

const FdatHeader fdat_header = {
    0x1a,
    {
        map_hook_none, map_update_events, map_hook_none, map_hook_none,
        map_hook_none, map_restore_progress, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_use_key_on_object, map_hook_none, map_hook_none, map_hook_none,
        creature_pick_move, creature_step_script, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x4d,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

/* Set once key item 0x114 unlocks table B entry 46 (0x2e), which carries
 * table C item 28. */
DATA(0x801edcd4, 0x4, ".bss")
static int fdat_801edcd4;

/* Set once key item 0x114 unlocks table B entry 2, which carries table C
 * item 29. */
DATA(0x801edcd8, 0x4, ".bss")
static int fdat_801edcd8;

/* The bridge (table B kind 0x97): 1 rising, 2 risen, 0xff down. */
DATA(0x801edcdc, 0x4, ".bss")
static int fdat_801edcdc;

/* Frames the bridge has risen. */
DATA(0x801edce0, 0x4, ".bss")
static int fdat_801edce0;

void map_slide_ground_item(int which);

/* Sets up the area by the second flag word's bits 8 and 9: table C items
 * 28/29, the scene words and the bridge pieces (table B kind 0x97). */
ADDRESS(0x801ed4a0, 0xf4)
void map_restore_progress(void)
{
    Vis48 *c28 = &vis_table_c.entries[28];
    Vis88 *b;
    int i;

    c28[1].unk22 = 4;
    c28->unk22 = 4;
    if (DAT_801cba08.unk1444.bit.f8 == 1)
        fdat_801edcd4 = 0xff;
    else
        fdat_801edcd4 = 0;
    if (DAT_801cba08.unk1444.bit.f9 == 1) {
        b = vis_table_b.entries;
        fdat_801edcd8 = 0xff;
        for (i = 239; i != -1; i--) {
            if (b->unk0a == 0x97)
                b->unk00 = 2;
            b++;
        }
        fdat_801edcdc = 0xff;
        vis_lists.unk47E0 |= 0x20;
    } else {
        fdat_801edcd8 = 0;
        fdat_801edcdc = 0;
        vis_lists.unk47E0 &= ~0x20;
    }
}

/*
 * Runs the area: unlocking table B entries 46 and 2 starts them (each
 * carrying its table C item on node 0, vertex 0x23); once entry 2 and then
 * entry 3 are done the bridge rises 0x18 a frame for 0x97 frames.
 */
ADDRESS(0x801ed594, 0x2fc)
void map_update_events(void)
{
    Vis48 *c28 = &vis_table_c.entries[28];
    Vis48 *c29 = &vis_table_c.entries[29];
    Vis88 *d46 = &vis_table_b.entries[46];
    Vis88 *d2 = &vis_table_b.entries[2];
    Vis88 *d3 = &vis_table_b.entries[3];
    Vis88 *b;
    VECTOR v1;
    VECTOR v2;
    int i;

    if (fdat_801edcd4 == 1) {
        map_slide_ground_item(0);
        fdat_801edcd4 = 2;
        d46->unk0C = 1;
        d46->unk02 = 0;
        DAT_801cba08.unk1444.word |= 0x100;
    } else if (fdat_801edcd4 == 2) {
        anim_set_node_vertex(*(StAnimSet **)d46->unk44, object_model_slot(d46->unk0a), 0, 0x23, &v1);
        c28->unk08.vx = d46->unk24.vx + v1.vx;
        c28->unk08.vy = d46->unk24.vy + v1.vy;
        c28->unk08.vz = d46->unk24.vz + v1.vz - 0x12;
        if (d46->unk0C == 0xff)
            fdat_801edcd4 = 0xff;
    }
    if (fdat_801edcd8 == 1) {
        map_slide_ground_item(1);
        fdat_801edcd8 = 2;
        d2->unk0C = 1;
        d2->unk02 = 0;
        DAT_801cba08.unk1444.word |= 0x200;
    } else if (fdat_801edcd8 == 2) {
        anim_set_node_vertex(*(StAnimSet **)d2->unk44, object_model_slot(d2->unk0a), 0, 0x23, &v2);
        c29->unk08.vx = d2->unk24.vx + v2.vx;
        c29->unk08.vy = d2->unk24.vy + v2.vy;
        c29->unk08.vz = d2->unk24.vz + v2.vz - 0x12;
        if (d2->unk0C == 0xff)
            fdat_801edcd8 = 0xfe;
    } else if (fdat_801edcd8 == 0xfe && d3->unk0C == 0xff) {
        fdat_801edcdc = 1;
        fdat_801edce0 = 0;
        fdat_801edcd8 = 0xff;
    }
    if (fdat_801edcdc == 1) {
        Vis88 *rising_bridge = vis_table_b.entries;
        for (i = 239; i != -1; i--) {
            if (rising_bridge->unk0a == 0x97)
                rising_bridge->unk24.vy += 0x18;
            rising_bridge++;
        }
        if (++fdat_801edce0 >= 0x97)
            fdat_801edcdc = 2;
    } else if (fdat_801edcdc == 2) {
        b = vis_table_b.entries;
        for (i = 239; i != -1; i--) {
            if (b->unk0a == 0x97)
                b->unk00 = 2;
            b++;
        }
        fdat_801edcdc = 0xff;
        vis_lists.unk47E0 |= 0x20;
    }
}

/* Records which of table B entries 46 (0x2e) and 2 key item 0x114
 * unlocked: item_use passes 0x4000 | the object's index. */
ADDRESS(0x801ed890, 0x4c)
void map_use_key_on_object(int event)
{
    short id = event & ~0x4000;

    if (event & 0x4000) {
        if (id == 0x2e)
            fdat_801edcd4 = 1;
        else if (id == 2)
            fdat_801edcd8 = 1;
    }
}

#include "../lib/fdat_801ed4a0.inc"
#include "../lib/fdat_801ed6e4.inc"

/* Slides table C item 28 (`which` 0) or 29 0x200 along z (horizontal),
 * 24 a frame while its model is loaded. */
ADDRESS(0x801edbd0, 0x104)
void map_slide_ground_item(int which)
{
    int step = 24;
    Vis48 *item;
    Vis44 *kind;
    int end;

    if (which == 0)
        item = &vis_table_c.entries[28];
    else
        item = &vis_table_c.entries[29];
    item->unk22 = 5;
    kind = &vis_table_c.unk0118[item->unk00];
    end = item->unk08.vz + 0x200;
    item->unk2D -= 12;
    for (;;) {
        StAnimFile *file;

        if (item->unk08.vz == end)
            break;
        anim_get_file(kind->unk4 + 588, &file);
        if (file != NULL)
            item->unk08.vz += step;
        if (item->unk08.vz > end)
            item->unk08.vz = end;
        world_run_frame(NULL, NULL, 0);
    }
    play_unlock_sound();
}
