/*
 * ST.EXE: the area module (WIP name): the area image fix-ups after a load,
 * the requests for a new area set, the loader's step setters and CD
 * completion callbacks, and the area load state machine, one step per call.
 * One translation unit: its string continues this module's constant pool
 * (0x800122a8).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/cd.h>
#include <st/data.h>
#include <st/effect.h>
#include <st/vis.h>
#include <st/menu.h>
#include <LIBCD.H>
#include <st/memory.h>
#include <st/anim.h>
#include <st/light.h>
#include <st/funcs.h>
#include <st/area.h>
#include <st/gfx.h>
#include <psyq/kernel.h>
#include <st/save.h>
#include <kf/lib/null.h>
#include <LIBSND.H>
#include <st/tim.h>

RODATA(0x80011f18, 0x39b)
SDATA(0x80088064, 0x38)

/* A tentative definition, so that ASPSX reaches it through $gp; st.player
 * allocates it. */
u8 *DAT_800881d8;

/* The empty area handler: every slot of the default table at 0x8008038c
 * (area_hooks_default) until an area's own handlers are loaded. */
ADDRESS(0x800443ec, 0x8)
void area_hook_none(void)
{
}

/* CD completion: marks the request done. */
ADDRESS(0x800443f4, 0x10)
void image_stream_set_ready(StCdRequest *request)
{
    request->status = 1;
}

/*
 * Reloads the area, then copies the images two areas patch in VRAM.
 */
ADDRESS(0x80044404, 0x270)
void area_reload_images(void)
{
    u32 flag;

    image_stream_queue(1, 481, image_stream_set_ready);
    area_state.loading = 1;
    area_state.load_step = 5;
    area_state.request_sound_bank = 0xff;
    area_state.unk14 = 0xff;
    area_state.request_images = area_state.loaded_area;
    area_load_update();
    cd_wait_idle();
    if (area_state.area == 24) {
        u32 marker;

        if (!((game_progress.event_flags.word >> 17) & 1))
            return;
        marker = (vis_lists.floor_kinds_off >> 3) & 1;
        if (marker != 1)
            return;
        {
            RECT a = {496, 0, 16, 64};
            RECT b = {480, 160, 16, 96};
            RECT c = {800, 224, 32, 32};
            RECT d = {512, 485, 16, 1};
            RECT e = {512, 486, 16, 1};
            RECT f = {0, 490, 256, 1};

            MoveImage(&a, 480, 0);
            MoveImage(&b, 480, 64);
            MoveImage(&c, 768, 224);
            MoveImage(&d, 512, 483);
            MoveImage(&e, 512, 484);
            MoveImage(&f, 0, 490);
        }
    } else if (area_state.area == 27) {
        flag = (game_progress.event_flags.word >> 13) & 1;
        if (flag != 1)
            return;
        if (((vis_lists.floor_kinds_off >> 4) & 1) != flag)
            return;
        {
            RECT g = {0, 487, 256, 1};

            MoveImage(&g, 0, 489);
        }
    }
}

/*
 * Game start: clears the area request, registers the archives, finds the
 * XA streams, and copies the members of the 0x1e0 table pack (FDAT archive)
 * to their tables.
 */
