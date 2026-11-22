/*
 * ST.EXE: the save-data image writer and reader and the area's per-frame
 * hook (WIP module, 0x800562d4..0x8005738c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBETC.H>
#include <psyq/libc.h>
#include <st/memory.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/save.h>

#define BIT(i) (1 << ((i) % 32))

/* Fills the save image; `fresh` stores the start position and no area. */
ADDRESS(0x800562d4, 0x7cc)
void save_pack_data(StSaveData *save, int fresh)
{
    int i;
    Vis48 *e32;
    Vis48 *e48;
    Vis44 *r;

    if (fresh)
        save->unk4[0] = save->unk4[1] = save->unk4[2] = save->unk4[3] = save->unk4[4] = 0;
    else
        *(StBytes5 *)save->unk4 = ((StAreaBytes *)&area_state)->unk4;
    copy_words(save->unkC, game_progress.unk0[0], 0x60);
    copy_words(save->unk18C, game_progress.unk180[0], 0x30);
    copy_words(save->unk24C, game_progress.unk240[0], 0x180);
    copy_words(save->unk84C, game_progress.unk840[0], 0x180);
    copy_words(save->unkE4C, game_progress.unkE40[0], 0x180);
    copy_words(save->unk144C, &game_progress.unk1440.word, 2);
    copy_words(save->unk1454, (u32 *)game_progress.unk1448, 12);
    copy_words(save->unk1484, (u32 *)game_progress.unk1478, 12);
    copy_words(save->unk14B4, game_progress.unk14A8, 5);
    save->unk14C8[0] = vis_table_c.unk0000[2];
    save->unk14C8[1] = vis_table_c.unk0000[3];
    save->unk14C8[2] = vis_table_c.unk0000[4];
    save->unk14C8[3] = vis_table_c.unk0000[5];
    copy_words(save->unk14CC, vis_table_c.unk0070, 9);
    e32 = &vis_table_c.entries[32];
    for (i = 0; i < 16; i++, e32++) {
        if (i < vis_table_c.unk0000[2]) {
            save->unk14F0[i].unk00 = e32->unk00;
            save->unk14F0[i].unk02 = e32->unk02;
            save->unk14F0[i].unk04[0] = e32->unk08.vx;
            save->unk14F0[i].unk04[1] = e32->unk08.vy;
            save->unk14F0[i].unk04[2] = e32->unk08.vz;
            save->unk14F0[i].unk10[0] = e32->unk18;
            save->unk14F0[i].unk10[1] = e32->unk1A;
            save->unk14F0[i].unk10[2] = e32->unk1C;
            save->unk14F0[i].unk16 = e32->unk22;
            save->unk14F0[i].unk17 = e32->unk2c;
        }
    }
    e48 = &vis_table_c.entries[48];
    for (i = 0; i < 16; i++, e48++) {
        if (i < vis_table_c.unk0000[3]) {
            save->unk14F0[16 + i].unk00 = e48->unk00;
            save->unk14F0[16 + i].unk02 = e48->unk02;
            save->unk14F0[16 + i].unk04[0] = e48->unk08.vx;
            save->unk14F0[16 + i].unk04[1] = e48->unk08.vy;
            save->unk14F0[16 + i].unk04[2] = e48->unk08.vz;
            save->unk14F0[16 + i].unk10[0] = e48->unk18;
            save->unk14F0[16 + i].unk10[1] = e48->unk1A;
            save->unk14F0[16 + i].unk10[2] = e48->unk1C;
            save->unk14F0[16 + i].unk16 = e48->unk22;
            save->unk14F0[16 + i].unk17 = e48->unk2c;
        }
    }
    save->unk17F0 = viewer.unk2C;
    save->unk17F4 = viewer.unk34;
    save->unk17F8 = viewer.unk2;
    save->unk17FA = viewer.unk6;
    save->unk17FC = viewer.unk8;
    save->unk17FE = viewer.unk30;
    save->unk1800 = viewer.unk26;
    save->unk1801 = viewer.unk27;
    save->unk1802 = viewer.unk28;
    save->unk1803 = viewer.unk32;
    copy_halfwords((u16 *)save->unk1804, (u16 *)viewer.unk62, 14);
    copy_halfwords((u16 *)save->unk1820, (u16 *)viewer.unk7E, 14);
    copy_halfwords(save->unk183C, viewer.unk9A, 14);
    copy_words(save->unk1870, (u32 *)&viewer.unk22C, 3);
    save->unk187C[0][0] = viewer.unk190[0][0];
    save->unk187C[0][1] = viewer.unk190[0][1];
    save->unk187C[1][0] = viewer.unk190[1][0];
    save->unk187C[1][1] = viewer.unk190[1][1];
    save->unk187C[2][0] = viewer.unk190[2][0];
    save->unk187C[2][1] = viewer.unk190[2][1];
    save->unk187C[3][0] = viewer.unk190[3][0];
    save->unk187C[3][1] = viewer.unk190[3][1];
    save->unk187C[4][0] = viewer.unk190[4][0];
    save->unk187C[4][1] = viewer.unk190[4][1];
    save->unk187C[5][0] = viewer.unk190[5][0];
    save->unk187C[5][1] = viewer.unk190[5][1];
    save->unk187C[6][0] = viewer.unk190[6][0];
    save->unk187C[6][1] = viewer.unk190[6][1];
    save->unk187C[7][0] = viewer.unk190[7][0];
    save->unk187C[7][1] = viewer.unk190[7][1];
    save->unk187C[8][0] = viewer.unk190[8][0];
    save->unk187C[8][1] = viewer.unk190[8][1];
    save->unk187C[9][0] = viewer.unk190[9][0];
    save->unk187C[9][1] = viewer.unk190[9][1];
    save->unk187C[10][0] = viewer.unk190[10][0];
    save->unk187C[10][1] = viewer.unk190[10][1];
    save->unk187C[11][0] = viewer.unk190[11][0];
    save->unk187C[11][1] = viewer.unk190[11][1];
    save->unk187C[12][0] = viewer.unk190[12][0];
    save->unk187C[12][1] = viewer.unk190[12][1];
    save->unk18B0[0] = viewer.unk219;
    save->unk18B0[1] = viewer.unk21A;
    save->unk18B0[2] = viewer.unk21B;
    if (fresh) {
        save->unk1858.vx = 0x3d000;
        save->unk1858.vy = 0x405c0;
        save->unk1858.vz = 0x3f000;
        save->unk1868.vx = 0;
        save->unk1868.vy = 3072;
        save->unk1868.vz = 0;
    } else {
        save->unk1858 = *(VECTOR *)&viewer.position.vx;
        save->unk1868 = viewer.unk278;
    }
    save->unk18B4[0] = game_settings.unk0;
    save->unk18B4[1] = game_settings.unk1;
    save->unk18B4[2] = game_settings.unk2;
    save->unk18B4[3] = game_settings.unk3;
    save->unk18B4[4] = game_settings.unk4;
    save->unk18B4[5] = game_settings.unk5;
    if (game_progress.unk16E0 == -1) {
        /* A save id from the vsync counter (low half) and rand() (high). */
        ((u16 *)&game_progress.unk16E0)[0] = VSync(-1);
        ((u16 *)&game_progress.unk16E0)[1] = rand();
    }
    save->unk0 = game_progress.unk16E0;
    copy_words(save->unk18BC, &game_progress.unk14A8[5], 5);
    copy_words(save->unk18D0, &game_progress.unk14A8[10], 0x4f);
    copy_words(save->unk1A0C, (u32 *)&inventory, 0x4f);
    copy_words(save->unk1B48, vis_table_c.unk0048, 10);
    save->unk1B70[0] = viewer.unk38[0];
    save->unk1B70[1] = viewer.unk38[1];
    save->unk1B70[2] = viewer.unk38[2];
    save->unk1B70[3] = viewer.unk38[3];
    save->unk1B70[4] = viewer.unk38[4];
    save->unk1B70[5] = viewer.unk38[5];
    save->unk1B70[6] = viewer.unk38[6];
    save->unk1B70[7] = viewer.unk38[7];
    save->unk1B70[8] = viewer.unk38[8];
    save->unk1B70[9] = viewer.unk38[9];
    save->unk1B70[10] = viewer.unk38[10];
    save->unk1B70[11] = viewer.unk38[11];
    save->unk1B70[12] = viewer.unk38[12];
    save->unk1B70[13] = viewer.unk38[13];
    save->unk1B70[14] = viewer.unk38[14];
    save->unk1B70[15] = viewer.unk38[15];
    save->unk1B70[16] = viewer.unk38[16];
    save->unk1B70[17] = viewer.unk38[17];
    save->unk1B70[18] = viewer.unk38[18];
    save->unk1B70[19] = viewer.unk38[19];
    save->unk1B70[20] = viewer.unk38[20];
    save->unk1B9C[0] = game_progress.unk14A8[89];
    save->unk1B9C[1] = game_progress.unk14A8[90];
    save->unk1B9C[2] = game_progress.unk14A8[91];
    r = vis_table_c.unk0118;
    for (i = 0; i < 314; i++, r++)
        save->unk1BA8[i] = r->unk28;
    copy_words(save->unk1CE4, (u32 *)lighting.area_mode, 24);
    copy_halfwords(save->unk1D44, inventory.unk13A, 14);
    *(StHalves7 *)save->unk1D60 = *(StHalves7 *)inventory.unk156;
    copy_halfwords(save->unk1D6E, pad_state.unk22, 12);
}

