/*
 * ST.EXE: an empty handler, a CD completion callback and the area image
 * fix-ups after an area load (WIP module; translation-unit start unknown).
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
#include <st/player.h>
#include <st/area.h>

RODATA(0x80011f18, 0x38f)
SDATA(0x80088064, 0x38)

DATA(0x800881d8, 0x4, ".sbss")
u8 *DAT_800881d8;

/* The handler every entry of the table at 0x8008038c starts with. */
ADDRESS(0x800443ec, 0x8)
void func_800443ec(void)
{
}

/* CD completion: marks the request done. */
ADDRESS(0x800443f4, 0x10)
void func_800443f4(StCdRequest *request)
{
    request->unk10 = 1;
}

/*
 * Reloads the area, then copies the images two areas patch in VRAM.
 * Not exact: retail keeps each `!= 1` flag test as a compare with 1 and
 * reuses the first flag (known 1) as the constant for the second.
 */
ADDRESS(0x80044404, 0x270)
void func_80044404(void)
{
    u32 flag;

    func_80045f90(1, 481, func_800443f4);
    area_state.unk0 = 1;
    area_state.unk2 = 5;
    area_state.unk13 = 0xff;
    area_state.unk14 = 0xff;
    area_state.unk12 = area_state.unk4;
    func_800456f8();
    func_80015ad4();
    if (area_state.area == 24) {
        if (!((DAT_801cba08.unk1440 >> 17) & 1))
            return;
        flag = (vis_lists.unk47E0 >> 3) & 1;
        if (flag != 1)
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
        flag = (DAT_801cba08.unk1440 >> 13) & 1;
        if (flag != 1)
            return;
        flag = (vis_lists.unk47E0 >> 4) & 1;
        if (flag != 1)
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
void func_80044674(void)
{
    CdlFILE file;
    u32 *p = (u32 *)DAT_8009c888;

    area_state.unk5 = area_state.unk6 = area_state.unk7 = 255;
    area_state.unk0 = 0;
    area_state.unk4 = 0;
    area_state.unk5 = 0;
    area_state.unk6 = 0;
    area_state.unk7 = 0;
    area_state.unk8 = 0;
    area_state.area = 0;
    area_state.unk2 = 0;
    vis_table_a.unk6169 = 0;
    DAT_800881d8 = DAT_80013980 + 4;
    DAT_800881dc = DAT_80013980;
    func_80016294(2, "CHR0\\M00.T");
    func_80016294(0, "COM\\MO.T");
    func_80016294(1, "COM\\FDAT.T");
    func_80016294(3, "COM\\STAT.T");
    func_80016294(4, "COM\\CRT.T");
    func_80016294(6, "COM\\ICON.T");
    func_80016294(5, "COM\\ITEM.T");
    func_80016294(7, "COM\\EQUIP.T");
    func_80016294(8, "COM\\TALKMSG.T");
    CdSearchFile(&file, "\\ST\\XA0\\HAM.XA;1");
    DAT_801cba08.unk1618[0] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\SHOP0.XA;1");
    DAT_801cba08.unk1618[1] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\SHOP1.XA;1");
    DAT_801cba08.unk1618[2] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\SHOP2.XA;1");
    DAT_801cba08.unk1618[3] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\SHOP3.XA;1");
    DAT_801cba08.unk1618[4] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\03100.XA;1");
    DAT_801cba08.unk1618[5] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\19010.XA;1");
    DAT_801cba08.unk1618[6] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\30200.XA;1");
    DAT_801cba08.unk1618[7] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\07900.XA;1");
    DAT_801cba08.unk1618[8] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\10300.XA;1");
    DAT_801cba08.unk1618[9] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\01300.XA;1");
    DAT_801cba08.unk1618[10] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\04601.XA;1");
    DAT_801cba08.unk1618[11] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\04400.XA;1");
    DAT_801cba08.unk1618[12] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\09502.XA;1");
    DAT_801cba08.unk1618[13] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\22700.XA;1");
    DAT_801cba08.unk1618[14] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\10200.XA;1");
    DAT_801cba08.unk1618[15] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\03000.XA;1");
    DAT_801cba08.unk1618[16] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\09506.XA;1");
    DAT_801cba08.unk1618[17] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\30400.XA;1");
    DAT_801cba08.unk1618[18] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\10100.XA;1");
    DAT_801cba08.unk1618[19] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\02400.XA;1");
    DAT_801cba08.unk1618[20] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\10301.XA;1");
    DAT_801cba08.unk1618[21] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\12300.XA;1");
    DAT_801cba08.unk1618[22] = file.pos;
    CdSearchFile(&file, "\\ST\\XA0\\22900.XA;1");
    DAT_801cba08.unk1618[23] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\09503.XA;1");
    DAT_801cba08.unk1618[24] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\10800.XA;1");
    DAT_801cba08.unk1618[25] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\09504.XA;1");
    DAT_801cba08.unk1618[26] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\11100.XA;1");
    DAT_801cba08.unk1618[27] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\00100.XA;1");
    DAT_801cba08.unk1618[28] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\02500.XA;1");
    DAT_801cba08.unk1618[29] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\07700.XA;1");
    DAT_801cba08.unk1618[30] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\10900.XA;1");
    DAT_801cba08.unk1618[31] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\08100.XA;1");
    DAT_801cba08.unk1618[32] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\09505.XA;1");
    DAT_801cba08.unk1618[33] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\10302.XA;1");
    DAT_801cba08.unk1618[34] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\20400.XA;1");
    DAT_801cba08.unk1618[35] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\13900.XA;1");
    DAT_801cba08.unk1618[36] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\01100.XA;1");
    DAT_801cba08.unk1618[37] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\29500.XA;1");
    DAT_801cba08.unk1618[38] = file.pos;
    CdSearchFile(&file, "\\ST\\XA1\\30600.XA;1");
    DAT_801cba08.unk1618[39] = file.pos;
    func_80016030(1, 0x1e0, (u_long *)DAT_8009c888);
    func_80045f90(1, 0x1e1, func_800443f4);
    func_80058a0c(1, 0x1e2, 0);
    func_80014cfc((u32 *)&vis_table_b, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80014cfc((u32 *)DAT_801b5220, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80014cfc((u32 *)DAT_80088c88, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80014cfc((u32 *)effects.defs, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80014cfc((u32 *)DAT_8007f78c, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80014cfc((u32 *)DAT_801dd1c0.sounds, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80014cfc((u32 *)&menu_messages, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80014cfc((u32 *)vis_table_c.unk0118, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80014cfc((u32 *)DAT_8009b088, p + 1, *p >> 2);
    anim_register_pack(40, DAT_8009b088);
    func_80015ad4();
    func_8003ea20();
    func_80052d10();
}
