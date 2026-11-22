/*
 * ST.EXE: XA voice stream wait and stop, the still images shown with the
 * streams, and the table A entry talk sequence (WIP module,
 * 0x800541f4..0x800549c8; xa_ready_callback/xa_start_stream open the file in
 * src/st/xa.c). -G8: the store to the 2-byte extern DAT_80088190 is the
 * assembler's lui $at form.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <LIBSND.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/hud.h>
#include <st/text.h>
#include <st/funcs.h>
#include <st/pad.h>
#include <st/xa.h>

/* Nonzero once the stream has run its length (or 3600 ticks). */
ADDRESS(0x800541f4, 0x84)
int xa_stream_finished(void)
{
    /* Retail keeps both positions in stack slots and reloads them. */
    volatile int pos[2];

    pos[1] = cd_loc_to_sector((CdlLOC *)&viewer.unk1DC);
    pos[0] = cd_loc_to_sector(&viewer.unk1D8);
    if (viewer.unk1E8 + 3600 < cd_state.unk4)
        return 1;
    return pos[1] - pos[0] >= viewer.unk1E0 * 2;
}

ADDRESS(0x80054278, 0x58)
void xa_stop_stream(void)
{
    u8 param[8];

    param[0] = CdlModeSpeed | CdlModeSize1;
    CdControlB(CdlSetmode, param, NULL);
    CdReadyCallback(NULL);
    SsSetSerialVol(0, 0, 0);
    CdControl(CdlPause, NULL, NULL);
}

/* Loads still image `id` (archive 8) into VRAM. */
ADDRESS(0x800542d0, 0x98)
void talk_load_image(int id, int full)
{
    DVECTOR pix;
    DVECTOR clut;

    cd_wait_idle();
    if (full == 0) {
        pix.vx = 960;
        pix.vy = 256;
        clut.vx = 560;
        clut.vy = 495;
    } else {
        pix.vx = 960;
        pix.vy = 0;
        clut.vx = 0;
        clut.vy = 511;
    }
    cd_read_member(8, id, DAT_80088c88);
    tim_load_images_at(DAT_80088c88, &pix, &clut, 1);
}

/* An entry's talk: its 0x70 animation fades unk2A in, the XA stream of its
 * record plays, then the 0x72 animation fades out. `fixed` runs 24-frame
 * fades without animations. */
ADDRESS(0x80054368, 0x4f0)
void creature_talk(Vis212 *e, int fixed, int arg2)
{
    Vis192 *r = &vis_table_a.unk3500[e->unk02];
    VisAnim *a;
    int i;

    if (!(e->unk70 & 0x2000000))
        return;
    if (!fixed) {
        if (e->unk36 != -1)
            return;
        a = (VisAnim *)creature_type_find_script(r, 0x70);
        if (a == NULL)
            return;
        e->unk08 = a->unk1;
        e->unkB0 = a->unkC;
        if (e->unkB0 > 0)
            e->unk2A = 0;
        else
            e->unk2A = 4096;
        world_state.unk1EE18 = 4096 / a->unkC;
    } else {
        world_state.unk1EE18 = 24;
    }
    world_state.unk1EE16 = 1;
    if (arg2)
        world_state.unk1EE17 = 3;
    else
        world_state.unk1EE17 = 0;
    world_state.unk1EE19 = 0;
    i = 0;
    for (;;) {
        if (!fixed) {
            e->unk2A += e->unkB0;
            if ((e->unkB0 > 0 && (short)e->unk2A >= 4096) ||
                (e->unkB0 < 0 && (short)e->unk2A < 0)) {
                e->unk2A = 4096;
                break;
            }
        } else if (++i == world_state.unk1EE18) {
            break;
        }
        world_run_frame(NULL, NULL, 0);
    }
    if (!fixed && (a = (VisAnim *)creature_type_find_script(r, 0x71)) != NULL) {
        e->unk08 = a->unk1;
        e->unk2A = 0;
        e->unkB0 = a->unkC;
    }
    if (DAT_801cd120.unk4 == 1)
        talk_load_image(r->unk09 * 6 + DAT_801cba08.unk1448[r->unk09], 0);
    DAT_80088190 = r->unk09;
    i = 0;
    xa_start_stream(r->unk09, DAT_801cba08.unk1448[r->unk09]);
    DAT_801cba08.unk1448[r->unk09]++;
    SsSetSerialVol(0, 64, 64);
    world_state.unk1EE16 = 2;
    world_state.unk1EE19 = 0;
    do {
        if (xa_stream_finished()) {
            xa_stop_stream();
            if (world_state.unk1EE16 == 3)
                world_state.unk1EE19 = 0;
            world_state.unk1EE16 = 4;
        }
        if (!fixed) {
            e->unk2A += e->unkB0;
            if ((short)e->unk2A >= 4096)
                e->unk2A = 0;
        }
        world_run_frame(NULL, NULL, 0);
    } while (world_state.unk1EE16 != 5);
    if (!fixed) {
        a = (VisAnim *)creature_type_find_script(r, 0x72);
        if (a != NULL) {
            e->unk08 = a->unk1;
            e->unkB0 = a->unkC;
            if (e->unkB0 > 0)
                e->unk2A = 0;
            else
                e->unk2A = 4096;
        }
        world_state.unk1EE18 = 4096 / abs(a->unkC);
    } else {
        world_state.unk1EE18 = 24;
    }
    world_state.unk1EE16 = 6;
    world_state.unk1EE19 = 0;
    while (1) {
        if (!fixed) {
            e->unk2A += e->unkB0;
            if (e->unkB0 > 0 && (short)e->unk2A >= 4096) {
                e->unk2A = 4096;
                break;
            }
            if (e->unkB0 < 0 && (short)e->unk2A < 0) {
                e->unk2A = 0;
                break;
            }
        } else if (++i == world_state.unk1EE18) {
            break;
        }
        world_run_frame(NULL, NULL, 0);
    }
    world_state.unk1EE16 = 0;
    pad_clear_input();
}

/* Shows still image 300 + id until a button is pressed. */
ADDRESS(0x80054858, 0x170)
void talk_show_message(int id)
{
    int n;

    world_state.unk1EE18 = 16;
    world_state.unk1EE16 = 1;
    world_state.unk1EE17 = 2;
    world_state.unk1EE19 = 0;
    n = 16;
    while (n--)
        world_run_frame(NULL, NULL, 0);
    talk_load_image(id + 300, 0);
    world_state.unk1EE16 = 2;
    world_state.unk1EE19 = 0;
    pad_clear_input();
    do {
        if (DAT_801cd180.unk4) {
            if (world_state.unk1EE16 == 3)
                world_state.unk1EE19 = 0;
            world_state.unk1EE16 = 4;
        }
        world_run_frame(NULL, NULL, 0);
    } while (world_state.unk1EE16 != 5);
    world_state.unk1EE18 = 16;
    world_state.unk1EE16 = 6;
    world_state.unk1EE19 = 0;
    n = 16;
    while (n--)
        world_run_frame(NULL, NULL, 0);
    world_state.unk1EE16 = 0;
    world_state.unk1EE17 = 0;
    pad_clear_input();
}