/* Restores the state save_pack_data saved. */
ADDRESS(0x80056aa0, 0x8bc)
void save_unpack_data(StSaveData *save)
{
    int i;
    int area;
    int bit;
    Vis48 *entry;
    Vis44 *r;

    ((StAreaBytes *)&area_state)->unk4 = *(StBytes5 *)save->unk4;
    copy_words(game_progress.unk0[0], save->unkC, 0x60);
    copy_words(game_progress.unk180[0], save->unk18C, 0x30);
    copy_words(game_progress.unk240[0], save->unk24C, 0x180);
    copy_words(game_progress.unk840[0], save->unk84C, 0x180);
    copy_words(game_progress.unkE40[0], save->unkE4C, 0x180);
    copy_words(&game_progress.unk1440.word, save->unk144C, 2);
    copy_words((u32 *)game_progress.unk1448, save->unk1454, 12);
    copy_words((u32 *)game_progress.unk1478, save->unk1484, 12);
    copy_words(game_progress.unk14A8, save->unk14B4, 5);
    for (area = 0; area < 48; area++) {
        game_progress.unk16E4[area] = 0;
        for (i = 0; i < 64; i++) {
            if ((game_progress.unk0[area][i / 32] & BIT(i)) && i < 48)
                game_progress.unk16E4[area]++;
        }
    }
    vis_table_c.unk0000[2] = save->unk14C8[0];
    vis_table_c.unk0000[3] = save->unk14C8[1];
    vis_table_c.unk0000[4] = save->unk14C8[2];
    vis_table_c.unk0000[5] = save->unk14C8[3];
    copy_words(vis_table_c.unk0070, save->unk14CC, 9);
    entry = &vis_table_c.entries[32];
    for (i = 0; i < 16; i++, entry++) {
        if (i < vis_table_c.unk0000[2]) {
            entry->unk00 = save->unk14F0[i].unk00;
            entry->unk02 = save->unk14F0[i].unk02;
            entry->unk08.vx = save->unk14F0[i].unk04[0];
            entry->unk08.vy = save->unk14F0[i].unk04[1];
            entry->unk08.vz = save->unk14F0[i].unk04[2];
            entry->unk18 = save->unk14F0[i].unk10[0];
            entry->unk1A = save->unk14F0[i].unk10[1];
            entry->unk1C = save->unk14F0[i].unk10[2];
            entry->unk22 = save->unk14F0[i].unk16;
            entry->unk2c = save->unk14F0[i].unk17;
            r = &vis_table_c.unk0118[entry->unk00];
            *(u32 *)entry->unk04 = 0;
            entry->unk20 = r->unk1E;
            entry->unk23 = 0;
            entry->unk24 = 0;
            entry->unk25 = 0;
            entry->unk26 = 0;
            entry->unk2D = r->unk5[1];
        } else {
            entry->unk00 = 0xffff;
            entry->unk22 = 0;
        }
    }
    entry = &vis_table_c.entries[48];
    for (i = 0; i < 16; i++, entry++) {
        if (i < vis_table_c.unk0000[3]) {
            entry->unk00 = save->unk14F0[16 + i].unk00;
            entry->unk02 = save->unk14F0[16 + i].unk02;
            entry->unk08.vx = save->unk14F0[16 + i].unk04[0];
            entry->unk08.vy = save->unk14F0[16 + i].unk04[1];
            entry->unk08.vz = save->unk14F0[16 + i].unk04[2];
            entry->unk18 = save->unk14F0[16 + i].unk10[0];
            entry->unk1A = save->unk14F0[16 + i].unk10[1];
            entry->unk1C = save->unk14F0[16 + i].unk10[2];
            entry->unk22 = save->unk14F0[16 + i].unk16;
            entry->unk2c = save->unk14F0[16 + i].unk17;
            r = &vis_table_c.unk0118[entry->unk00];
            *(u32 *)entry->unk04 = 0;
            entry->unk20 = r->unk1E;
            entry->unk23 = 0;
            entry->unk24 = 0;
            entry->unk25 = 0;
            entry->unk26 = 0;
            entry->unk2D = r->unk5[1];
        } else {
            entry->unk00 = 0xffff;
            entry->unk22 = 0;
        }
    }
    viewer.unk2C = save->unk17F0;
    viewer.unk34 = save->unk17F4;
    viewer.unk2 = save->unk17F8;
    viewer.unk6 = save->unk17FA;
    viewer.unk8 = save->unk17FC;
    viewer.unk30 = save->unk17FE;
    viewer.unk26 = save->unk1800;
    viewer.unk27 = save->unk1801;
    viewer.unk28 = 0;
    for (bit = 0; bit < 266; bit++) {
        if (vis_table_c.unk0070[bit / 32] & BIT(bit))
            viewer.unk28++;
    }
    viewer.unk32 = save->unk1803;
    copy_halfwords((u16 *)viewer.unk62, (u16 *)save->unk1804, 14);
    copy_halfwords((u16 *)viewer.unk7E, (u16 *)save->unk1820, 14);
    copy_halfwords(viewer.unk9A, save->unk183C, 14);
    copy_words((u32 *)&viewer.unk22C, save->unk1870, 3);
    viewer.unk190[0][0] = save->unk187C[0][0];
    viewer.unk190[0][1] = save->unk187C[0][1];
    viewer.unk190[1][0] = save->unk187C[1][0];
    viewer.unk190[1][1] = save->unk187C[1][1];
    viewer.unk190[2][0] = save->unk187C[2][0];
    viewer.unk190[2][1] = save->unk187C[2][1];
    viewer.unk190[3][0] = save->unk187C[3][0];
    viewer.unk190[3][1] = save->unk187C[3][1];
    viewer.unk190[4][0] = save->unk187C[4][0];
    viewer.unk190[4][1] = save->unk187C[4][1];
    viewer.unk190[5][0] = save->unk187C[5][0];
    viewer.unk190[5][1] = save->unk187C[5][1];
    viewer.unk190[6][0] = save->unk187C[6][0];
    viewer.unk190[6][1] = save->unk187C[6][1];
    viewer.unk190[7][0] = save->unk187C[7][0];
    viewer.unk190[7][1] = save->unk187C[7][1];
    viewer.unk190[8][0] = save->unk187C[8][0];
    viewer.unk190[8][1] = save->unk187C[8][1];
    viewer.unk190[9][0] = save->unk187C[9][0];
    viewer.unk190[9][1] = save->unk187C[9][1];
    viewer.unk190[10][0] = save->unk187C[10][0];
    viewer.unk190[10][1] = save->unk187C[10][1];
    viewer.unk190[11][0] = save->unk187C[11][0];
    viewer.unk190[11][1] = save->unk187C[11][1];
    viewer.unk190[12][0] = save->unk187C[12][0];
    viewer.unk190[12][1] = save->unk187C[12][1];
    viewer.unk219 = save->unk18B0[0];
    viewer.unk21A = save->unk18B0[1];
    viewer.unk21B = save->unk18B0[2];
    viewer.position = save->unk1858;
    viewer.unk278 = save->unk1868;
    game_settings.unk0 = save->unk18B4[0];
    game_settings.unk1 = save->unk18B4[1];
    game_settings.unk2 = save->unk18B4[2];
    game_settings.unk3 = save->unk18B4[3];
    game_settings.unk4 = save->unk18B4[4];
    game_settings.unk5 = save->unk18B4[5];
    game_progress.unk16E0 = save->unk0;
    copy_words(&game_progress.unk14A8[5], save->unk18BC, 5);
    copy_words(&game_progress.unk14A8[10], save->unk18D0, 0x4f);
    copy_words((u32 *)&inventory, save->unk1A0C, 0x4f);
    copy_words(vis_table_c.unk0048, save->unk1B48, 10);
    viewer.unk38[0] = save->unk1B70[0];
    viewer.unk38[1] = save->unk1B70[1];
    viewer.unk38[2] = save->unk1B70[2];
    viewer.unk38[3] = save->unk1B70[3];
    viewer.unk38[4] = save->unk1B70[4];
    viewer.unk38[5] = save->unk1B70[5];
    viewer.unk38[6] = save->unk1B70[6];
    viewer.unk38[7] = save->unk1B70[7];
    viewer.unk38[8] = save->unk1B70[8];
    viewer.unk38[9] = save->unk1B70[9];
    viewer.unk38[10] = save->unk1B70[10];
    viewer.unk38[11] = save->unk1B70[11];
    viewer.unk38[12] = save->unk1B70[12];
    viewer.unk38[13] = save->unk1B70[13];
    viewer.unk38[14] = save->unk1B70[14];
    viewer.unk38[15] = save->unk1B70[15];
    viewer.unk38[16] = save->unk1B70[16];
    viewer.unk38[17] = save->unk1B70[17];
    viewer.unk38[18] = save->unk1B70[18];
    viewer.unk38[19] = save->unk1B70[19];
    viewer.unk38[20] = save->unk1B70[20];
    game_progress.unk14A8[89] = save->unk1B9C[0];
    game_progress.unk14A8[90] = save->unk1B9C[1];
    game_progress.unk14A8[91] = save->unk1B9C[2];
    r = vis_table_c.unk0118;
    for (i = 0; i < 314; i++, r++)
        r->unk28 = save->unk1BA8[i];
    copy_words((u32 *)lighting.area_mode, save->unk1CE4, 24);
    copy_halfwords(inventory.unk13A, save->unk1D44, 14);
    *(StHalves7 *)inventory.unk156 = *(StHalves7 *)save->unk1D60;
    copy_halfwords(pad_state.unk22, save->unk1D6E, 12);
}

ADDRESS(0x8005735c, 0x30)
void area_run_frame_hook(void)
{
    area_state.hooks->frame();
}
