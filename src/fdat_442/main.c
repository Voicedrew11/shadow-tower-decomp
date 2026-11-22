/*
 * FDAT_442.BIN as one translation unit (WIP module): its handler table,
 * the 21 functions every overlay starts with (src/lib/fdat_common_a.inc,
 * fdat_common.inc, fdat_common_b.inc), the scene reset it shares with the
 * other three ending overlays (src/lib/fdat_801ed420.inc) and its ending
 * handlers. One object, so that its two scene words are consecutive
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
DATA(0x801ed8c0, 0x4, ".bss")
static int fdat_801ed8c0;

/* The scene state: 0 until the ending is picked (map_enter_shop clears it). */
DATA(0x801ed8c4, 0x4, ".bss")
static int fdat_801ed8c4;

/* Which ending plays: 0..3 by the viewer's state (see map_pick_shopkeeper_line). */
DATA(0x801ed8c8, 0x4, ".bss")
static int fdat_801ed8c8;

void map_enter_shop();
void map_pick_shopkeeper_line();
void map_run_shop_visit();

const FdatHeader fdat_header = {
    0x31,
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
    0x64,
};

#include "../lib/fdat_common_a.inc"
#include "../lib/fdat_common.inc"
#include "../lib/fdat_common_b.inc"

#include "../lib/fdat_801ed420.inc"

void SsSetSerialVol(short s_num, short voll, short volr);
void player_enter_frozen_state(void);
void shop_run(void);

/*
 * Once: picks the ending: item 266 not held makes ending 1; flags 31 and
 * the second word's bit 0 clear make 0; then no stock, the viewer's
 * figures and a random draw choose 2, 3, 4 or 5 (no stream).
 */
ADDRESS(0x801ed454, 0x1e8)
void map_pick_shopkeeper_line(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = &vis_table_a.unk3500[e->unk02];
    int sum;
    u32 flag31;
    int i;
    Data801c7590 *items;

    if (fdat_801ed8c4 != 0)
        return;
    fdat_801ed8c4 = 1;
    items = &DAT_801c7590;
    if (!(*(u16 *)&items->counts[266] & 0x7f)) {
        fdat_801ed8c8 = 1;
        creature_start_script(e, 1);
    } else {
        flag31 = DAT_801cba08.unk1440.word >> 31;
        if (((DAT_801cba08.unk1444.word & 1) << 1 | flag31) == 0) {
            fdat_801ed8c8 = 0;
            creature_start_script(e, 0);
            DAT_801cba08.unk1440.word |= 0x80000000;
            DAT_801cba08.unk1444.word &= ~1;
        } else {
            sum = 0;
            for (i = 0; i < 14; i++)
                sum += DAT_801c7590.unk156[i];
            if (sum == 0) {
                fdat_801ed8c8 = 2;
                creature_start_script(e, 2);
            } else if (viewer.unk2 < viewer.unk0 >> 3) {
                fdat_801ed8c8 = 3;
                creature_start_script(e, 3);
            } else if (rand() < 0x2000) {
                fdat_801ed8c8 = 4;
                creature_start_script(e, 1);
            } else {
                fdat_801ed8c8 = 5;
                creature_start_script(e, 1);
                fdat_801ed8c4 = 4;
            }
        }
    }
    if (fdat_801ed8c8 != 5) {
        cd_wait_idle();
        talk_load_image(r->unk09 * 6 + fdat_801ed8c8, 1);
    }
}

/*
 * The ending entry's handler: steps the ending stages in fdat_801ed8c4
 * (1 waiting for half the animation, 2 streaming, 3 stream done, 4 menu,
 * 5 done) by the XA stream state in the world block.
 */
ADDRESS(0x801ed63c, 0x284)
void map_run_shop_visit(void)
{
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    MenuScratch *menu = MENU_SCRATCH;

    e->unk1C.bytes[2] &= 0xf2;
    e->unk36 = -1;
    if (e->unkA4 == 0)
        return;
    if (fdat_801ed8c4 == 2 && xa_stream_finished()) {
        if (world_state.unk1EE16 == 3) {
            xa_stop_stream();
            world_state.unk1EE19 = 0;
            world_state.unk1EE16 = 4;
        }
    }
    if (world_state.unk1EE16 == 5) {
        fdat_801ed8c4 = 3;
        world_state.unk1EE16 = 0;
    }
    e->unk2A += e->unkB0;
    if ((short)e->unk2A >= 0x1000) {
        e->unk2A = 0;
        if (fdat_801ed8c4 == 2 && fdat_801ed8c8 == 0)
            creature_start_script(e, 1);
    }
    if (fdat_801ed8c4 == 1) {
        int ending = fdat_801ed8c8;
        if (ending != 0 || (short)e->unk2A >= 0x600) {
            viewer.unk1E4 = 0;
            xa_start_stream(r->unk09, ending);
            SsSetSerialVol(0, 0x40, 0x40);
            world_state.unk1EE16 = 2;
            world_state.unk1EE17 = 1;
            world_state.unk1EE19 = 0;
            fdat_801ed8c4 = 2;
            player_enter_frozen_state();
        }
    } else if (fdat_801ed8c4 == 3) {
        if (fdat_801ed8c8 == 1 || fdat_801ed8c8 == 2)
            fdat_801ed8c4 = 5;
        else
            fdat_801ed8c4 = 4;
    } else if (fdat_801ed8c4 == 4) {
        DAT_801cd180.unk4 = 0;
        menu->unk2DC = 1;
        menu->unk2E5 = 0;
        shop_run();
        fdat_801ed8c4 = 5;
    } else if (fdat_801ed8c4 == 5) {
        if (fdat_801ed8c8 == 2) {
            DAT_801cba08.unk1440.word &= 0x7fffffff;
            DAT_801cba08.unk1444.word |= 1;
        }
        DAT_801cd0e4 = 1;
    }
}
