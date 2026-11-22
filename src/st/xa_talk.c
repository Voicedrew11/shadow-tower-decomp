/*
 * ST.EXE: XA voice stream wait and stop, the still images shown with the
 * streams, and the table A entry talk sequence (WIP module,
 * 0x800541f4..0x800549c8; xa_ready_callback/xa_start_stream open the file in
 * src/st/xa.c). -G8: the store to the 2-byte extern talk_stream_file is the
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

    pos[1] = cd_loc_to_sector((CdlLOC *)&viewer.xa_pos);
    pos[0] = cd_loc_to_sector(&viewer.xa_start);
    if (viewer.xa_start_tick + 3600 < cd_state.unk4)
        return 1;
    return pos[1] - pos[0] >= viewer.xa_length * 2;
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
    cd_read_member(8, id, cd_file_buffer);
    tim_load_images_at(cd_file_buffer, &pix, &clut, 1);
}

/* An entry's talk: its 0x70 animation fades unk2A in, the XA stream of its
 * record plays, then the 0x72 animation fades out. `fixed` runs 24-frame
 * fades without animations. */
ADDRESS(0x80054368, 0x4f0)
void creature_talk(Creature *e, int fixed, int arg2)
{
    CreatureType *r = &vis_table_a.types[e->type];
    CreatureTalkScript *a;
    int i;

    if (!(e->flags & 0x2000000))
        return;
    if (!fixed) {
        if (e->fade != -1)
            return;
        a = (CreatureTalkScript *)creature_type_find_script(r, 0x70);
        if (a == NULL)
            return;
        e->anim = a->anim;
        e->anim_step = a->anim_step;
        if (e->anim_step > 0)
            e->anim_frame = 0;
        else
            e->anim_frame = 4096;
        world_state.unk1EE18 = 4096 / a->anim_step;
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
            e->anim_frame += e->anim_step;
            if ((e->anim_step > 0 && (short)e->anim_frame >= 4096) ||
                (e->anim_step < 0 && (short)e->anim_frame < 0)) {
                e->anim_frame = 4096;
                break;
            }
        } else if (++i == world_state.unk1EE18) {
            break;
        }
        world_run_frame(NULL, NULL, 0);
    }
    if (!fixed && (a = (CreatureTalkScript *)creature_type_find_script(r, 0x71)) != NULL) {
        e->anim = a->anim;
        e->anim_frame = 0;
        e->anim_step = a->anim_step;
    }
    if (game_settings.subtitles == 1)
        talk_load_image(r->talk_id * 6 + game_progress.talk_counts[r->talk_id], 0);
    talk_stream_file = r->talk_id;
    i = 0;
    xa_start_stream(r->talk_id, game_progress.talk_counts[r->talk_id]);
    game_progress.talk_counts[r->talk_id]++;
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
            e->anim_frame += e->anim_step;
            if ((short)e->anim_frame >= 4096)
                e->anim_frame = 0;
        }
        world_run_frame(NULL, NULL, 0);
    } while (world_state.unk1EE16 != 5);
    if (!fixed) {
        a = (CreatureTalkScript *)creature_type_find_script(r, 0x72);
        if (a != NULL) {
            e->anim = a->anim;
            e->anim_step = a->anim_step;
            if (e->anim_step > 0)
                e->anim_frame = 0;
            else
                e->anim_frame = 4096;
        }
        world_state.unk1EE18 = 4096 / abs(a->anim_step);
    } else {
        world_state.unk1EE18 = 24;
    }
    world_state.unk1EE16 = 6;
    world_state.unk1EE19 = 0;
    while (1) {
        if (!fixed) {
            e->anim_frame += e->anim_step;
            if (e->anim_step > 0 && (short)e->anim_frame >= 4096) {
                e->anim_frame = 4096;
                break;
            }
            if (e->anim_step < 0 && (short)e->anim_frame < 0) {
                e->anim_frame = 0;
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
        if (pad_state.pressed) {
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