ADDRESS(0x80044674, 0x874)
void game_load_common_data(void)
{
    CdlFILE file;
    u32 *p = (u32 *)model_heap;

    area_state.loaded_world = area_state.loaded_images = area_state.loaded_sound_bank = 255;
    area_state.loading = 0;
    area_state.loaded_area = 0;
    area_state.loaded_world = 0;
    area_state.loaded_images = 0;
    area_state.loaded_sound_bank = 0;
    area_state.unk8 = 0;
    area_state.area = 0;
    area_state.load_step = 0;
    vis_table_a.object_sequence_state = 0;
    DAT_800881d8 = overlay_area_start + 4;
    DAT_800881dc[0] = overlay_area_start;
    cd_archive_open(2, "CHR0\\M00.T");
    cd_archive_open(0, "COM\\MO.T");
    cd_archive_open(1, "COM\\FDAT.T");
    cd_archive_open(3, "COM\\STAT.T");
    cd_archive_open(4, "COM\\CRT.T");
    cd_archive_open(6, "COM\\ICON.T");
    cd_archive_open(5, "COM\\ITEM.T");
    cd_archive_open(7, "COM\\EQUIP.T");
    cd_archive_open(8, "COM\\TALKMSG.T");
    CdSearchFile(&file, "\\ST\\XA0\\HAM.XA;1");
    game_progress.xa_positions[0] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\SHOP0.XA;1");
    game_progress.xa_positions[1] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\SHOP1.XA;1");
    game_progress.xa_positions[2] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\SHOP2.XA;1");
    game_progress.xa_positions[3] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\SHOP3.XA;1");
    game_progress.xa_positions[4] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\03100.XA;1");
    game_progress.xa_positions[5] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\19010.XA;1");
    game_progress.xa_positions[6] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\30200.XA;1");
    game_progress.xa_positions[7] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\07900.XA;1");
    game_progress.xa_positions[8] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\10300.XA;1");
    game_progress.xa_positions[9] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\01300.XA;1");
    game_progress.xa_positions[10] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\04601.XA;1");
    game_progress.xa_positions[11] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\04400.XA;1");
    game_progress.xa_positions[12] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\09502.XA;1");
    game_progress.xa_positions[13] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\22700.XA;1");
    game_progress.xa_positions[14] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\10200.XA;1");
    game_progress.xa_positions[15] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\03000.XA;1");
    game_progress.xa_positions[16] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\09506.XA;1");
    game_progress.xa_positions[17] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\30400.XA;1");
    game_progress.xa_positions[18] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\10100.XA;1");
    game_progress.xa_positions[19] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\02400.XA;1");
    game_progress.xa_positions[20] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\10301.XA;1");
    game_progress.xa_positions[21] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\12300.XA;1");
    game_progress.xa_positions[22] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\22900.XA;1");
    game_progress.xa_positions[23] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\09503.XA;1");
    game_progress.xa_positions[24] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\10800.XA;1");
    game_progress.xa_positions[25] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\09504.XA;1");
    game_progress.xa_positions[26] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\11100.XA;1");
    game_progress.xa_positions[27] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\00100.XA;1");
    game_progress.xa_positions[28] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\02500.XA;1");
    game_progress.xa_positions[29] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\07700.XA;1");
    game_progress.xa_positions[30] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\10900.XA;1");
    game_progress.xa_positions[31] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\08100.XA;1");
    game_progress.xa_positions[32] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\09505.XA;1");
    game_progress.xa_positions[33] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\10302.XA;1");
    game_progress.xa_positions[34] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\20400.XA;1");
    game_progress.xa_positions[35] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\13900.XA;1");
    game_progress.xa_positions[36] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\01100.XA;1");
    game_progress.xa_positions[37] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\29500.XA;1");
    game_progress.xa_positions[38] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\30600.XA;1");
    game_progress.xa_positions[39] = file.pos;
    cd_read_member(1, 0x1e0, (u_long *)model_heap);
    image_stream_queue(1, 0x1e1, image_stream_set_ready);
    sound_bank_load(1, 0x1e2, 0);
    copy_words((u32 *)&vis_table_b, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    copy_words((u32 *)equipment_model_placements, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    copy_words((u32 *)cd_file_buffer, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    copy_words((u32 *)effects.defs, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    copy_words((u32 *)lighting_defaults, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    copy_words((u32 *)sound_state.sounds, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    copy_words((u32 *)&menu_messages, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    copy_words((u32 *)vis_table_c.types, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    p = (u32 *)((u8 *)p + (*p + 4));
    copy_words((u32 *)common_anim_pack, p + 1, *p >> 2);
    anim_register_pack(40, common_anim_pack);
    cd_wait_idle();
    player_init();
    item_model_slots_clear();
}

/*
 * Loading the current area set again and waiting for it, and moving the
 * viewer.
 */

/* Requests the current areas again (99 marks each unloaded) and runs the
 * loader until the request is done, then resets the primitive buffers. */
ADDRESS(0x80044ee8, 0x128)
void area_load_current_blocking(void)
{
    area_state.request_area = area_state.loaded_area;
    area_state.request_world = area_state.loaded_world;
    area_state.request_images = area_state.loaded_images;
    area_state.request_sound_bank = area_state.loaded_sound_bank;
    area_state.unk14 = area_state.unk8;
    area_state.loading = 1;
    area_state.loaded_area = 99;
    area_state.loaded_world = 99;
    area_state.loaded_images = 99;
    area_state.loaded_sound_bank = 99;
    area_state.unk8 = 99;
    area_state.load_step = 0;
    area_state.shift_x = 127;
    area_state.shift_z = 127;
    area_state.shift_y = 127;
    area_state.skip_model_sync = 0;
    do {
        cd_wait_frame();
        area_load_update();
        if (area_state.loading == 0)
            break;
    } while ((s16)area_state.load_step < 6 || (s16)area_state.load_step == 240);
    cd_wait_idle();
    world_state.area_geometry[0] = area_world_buffer;
    world_state.area_geometry[1] = area_collision_buffer;
}

ADDRESS(0x80045010, 0x34)
void player_shift_position(int dx, int dy, int dz)
{
    viewer.position.vx += dx;
    viewer.position.vz += dz;
    viewer.position.vy += dy;
}

/*
 * Requesting a new area set (five area ids, 255 keeps one) with its screen
 * fade.
 */

/* Starts loading areas a0..a4 unless they are already loaded or being
 * loaded. f0..f5 shift the viewer once the new set is in: x by
 * f0 * 4096 + f3 * 128, y by f2 * 4096 + f4 * 128 and z by
 * f1 * 4096 + f5 * 128 (player_shift_position); f0 = 127 shifts nothing,
 * and such a request while a shift is in force negates f0..f2. */
/* All nine direct callers discard the word result; the return edges keep
 * v0 live. */
ADDRESS(0x80045044, 0x524)
KF_VALUELESS_S32 area_request_load(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, s8 f0, s8 f1, s8 f2, u8 f3, u8 f4,
                   u8 f5)
{
    u8 n0, n1, n2, n3, n4;
    u8 m0, m1, m2, m3, m4;

    if (a0 == 0xff) {
        n0 = area_state.loaded_area;
        m0 = area_state.request_area;
        if (a1 == 0xff) {
            n1 = area_state.loaded_world;
            m1 = area_state.request_world;
        } else {
            n1 = m1 = a1;
        }
        if (a2 == 0xff) {
            n2 = area_state.loaded_images;
            m2 = area_state.request_images;
        } else {
            n2 = m2 = a2;
        }
        if (a3 == 0xff) {
            n3 = area_state.loaded_sound_bank;
            m3 = area_state.request_sound_bank;
        } else {
            n3 = m3 = a3;
        }
        if (a4 == 0xff) {
            n4 = area_state.unk8;
            m4 = area_state.unk14;
        } else {
            n4 = m4 = a4;
        }
    } else {
        m1 = a1;
        m4 = a4;
        m0 = a0;
        n0 = a0;
        n1 = area_state.loaded_world;
        n2 = area_state.loaded_images;
        n3 = area_state.loaded_sound_bank;
        n4 = area_state.unk8;
        m2 = a2;
        m3 = a3;
    }
    if (area_state.loading != 0)
        goto busy;
    if (area_state.loaded_area == n0 && area_state.loaded_world == n1 && area_state.loaded_images == n2 &&
        area_state.loaded_sound_bank == n3 && area_state.unk8 == n4)
        return;
start:
    if (area_state.loaded_area != 99 && area_state.loaded_area != n0)
        area_save_snapshot(area_state.loaded_area);
    area_state.loading = 1;
    area_state.load_step = 0;
    area_state.request_area = a0;
    area_state.request_world = a1;
    area_state.request_images = a2;
    area_state.request_sound_bank = a3;
    area_state.unk14 = a4;
    area_state.shift_x = f0;
    area_state.shift_z = f1;
    area_state.shift_y = f2;
    area_state.shift_x_fine = f3;
    area_state.shift_y_fine = f4;
    area_state.shift_z_fine = f5;
    area_state.shift_applied = 0;
    if (a0 < 44)
        area_state.entry_fade_start = cd_state.vsync_count;
    else
        area_state.entry_fade_start = -1;
    if (a1 == 0xff) {
        area_state.loading_world = 0;
    } else {
        area_state.loading_world = 1;
        area_state.skip_model_sync = 1;
    }
    return;
busy:
    if ((area_state.loading != 1 || area_state.request_area == m0) && area_state.request_world == m1 &&
        area_state.request_images == m2 && area_state.request_sound_bank == m3 && area_state.unk14 == m4)
        return;
    if (area_state.request_area == 0xff && area_state.loaded_area == n0)
        return;
    if (area_state.request_world == 0xff && area_state.loaded_world == a1)
        return;
    if (area_state.request_images == 0xff && area_state.loaded_images == a2)
        return;
    if (area_state.request_sound_bank == 0xff && area_state.loaded_sound_bank == a3)
        return;
    if (area_state.unk14 == 0xff && area_state.unk8 == a4)
        return;
    if ((area_state.request_area != 0xff && a0 == 0xff) || (area_state.request_world != 0xff && a1 == 0xff) ||
        (area_state.request_images != 0xff && a2 == 0xff) || (area_state.request_sound_bank != 0xff && a3 == 0xff) ||
        (area_state.unk14 != 0xff && a4 == 0xff)) {
        cd_wait_idle();
        while (area_state.loading) {
            cd_wait_frame();
            area_load_update();
        }
    } else {
        while ((s16)area_state.load_step == 240)
            cd_wait_frame();
    }
    ExitCriticalSection();
    if (area_state.shift_applied && a0 != 0xff && f0 == 127 && area_state.request_area != a0) {
        f0 = -area_state.shift_x;
        f2 = -area_state.shift_y;
        f1 = -area_state.shift_z;
    }

    goto start;
}

/*
 * The area loader's step setters and CD completion callbacks, and the
 * commit of a finished area request.
 */

ADDRESS(0x80045568, 0x14)
void func_80045568(void)
{
    area_state.load_step = 0;
}

ADDRESS(0x8004557c, 0x18)
void area_load_records_done(void)
{
    area_state.load_step = 1;
}

ADDRESS(0x80045594, 0x18)
void area_load_overlay_done(void)
{
    area_state.load_step = 2;
}

ADDRESS(0x800455ac, 0x18)
void area_load_tables_done(void)
{
    area_state.load_step = 3;
}

ADDRESS(0x800455c4, 0x18)
void area_load_world_done(void)
{
    area_state.load_step = 4;
}

ADDRESS(0x800455dc, 0x18)
void area_load_collision_done(void)
{
    area_state.load_step = 5;
}

ADDRESS(0x800455f4, 0x54)
void area_load_images_done(StCdRequest *request)
{
    if (request->status == 0) {
        request->status = 1;
    } else if (request->status == 2) {
        area_state.load_step = 6;
        sound_state.unkC = 0;
        area_state.unk26 = game_settings.bgm_volume;
    }
}

ADDRESS(0x80045648, 0x1c)
void area_stream_set_ready(StCdRequest *request)
{
    if (request->status == 0)
        request->status = 1;
}

ADDRESS(0x80045664, 0x18)
void func_80045664(void)
{
    area_state.load_step = 8;
}

/* Commits the requested area numbers that are set (0xff keeps one). */
ADDRESS(0x8004567c, 0x7c)
void area_request_commit(void)
{
    vis_table_a.object_sequence_state = 0;
    area_state.tables_loaded = 0;
    if (area_state.request_area != 0xff)
        area_state.loaded_area = area_state.request_area;
    if (area_state.request_world != 0xff)
        area_state.loaded_world = area_state.request_world;
    if (area_state.request_images != 0xff)
        area_state.loaded_images = area_state.request_images;
    if (area_state.request_sound_bank != 0xff)
        area_state.loaded_sound_bank = area_state.request_sound_bank;
    if (area_state.unk14 != 0xff)
        area_state.unk8 = area_state.unk14;
    area_state.loading = 0;
}

/*
 * The area load state machine: the area's script and tables, its character
 * file table, the object tables and sound banks.
 */

ADDRESS(0x800456f8, 0x898)
void area_load_update(void)
{
    Creature *a;
    AreaObject *b;
    GroundItem *c;
    StVabSlot *slot;
    u_long *p = cd_file_buffer;
    int i, j;

    /* Dead: phase 3 starts the cursor over. What survives is the buffer
     * address's high half, which phases 1 and 2 reuse from $s2. */
    p++;
    if (area_state.loading == 0)
        return;
    if (area_state.loading != 1)
        return;
    switch ((s16)area_state.load_step) {
    case 0:
        if (area_state.request_area == 0xff)
            goto phase3;
        if (cd_archive_member_size(1, area_state.request_area * 10))
            image_stream_queue(1, area_state.request_area * 10, area_stream_set_ready);
        area_state.load_step = 240;
        b = vis_table_b.entries;
        for (i = 239; i != -1; i--) {
            if (b->kind == 236 && b->kind_data.snd.voice != -1) {
                sound_stop(b->kind_data.snd.voice, b->params.snd.sound + 1108);
                b->kind_data.snd.voice = -1;
            }
            b->kind = 0xff;
            b++;
        }
        cd_queue_read_member(1, (area_state.request_area * 10) | 1, (u8 *)cd_file_buffer, area_load_records_done);
        break;
    case 1:
        area_state.load_step = 240;
        area_state.hooks = &area_hooks_default;
        cd_queue_read_member(1, area_state.request_area * 10 + 2, DAT_800881dc[0], area_load_overlay_done);
        viewer.block_record = NULL;
        viewer.block = -1;
        copy_words((u32 *)vis_records, (u32 *)cd_file_buffer, 0x3e00);
        if ((s8)area_state.shift_x != 127) {
            area_state.shift_applied = 1;
            player_shift_position((s8)area_state.shift_x * 4096 + (s8)area_state.shift_x_fine * 128,
                          (s8)area_state.shift_y * 4096 + (s8)area_state.shift_y_fine * 128,
                          (s8)area_state.shift_z * 4096 + (s8)area_state.shift_z_fine * 128);
        }
        area_state.area = area_state.request_area;
        break;
    case 2:
        area_state.load_step = 240;
        cd_queue_read_member(1, area_state.request_area * 10 + 3, (u8 *)cd_file_buffer, area_load_tables_done);
        break;
    case 3:
    phase3:
        if (area_state.request_world != 0xff) {
            char name[] = "CHRX\\MXX.T";

            mem_free(cd_state.archives[2].sectors);
            cd_state.archives[2].sectors = NULL;
            name[3] = area_state.request_area / 10 + '0';
            name[6] = area_state.request_area / 10 + '0';
            name[7] = area_state.request_area % 10 + '0';
            cd_archive_open(2, name);
            area_state.load_step = 240;
            cd_queue_read_member(1, area_state.request_world * 10 + 4, (u8 *)area_world_buffer, area_load_world_done);
        }
        if (area_state.request_area != 0xff) {
            p = cd_file_buffer;
            a = vis_table_a.entries;
            for (i = 63; i != -1; i--) {
                if (a->anim_slot.slot) {
                    anim_slot_free(a->anim_slot.slot);
                    if (a->anim_slot.slot)
                        anim_set_free(a->anim_slot.set);
                }
                a->spawn_chance = 0xff;
                a++;
            }
            b = vis_table_b.entries;
            for (i = 239; i != -1; i--) {
                if (b->anim_slot.slot) {
                    anim_slot_free(b->anim_slot.slot);
                    if (b->anim_slot.slot)
                        anim_set_free(b->anim_slot.set);
                }
                b->type = 0xffff;
                b++;
            }
            c = vis_table_c.entries;
            for (i = 63; i != -1; i--) {
                if (c->anim_slot.slot) {
                    anim_slot_free(c->anim_slot.slot);
                    if (c->anim_slot.slot)
                        anim_set_free(c->anim_slot.set);
                }
                c++;
            }
            item_model_slots_clear();
            for (i = 15; i != -1; i--) {
                if (world_state.model_files[128 + i])
                    heap_free(world_state.model_files[128 + i]);
            }
            for (i = 127; i != -1; i--) {
                if (world_state.model_files[588 + i])
                    heap_free(world_state.model_files[588 + i]);
            }
            copy_words((u32 *)vis_table_a.types, (u32 *)(p + 1), 2816);
            creature_types_link_scripts();
            p = (u_long *)((u8 *)p + (*p + 4));
            creatures_load((CreatureFileEntry *)(p + 1));
            p = (u_long *)((u8 *)p + (*p + 4));
            copy_words((u32 *)&vis_table_b.types[300], (u32 *)(p + 1), 192);
            p = (u_long *)((u8 *)p + (*p + 4));
            objects_load((u8 *)(p + 1));
            p = (u_long *)((u8 *)p + (*p + 4));
            ground_items_load((GroundItemFileEntry *)(p + 1));
            p = (u_long *)((u8 *)p + (*p + 4));
            placements_expand((StPlacement *)(p + 1));
            area_restore_snapshot(area_state.request_area);
            game_progress.areas_visited[area_state.request_area] |= 1;
            area_state.hooks = (AreaHooks *)DAT_800881d8;
            texanim_clear_all();
            area_state.hooks->loaded();
            vis_table_a.zone = 254;
            vis_table_a.zone_history[3] = 0xff;
            vis_table_a.zone_history[2] = 0xff;
            vis_table_a.zone_history[1] = 0xff;
            vis_table_a.zone_history[0] = 0xff;
            slot = &sound_state.slots[8];
            for (j = 15; j != -1; j--) {
                if (slot->block) {
                    while (slot->block[2] == 3)
                        cd_wait_frame();
                    SsVabClose(slot->vab);
                    slot->vab = -1;
                    slot->block = NULL;
                }
                slot++;
            }
            viewer.update_all_spawns = 1;
            area_state.tables_loaded = 1;
        }
        if (area_state.request_world == 0xff)
            goto phase5;
        break;
    case 4:
        if (area_state.request_world != 0xff) {
            area_state.load_step = 240;
            cd_queue_read_member(1, area_state.request_world * 10 + 5, (u8 *)area_collision_buffer, area_load_collision_done);
        }
        break;
    case 5:
    phase5:
        if (area_state.request_images != 0xff) {
            area_state.load_step = 240;
            image_stream_queue(1, area_state.request_images * 10 + 6, area_load_images_done);
        }
        cd_wait_idle();
        area_state.loading_world = 0;
        area_state.skip_model_sync = 0;
        if (area_state.request_sound_bank == 0xff && area_state.unk14 == 0xff) {
            area_state.load_step = 7;
            goto done;
        }
        area_state.load_step = 6;
        /* fall through */
    case 6: {
        StVabSlot *bgm = &sound_state.slots[1];

        if (bgm->vab != -1) {
            SsVabClose(bgm->vab);
            bgm->vab = -1;
            bgm->block = NULL;
        }
    }
        if (area_state.request_sound_bank != 0xff) {
            area_state.load_step = 240;
            sound_bank_load(1, area_state.request_sound_bank * 10 + 7, 1);
        }
        if (area_state.unk14 == 0xff)
            goto done;
        area_state.load_step = 7;
        break;
    case 7:
    done:
        area_request_commit();
        break;
    case 8:
        area_request_commit();
        break;
    case 240:
        /* a CD read is pending */
        break;
    }
}
