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
 * (DAT_8008038c) until an area's own handlers are loaded. */
ADDRESS(0x800443ec, 0x8)
void area_hook_none(void)
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
        u32 marker;

        if (!((DAT_801cba08.unk1440.word >> 17) & 1))
            return;
        marker = (vis_lists.unk47E0 >> 3) & 1;
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
        flag = (DAT_801cba08.unk1440.word >> 13) & 1;
        if (flag != 1)
            return;
        if (((vis_lists.unk47E0 >> 4) & 1) != flag)
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
    DAT_800881dc[0] = DAT_80013980;
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

/*
 * Loading the current area set again and waiting for it, and moving the
 * viewer.
 */

/* Requests the current areas again (99 marks each unloaded) and runs the
 * loader until the request is done, then resets the primitive buffers. */
ADDRESS(0x80044ee8, 0x128)
void func_80044ee8(void)
{
    area_state.unk10 = area_state.unk4;
    area_state.unk11 = area_state.unk5;
    area_state.unk12 = area_state.unk6;
    area_state.unk13 = area_state.unk7;
    area_state.unk14 = area_state.unk8;
    area_state.unk0 = 1;
    area_state.unk4 = 99;
    area_state.unk5 = 99;
    area_state.unk6 = 99;
    area_state.unk7 = 99;
    area_state.unk8 = 99;
    area_state.unk2 = 0;
    area_state.unk20 = 127;
    area_state.unk21 = 127;
    area_state.unk22 = 127;
    area_state.unk1F = 0;
    do {
        func_8001531c();
        func_800456f8();
        if (area_state.unk0 == 0)
            break;
    } while ((s16)area_state.unk2 < 6 || (s16)area_state.unk2 == 240);
    func_80015ad4();
    world_state.unk8308[0] = DAT_800fc488;
    world_state.unk8308[1] = DAT_800f2c88;
}

ADDRESS(0x80045010, 0x34)
void func_80045010(int dx, int dy, int dz)
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
 * loaded; f0..f2 is the fade colour and f3..f5 the next fade's (WIP). */
/* All nine direct callers discard the word result; the return edges keep
 * v0 live. */
