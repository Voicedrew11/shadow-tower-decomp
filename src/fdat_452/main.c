/*
 * FDAT_452.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), the scene reset it shares with the
 * other three shop overlays (src/lib/fdat_map_enter_shop.inc) and its
 * shopkeeper handlers. One object, so that its two scene words are consecutive
 * statics after the code.
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

RODATA(0x801eb560, 0x1cc)

/* A word no handler touches; it keeps the scene words four bytes off the code. */
DATA(0x801ed980, 0x4, ".bss")
static int fdat_801ed8c0;

/* The visit state: 0 until the shopkeeper's line is picked (map_enter_shop clears it). */
DATA(0x801ed984, 0x4, ".bss")
static int fdat_801ed8c4;

/* Which line the shopkeeper says, by the viewer's state (see map_pick_shopkeeper_line). */
DATA(0x801ed988, 0x4, ".bss")
static int fdat_801ed988;

void map_enter_shop();
void map_pick_shopkeeper_line();
void map_run_shop_visit();

const FdatHeader fdat_header = {
    0x32,
    {
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_enter_shop, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_pick_shopkeeper_line, map_run_shop_visit, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
        map_hook_none, map_hook_none, map_hook_none, map_hook_none,
    },
    0x65,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

#include "../lib/fdat_map_enter_shop.inc"

void SsSetSerialVol(short s_num, short voll, short volr);
void player_enter_frozen_state(void);
void shop_run(void);

/*
 * Once: picks the shopkeeper's line. Every item the viewer holds outside its slots
 * (ids below 266) makes line 0 or later; none makes line 1; then the
 * second flag word's bits 1-2 and the viewer's figures choose 0, 2, 3 or
 * 4 (no stream). The entry plays the line's animation and its XA
 * stream starts.
 */
ADDRESS(0x801ed454, 0x2ec)
void map_pick_shopkeeper_line(void)
{
    Creature *e = vis_table_a.current;
    CreatureType *r = &vis_table_a.types[e->type];
    int none;
    int i;

    if (fdat_801ed8c4 != 0)
        return;
    fdat_801ed8c4 = 1;
    none = 1;
    for (i = 0; i < 266; i++) {
        if (inventory.counts[i].count == 0)
            continue;
        if (i == viewer.equipment[EQUIP_SLOT_RIGHT_RING] || i == viewer.equipment[EQUIP_SLOT_RIGHT_BRACELET]
            || i == viewer.equipment[EQUIP_SLOT_GLOVES]
            || i == viewer.equipment[EQUIP_SLOT_RIGHT_HAND]
            || i == viewer.equipment[EQUIP_SLOT_RIGHT_RESERVE] || i == viewer.equipment[EQUIP_SLOT_QUICK_1]
            || i == viewer.equipment[EQUIP_SLOT_AMULET] || i == viewer.equipment[EQUIP_SLOT_HELMET]
            || i == viewer.equipment[EQUIP_SLOT_ARMOR]
            || i == viewer.equipment[EQUIP_SLOT_BOOTS] || i == viewer.equipment[EQUIP_SLOT_QUICK_3]
            || i == viewer.equipment[EQUIP_SLOT_LEFT_RING]
            || i == viewer.equipment[EQUIP_SLOT_LEFT_BRACELET]
            || i == viewer.equipment[EQUIP_SLOT_GLOVES_MIRROR] || i == viewer.equipment[EQUIP_SLOT_LEFT_HAND]
            || i == viewer.equipment[EQUIP_SLOT_LEFT_RESERVE] || i == viewer.equipment[EQUIP_SLOT_QUICK_5]
            || i == viewer.equipment[EQUIP_SLOT_QUICK_2]
            || i == viewer.equipment[EQUIP_SLOT_QUICK_4])
            continue;
        none = 0;
        break;
    }
    if (none) {
        fdat_801ed988 = 1;
        creature_start_script(e, 1);
    } else if ((game_progress.event_flags2.word >> 1 & 3) == 0) {
        fdat_801ed988 = 0;
        creature_start_script(e, 0);
        game_progress.event_flags2.word = (game_progress.event_flags2.word & ~6) | 2;
    } else if (viewer.hp < viewer.max_hp >> 3) {
        fdat_801ed988 = 2;
        creature_start_script(e, 2);
    } else if (viewer.hp == viewer.max_hp) {
        fdat_801ed988 = 3;
        creature_start_script(e, 2);
    } else {
        fdat_801ed988 = 4;
        creature_start_script(e, 3);
        fdat_801ed8c4 = 4;
    }
    if (fdat_801ed988 != 4) {
        cd_wait_idle();
        talk_load_image(r->talk_id * 6 + fdat_801ed988, 1);
    }
}

/*
 * The shopkeeper entry's handler: steps the visit stages in fdat_801ed8c4
 * (1 waiting for half the animation, 2 streaming, 3 stream done, 4 menu,
 * 5 done) by the XA stream state in the world block.
 */
ADDRESS(0x801ed740, 0x240)
void map_run_shop_visit(void)
{
    Creature *e = vis_table_a.current;
    CreatureType *r = vis_table_a.current_type;
    MenuScratch *menu = MENU_SCRATCH;

    e->status.bytes[2] &= 0xf2;
    e->fade = -1;
    if (e->anim_slot == 0)
        return;
    if (fdat_801ed8c4 == 2 && xa_stream_finished()) {
        if (world_state.talk_overlay_state == 3) {
            xa_stop_stream();
            world_state.talk_frame = 0;
            world_state.talk_overlay_state = 4;
        }
    }
    if (world_state.talk_overlay_state == 5) {
        fdat_801ed8c4 = 3;
        world_state.talk_overlay_state = 0;
    }
    e->anim_frame += e->anim_step;
    if ((short)e->anim_frame >= 0x1000) {
        e->anim_frame = 0;
        if (fdat_801ed8c4 == 2 && fdat_801ed988 == 0)
            creature_start_script(e, 3);
    }
    if (fdat_801ed8c4 == 1) {
        int ending = fdat_801ed988;
        if (ending != 0 || (short)e->anim_frame >= 0x800) {
            viewer.unk1E4 = 0;
            xa_start_stream(r->talk_id, ending);
            SsSetSerialVol(0, 0x40, 0x40);
            world_state.talk_overlay_state = 2;
            world_state.talk_overlay_mode = 1;
            world_state.talk_frame = 0;
            fdat_801ed8c4 = 2;
            player_enter_frozen_state();
        }
    } else if (fdat_801ed8c4 == 3) {
        if (fdat_801ed988 == 1)
            fdat_801ed8c4 = 5;
        else
            fdat_801ed8c4 = 4;
    } else if (fdat_801ed8c4 == 4) {
        pad_state.pressed = 0;
        menu->mode = 2;
        menu->font_mode = MENU_FONT_LOW_RES;
        shop_run();
        fdat_801ed8c4 = 5;
    } else if (fdat_801ed8c4 == 5) {
        DAT_801cd0e4 = 1;
    }
}