ADDRESS(0x80045044, 0x524)
KF_VALUELESS_S32 func_80045044(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, s8 f0, s8 f1, s8 f2, u8 f3, u8 f4,
                   u8 f5)
{
    u8 n0, n1, n2, n3, n4;
    u8 m0, m1, m2, m3, m4;

    if (a0 == 0xff) {
        n0 = area_state.unk4;
        m0 = area_state.unk10;
        if (a1 == 0xff) {
            n1 = area_state.unk5;
            m1 = area_state.unk11;
        } else {
            n1 = m1 = a1;
        }
        if (a2 == 0xff) {
            n2 = area_state.unk6;
            m2 = area_state.unk12;
        } else {
            n2 = m2 = a2;
        }
        if (a3 == 0xff) {
            n3 = area_state.unk7;
            m3 = area_state.unk13;
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
        n1 = area_state.unk5;
        n2 = area_state.unk6;
        n3 = area_state.unk7;
        n4 = area_state.unk8;
        m2 = a2;
        m3 = a3;
    }
    if (area_state.unk0 != 0)
        goto busy;
    if (area_state.unk4 == n0 && area_state.unk5 == n1 && area_state.unk6 == n2 &&
        area_state.unk7 == n3 && area_state.unk8 == n4)
        return;
start:
    if (area_state.unk4 != 99 && area_state.unk4 != n0)
        func_80055cf4(area_state.unk4);
    area_state.unk0 = 1;
    area_state.unk2 = 0;
    area_state.unk10 = a0;
    area_state.unk11 = a1;
    area_state.unk12 = a2;
    area_state.unk13 = a3;
    area_state.unk14 = a4;
    area_state.unk20 = f0;
    area_state.unk21 = f1;
    area_state.unk22 = f2;
    area_state.unk23 = f3;
    area_state.unk24 = f4;
    area_state.unk25 = f5;
    area_state.unk1C = 0;
    if (a0 < 44)
        area_state.unk18 = cd_state.unk4;
    else
        area_state.unk18 = -1;
    if (a1 == 0xff) {
        area_state.unk1D = 0;
    } else {
        area_state.unk1D = 1;
        area_state.unk1F = 1;
    }
    return;
busy:
    if ((area_state.unk0 != 1 || area_state.unk10 == m0) && area_state.unk11 == m1 &&
        area_state.unk12 == m2 && area_state.unk13 == m3 && area_state.unk14 == m4)
        return;
    if (area_state.unk10 == 0xff && area_state.unk4 == n0)
        return;
    if (area_state.unk11 == 0xff && area_state.unk5 == a1)
        return;
    if (area_state.unk12 == 0xff && area_state.unk6 == a2)
        return;
    if (area_state.unk13 == 0xff && area_state.unk7 == a3)
        return;
    if (area_state.unk14 == 0xff && area_state.unk8 == a4)
        return;
    if ((area_state.unk10 != 0xff && a0 == 0xff) || (area_state.unk11 != 0xff && a1 == 0xff) ||
        (area_state.unk12 != 0xff && a2 == 0xff) || (area_state.unk13 != 0xff && a3 == 0xff) ||
        (area_state.unk14 != 0xff && a4 == 0xff)) {
        func_80015ad4();
        while (area_state.unk0) {
            func_8001531c();
            func_800456f8();
        }
    } else {
        while ((s16)area_state.unk2 == 240)
            func_8001531c();
    }
    ExitCriticalSection();
    if (area_state.unk1C && a0 != 0xff && f0 == 127 && area_state.unk10 != a0) {
        f0 = -area_state.unk20;
        f2 = -area_state.unk22;
        f1 = -area_state.unk21;
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
    area_state.unk2 = 0;
}

ADDRESS(0x8004557c, 0x18)
void func_8004557c(void)
{
    area_state.unk2 = 1;
}

ADDRESS(0x80045594, 0x18)
void func_80045594(void)
{
    area_state.unk2 = 2;
}

ADDRESS(0x800455ac, 0x18)
void func_800455ac(void)
{
    area_state.unk2 = 3;
}

ADDRESS(0x800455c4, 0x18)
void func_800455c4(void)
{
    area_state.unk2 = 4;
}

ADDRESS(0x800455dc, 0x18)
void func_800455dc(void)
{
    area_state.unk2 = 5;
}

ADDRESS(0x800455f4, 0x54)
void func_800455f4(StCdRequest *request)
{
    if (request->unk10 == 0) {
        request->unk10 = 1;
    } else if (request->unk10 == 2) {
        area_state.unk2 = 6;
        DAT_801dd1c0.unkC = 0;
        area_state.unk26 = DAT_801cd120.unk0;
    }
}

ADDRESS(0x80045648, 0x1c)
void func_80045648(StCdRequest *request)
{
    if (request->unk10 == 0)
        request->unk10 = 1;
}

ADDRESS(0x80045664, 0x18)
void func_80045664(void)
{
    area_state.unk2 = 8;
}

/* Commits the requested area numbers that are set (0xff keeps one). */
ADDRESS(0x8004567c, 0x7c)
void func_8004567c(void)
{
    vis_table_a.unk6169 = 0;
    area_state.unk1E = 0;
    if (area_state.unk10 != 0xff)
        area_state.unk4 = area_state.unk10;
    if (area_state.unk11 != 0xff)
        area_state.unk5 = area_state.unk11;
    if (area_state.unk12 != 0xff)
        area_state.unk6 = area_state.unk12;
    if (area_state.unk13 != 0xff)
        area_state.unk7 = area_state.unk13;
    if (area_state.unk14 != 0xff)
        area_state.unk8 = area_state.unk14;
    area_state.unk0 = 0;
}

/*
 * The area load state machine: the area's script and tables, its character
 * file table, the object tables and sound banks.
 */

ADDRESS(0x800456f8, 0x898)
void func_800456f8(void)
{
    Vis212 *a;
    Vis88 *b;
    Vis48 *c;
    StVabSlot *slot;
    u_long *p = DAT_80088c88;
    int i, j;

    /* Dead: phase 3 starts the cursor over. What survives is the buffer
     * address's high half, which phases 1 and 2 reuse from $s2. */
    p++;
    if (area_state.unk0 == 0)
        return;
    if (area_state.unk0 != 1)
        return;
    switch ((s16)area_state.unk2) {
    case 0:
        if (area_state.unk10 == 0xff)
            goto phase3;
        if (func_80015ec8(1, area_state.unk10 * 10))
            func_80045f90(1, area_state.unk10 * 10, func_80045648);
        area_state.unk2 = 240;
        b = vis_table_b.entries;
        for (i = 239; i != -1; i--) {
            if (b->unk05 == 236 && b->unk54 != -1) {
                func_8005850c(b->unk54, b->unk4A.snd.unk4A + 1108);
                b->unk54 = -1;
            }
            b->unk05 = 0xff;
            b++;
        }
        func_80015d54(1, (area_state.unk10 * 10) | 1, (u8 *)DAT_80088c88, func_8004557c);
        break;
    case 1:
        area_state.unk2 = 240;
        area_state.unkC = &DAT_8008038c;
        func_80015d54(1, area_state.unk10 * 10 + 2, DAT_800881dc[0], func_80045594);
        viewer.unk174 = NULL;
        viewer.unk178 = -1;
        func_80014cfc((u32 *)vis_records, (u32 *)DAT_80088c88, 0x3e00);
        if ((s8)area_state.unk20 != 127) {
            area_state.unk1C = 1;
            func_80045010((s8)area_state.unk20 * 4096 + (s8)area_state.unk23 * 128,
                          (s8)area_state.unk22 * 4096 + (s8)area_state.unk24 * 128,
                          (s8)area_state.unk21 * 4096 + (s8)area_state.unk25 * 128);
        }
        area_state.area = area_state.unk10;
        break;
    case 2:
        area_state.unk2 = 240;
        func_80015d54(1, area_state.unk10 * 10 + 3, (u8 *)DAT_80088c88, func_800455ac);
        break;
    case 3:
    phase3:
        if (area_state.unk11 != 0xff) {
            char name[] = "CHRX\\MXX.T";

            func_800152f8(cd_state.archives[2].sectors);
            cd_state.archives[2].sectors = NULL;
            name[3] = area_state.unk10 / 10 + '0';
            name[6] = area_state.unk10 / 10 + '0';
            name[7] = area_state.unk10 % 10 + '0';
            func_80016294(2, name);
            area_state.unk2 = 240;
            func_80015d54(1, area_state.unk11 * 10 + 4, (u8 *)DAT_800fc488, func_800455c4);
        }
        if (area_state.unk10 != 0xff) {
            p = DAT_80088c88;
            a = vis_table_a.entries;
            for (i = 63; i != -1; i--) {
                if (a->unkA4) {
                    anim_slot_free((StAnimSlot *)a->unkA4);
                    if (a->unkA4)
                        anim_set_free((StAnimSet *)a->unkA4);
                }
                a->unk00 = 0xff;
                a++;
            }
            b = vis_table_b.entries;
            for (i = 239; i != -1; i--) {
                if (*(StAnimSlot **)b->unk44) {
                    anim_slot_free(*(StAnimSlot **)b->unk44);
                    if (*(StAnimSlot **)b->unk44)
                        anim_set_free(*(StAnimSet **)b->unk44);
                }
                b->unk0a = 0xffff;
                b++;
            }
            c = vis_table_c.entries;
            for (i = 63; i != -1; i--) {
                if (*(StAnimSlot **)c->unk04) {
                    anim_slot_free(*(StAnimSlot **)c->unk04);
                    if (*(StAnimSlot **)c->unk04)
                        anim_set_free(*(StAnimSet **)c->unk04);
                }
                c++;
            }
            func_80052d10();
            for (i = 15; i != -1; i--) {
                if (world_state.unk834C[128 + i])
                    func_80015230(world_state.unk834C[128 + i]);
            }
            for (i = 127; i != -1; i--) {
                if (world_state.unk8C7C[i])
                    func_80015230(world_state.unk8C7C[i]);
            }
            func_80014cfc((u32 *)vis_table_a.unk3500, (u32 *)(p + 1), 2816);
            func_8004e8ec();
            p = (u_long *)((u8 *)p + (*p + 4));
            func_8004e960((VisAFileEntry *)(p + 1));
            p = (u_long *)((u8 *)p + (*p + 4));
            func_80014cfc((u32 *)&vis_table_b.unk0000[300], (u32 *)(p + 1), 192);
            p = (u_long *)((u8 *)p + (*p + 4));
            func_8004eebc((u8 *)(p + 1));
            p = (u_long *)((u8 *)p + (*p + 4));
            func_80052f30((VisCFileEntry *)(p + 1));
            p = (u_long *)((u8 *)p + (*p + 4));
            placements_expand((StPlacement *)(p + 1));
            func_80055f9c(area_state.unk10);
            DAT_801cba08.unk1478[area_state.unk10] |= 1;
            area_state.unkC = (AreaHooks *)DAT_800881d8;
            func_80016878();
            area_state.unkC->unk14();
            vis_table_a.unk6168 = 254;
            vis_table_a.unk6158[3] = 0xff;
            vis_table_a.unk6158[2] = 0xff;
            vis_table_a.unk6158[1] = 0xff;
            vis_table_a.unk6158[0] = 0xff;
            slot = &DAT_801dd1c0.slots[8];
            for (j = 15; j != -1; j--) {
                if (slot->block) {
                    while (slot->block[2] == 3)
                        func_8001531c();
                    SsVabClose(slot->vab);
                    slot->vab = -1;
                    slot->block = NULL;
                }
                slot++;
            }
            viewer.unk21C = 1;
            area_state.unk1E = 1;
        }
        if (area_state.unk11 == 0xff)
            goto phase5;
        break;
    case 4:
        if (area_state.unk11 != 0xff) {
            area_state.unk2 = 240;
            func_80015d54(1, area_state.unk11 * 10 + 5, (u8 *)DAT_800f2c88, func_800455dc);
        }
        break;
    case 5:
    phase5:
        if (area_state.unk12 != 0xff) {
            area_state.unk2 = 240;
            func_80045f90(1, area_state.unk12 * 10 + 6, func_800455f4);
        }
        func_80015ad4();
        area_state.unk1D = 0;
        area_state.unk1F = 0;
        if (area_state.unk13 == 0xff && area_state.unk14 == 0xff) {
            area_state.unk2 = 7;
            goto done;
        }
        area_state.unk2 = 6;
        /* fall through */
    case 6: {
        StVabSlot *bgm = &DAT_801dd1c0.slots[1];

        if (bgm->vab != -1) {
            SsVabClose(bgm->vab);
            bgm->vab = -1;
            bgm->block = NULL;
        }
    }
        if (area_state.unk13 != 0xff) {
            area_state.unk2 = 240;
            func_80058a0c(1, area_state.unk13 * 10 + 7, 1);
        }
        if (area_state.unk14 == 0xff)
            goto done;
        area_state.unk2 = 7;
        break;
    case 7:
    done:
        func_8004567c();
        break;
    case 8:
        func_8004567c();
        break;
    case 240:
        /* a CD read is pending */
        break;
    }
}
