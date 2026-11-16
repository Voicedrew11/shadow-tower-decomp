/*
 * ST.EXE: the pause menu pages (WIP module). Each page restarts the frame,
 * draws its text, numbers and icons with the src/st/text.c primitives,
 * frames itself with func_80029618 and darkens the world behind it.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/vis.h>
#include <st/text.h>
#include <st/menu.h>
#include <st/funcs.h>

RODATA(0x800110d0, 0xe0)
SDATA(0x80087f38, 0x28)

/* Blink phase of the map page's arrows (0..31). */
DATA(0x80087f34, 0x4, ".sdata")
static s32 DAT_80087f34 = 0;

/* func_80027440/func_800276bc take the digit count in the top byte. */
#define DIGITS(n) ((n) << 24)

/* Closes a page: its frame, the end of the frame and the darkened world. */
#define PAGE_END()                     \
    func_80029618(16, 36, 288, 192);   \
    func_800175a4();                   \
    func_8002948c();                   \
    func_80017634()

/*
 * The status page.
 *
 * Not exact (99.8%): retail schedules the second `i = 0` after the first
 * number call's colour argument (move s4,zero after move a3,s0).
 */
ADDRESS(0x80029b74, 0x65c)
void func_80029b74(void)
{
    u16 label_x[8] = {37, 47, 20, 20, 30, 41, 34, 37};
    DVECTOR row[4] = {{196, 94}, {274, 94}, {172, 104}, {298, 104}};
    CVECTOR lit[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {128, 64, 0, 0}, {128, 64, 0, 0}};
    CVECTOR dim[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 64, 128, 0}, {0, 64, 128, 0}};
    MenuScratch *sp;
    int i;
    int kind;
    int n;

    func_800174c4();
    sp = MENU_SCRATCH;
    sp->style[0].r = 72;
    sp->style[0].g = 72;
    sp->style[0].b = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    for (i = 0; i < 11; i++)
        func_8002732c(MESSAGE(i), 24, i * 16 + 48, &sp->style[0]);
    if (viewer.unk2C & 7)
        func_8002732c(MESSAGE(20), 106, 112, &sp->style[0]);
    else if (viewer.unk2C & 0x18)
        func_8002732c(MESSAGE(21), 93, 112, &sp->style[0]);
    else if (viewer.unk2C & 0x60)
        func_8002732c(MESSAGE(22), 119, 112, &sp->style[0]);
    else if (viewer.unk2C & 0x80)
        func_8002732c(MESSAGE(23), 119, 112, &sp->style[0]);
    else if (viewer.unk2C & 0x100)
        func_8002732c(MESSAGE(24), 106, 112, &sp->style[0]);
    else
        func_8002732c(MESSAGE(19), 106, 112, &sp->style[0]);
    if (viewer.unk38[16] != 0xffff && vis_table_c.unk0118[viewer.unk38[16]].unk5[0x1d] >= 9) {
        kind = vis_table_c.unk0118[viewer.unk38[16]].unk5[0x1d];
        n = 0;
        if (kind - 9 == 2)
            n = 14;
        func_8002732c(MESSAGE(17 + kind), 119 - n, 208, &sp->style[0]);
    } else {
        func_8002732c(MESSAGE(25), 119, 208, &sp->style[0]);
    }
    for (i = 0; i < 8; i++)
        func_8002732c(MESSAGE(11 + i), label_x[i] + 172, i * 16 + 90, &sp->style[2]);
    i = 0;
    func_800276bc(viewer.unk2 | DIGITS(4), 78, 48, &sp->style[0]);
    func_80026ecc(15, 106, 48, &sp->style[0]);
    func_800276bc(viewer.unk0 | DIGITS(4), 120, 48, &sp->style[0]);
    func_800276bc(viewer.unk6 | DIGITS(4), 78, 64, &sp->style[0]);
    func_80026ecc(15, 106, 64, &sp->style[0]);
    func_800276bc(viewer.unk4 | DIGITS(4), 120, 64, &sp->style[0]);
    func_800276bc(viewer.unk8 | DIGITS(4), 78, 80, &sp->style[0]);
    func_80026ecc(15, 106, 80, &sp->style[0]);
    func_800276bc(4921 | DIGITS(4), 120, 80, &sp->style[0]);
    func_800276bc(DAT_801c7590.counts[0x10a].count | DIGITS(3), 127, 96, &sp->style[0]);
    func_800276bc(viewer.unk10 | DIGITS(3), 127, 128, &sp->style[0]);
    func_800276bc(viewer.unkE | DIGITS(3), 127, 144, &sp->style[0]);
    func_800276bc(viewer.unkBA[0] | DIGITS(3), 127, 160, &sp->style[0]);
    func_800276bc(viewer.unkBA[1] | DIGITS(3), 127, 176, &sp->style[0]);
    func_800276bc(viewer.unkBA[2] | DIGITS(3), 127, 192, &sp->style[0]);
    sp->unk358.vx = 622;
    sp->unk358.vy = 416;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.x = 196;
    sp->unk33C.y = 54;
    sp->unk33C.w = 72;
    sp->unk33C.h = 24;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 1);
    for (; i < 8; i++) {
        if (i == (s16)sp->unk2DE)
            func_80028c2c(row, lit, 1);
        else
            func_80028c2c(row, dim, 1);
        row[0].vy += 16;
        row[1].vy += 16;
        row[2].vy += 16;
        row[3].vy += 16;
    }
    func_800286a8(0, 0, 0);
    PAGE_END();
}

/*
 * A scrolling list page (five rows of icon, name and count).
 *
 * Not exact (95.4%): retail loads both cursor-colour bytes before the
 * rectangle height store and keeps green in $t0; register/schedule residue.
 */
ADDRESS(0x8002a1d0, 0x3d4)
void func_8002a1d0(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int id = sp->unk0.page_list.entries[sp->unk2E2 + sp->unk2E0];
    int entry;
    int rows;
    int i;

    func_800174c4();
    sp->style[0].r = sp->style[4].r = 72;
    sp->style[0].g = sp->style[4].g = 72;
    sp->style[0].b = sp->style[4].b = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->style[4].cd = 41;
    if (sp->unk2E4 < 16)
        sp->unk360 = 8 - sp->unk2E4;
    else
        sp->unk360 = sp->unk2E4 - 24;
    sp->unk330.x = 23;
    sp->unk330.w = 276;
    sp->unk330.h = 26;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 2 + 48;
    sp->unk330.y = sp->unk2E0 * 26 + 69;
    func_80027d18(&sp->unk330, &sp->unk32C, 257, 1);
    func_800286a8(1, 0, 0);
    sp->unk350.x = 24;
    sp->unk350.y = 70;
    sp->unk350.w = 24;
    sp->unk350.h = 24;
    rows = 5;
    if (sp->unk0.page_list.count < 6)
        rows = sp->unk0.page_list.count;
    sp->style[0].cd |= 0x80;
    if (sp->unk2E0 < rows)
        func_8002732c(MESSAGE_IN(sp->unk0.page_list.messages, 0x176 + id), 32, 208, &sp->style[0]);
    for (i = 0; i < rows; i++) {
        entry = sp->unk0.page_list.entries[sp->unk2E2 + i];
        func_8002732c(MESSAGE_IN(sp->unk0.page_list.messages, entry), 54, i * 26 + 76, &sp->style[0]);
        func_80027440(DAT_801c7590.counts[entry].count | DIGITS(2), 274, i * 26 + 76, &sp->style[4]);
        func_80028364(&sp->unk350, &sp->unk0.icons[(sp->unk2E2 + i) % 5 + 1], NULL, NULL, 16);
        sp->unk350.y += 26;
    }
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.x = 70;
    sp->unk33C.y = 44;
    sp->unk33C.w = 35;
    sp->unk33C.h = 16;
    sp->unk358.vx = 624;
    sp->unk358.vy = 448;
    sp->style[0].cd &= 0x7f;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    sp->unk358.vx = 633;
    sp->unk33C.x = 32;
    sp->unk358.vy = 448;
    sp->unk33C.w = 30;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    PAGE_END();
}

/* The attribute page: seven rows of two columns. */
ADDRESS(0x8002a5a4, 0x3b0)
void func_8002a5a4(void)
{
    MenuScratch *sp;
    CVECTOR *left, *left_num, *right, *right_num;
    int i;

    func_800174c4();
    sp = MENU_SCRATCH;
    sp->style[0].r = sp->style[4].r = 42;
    sp->style[0].g = sp->style[4].g = 42;
    sp->style[0].b = sp->style[4].b = 42;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->style[6].cd = 41;
    sp->style[4].cd = 41;
    sp->unk33C.y = 70;
    sp->unk33C.w = 10;
    sp->unk33C.h = 10;
    sp->unk358.vx = 597;
    sp->unk358.vy = 433;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 502;
    for (i = 0; i < 7; i++) {
        if (sp->unk0.stats.selected == i) {
            left = &sp->style[2];
            left_num = &sp->style[6];
        } else {
            left = &sp->style[0];
            left_num = &sp->style[4];
        }
        if (sp->unk0.stats.selected == i + 7) {
            right = &sp->style[2];
            right_num = &sp->style[6];
        } else {
            right = &sp->style[0];
            right_num = &sp->style[4];
        }
        func_8002732c(MESSAGE(0x212 + i), 46, i * 20 + 68, left);
        func_8002732c(MESSAGE(0x219 + i), 170, i * 20 + 68, right);
        func_80027440((sp->unk0.stats.base[0][i] + sp->unk0.stats.bonus[0][i]) | DIGITS(3), 108, i * 20 + 68, left_num);
        func_80027440((sp->unk0.stats.base[1][i] + sp->unk0.stats.bonus[1][i]) | DIGITS(3), 232, i * 20 + 68, right_num);
        sp->unk33C.x = 141;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, left, 3);
        sp->unk33C.x = 265;
        func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, right, 3);
        sp->unk33C.y += 20;
    }
    sp->style[0].r = sp->style[4].r = 72;
    sp->style[0].g = sp->style[4].g = 72;
    sp->style[0].b = sp->style[4].b = 72;
    func_8002732c(MESSAGE(0x220), 56, 208, &sp->style[0]);
    func_80027440(sp->unk0.stats.value | DIGITS(2), 242, 208, &sp->style[4]);
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.x = 70;
    sp->unk33C.y = 44;
    sp->unk33C.w = 76;
    sp->unk33C.h = 16;
    sp->unk358.vx = 621;
    sp->unk358.vy = 480;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    sp->unk358.vx = 633;
    sp->unk358.vy = 448;
    sp->unk33C.x = 32;
    sp->unk33C.w = 30;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    PAGE_END();
}

/*
 * The map page: visited areas and their completion, or the area summary.
 *
 * Not exact (93.7%): in the summary branch retail keeps the total minutes in
 * $a0 and hours/minutes in $s5 (this build: $s1), and swaps $s2/$s3 for the
 * two colour pointers.
 */
ADDRESS(0x8002a954, 0x994)
void func_8002a954(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    u8 total[48] = {
        255, 18, 25, 21, 20, 19, 20, 255, 25, 27, 29, 32, 28, 24, 26, 23,
        255, 19, 25, 255, 27, 13, 255, 22, 24, 23, 22, 32, 24, 255, 255, 255,
        32, 12, 255, 255, 255, 11, 4, 255, 255, 255, 255, 255, 255, 255, 255, 0,
    };
    CVECTOR arrow;
    RECT unused;
    RECT up = {159, 130, 16, 8};
    RECT next = {296, 130, 16, 8};
    RECT prev = {23, 130, 16, 8};
    RECT down = {160, 130, 16, 8};
    s16 name_x[42] = {
        0,  42, 35, 42, 35, 28, 21, 21, 42, 21, 35, 35, 14, 14, 28, 0,  35, 42, 0,  35, 28,
        0,  28, 42, 21, 42, 7,  42, 0,  0,  0,  35, 21, 0,  0,  0,  35, 42, 0,  42, 0,  0,
    };
    u32 time;
    int hours;
    int minutes;
    int i;
    int id;
    int level;

    func_800174c4();
    if (DAT_80087f34 < 16)
        sp->unk360 = 8 - DAT_80087f34;
    else
        sp->unk360 = DAT_80087f34 - 24;
    sp->style[0].r = sp->style[4].r = 72;
    sp->style[0].g = sp->style[4].g = 72;
    sp->style[0].b = sp->style[4].b = 72;
    sp->style[2].r = sp->style[6].r = 98;
    sp->style[2].g = sp->style[6].g = 66;
    sp->style[2].b = sp->style[6].b = 48;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->style[6].cd = 41;
    sp->style[4].cd = 41;
    sp->unk32C.b = 0;
    sp->unk32C.r = (u8)sp->unk360 * 4 + 96;
    sp->unk32C.g = (u8)sp->unk360 * 4 + 48;
    if (DAT_800881a8 != 0) {
        for (i = 0; i < 10; i++) {
            id = sp->unk0.map.ids[i];
            if (id >= 90) {
                if (id == 99)
                    func_8002732c(MESSAGE(0x269), 53, i * 16 + 54, &sp->style[0]);
                else if (id == sp->unk0.map.current)
                    func_8002732c(MESSAGE(0x208 + id), 53, i * 16 + 54, &sp->style[2]);
                else
                    func_8002732c(MESSAGE(0x208 + id), 53, i * 16 + 54, &sp->style[0]);
            } else if (id == 89) {
                func_8002732c(MESSAGE(0x26a), 67, i * 16 + 54, &sp->style[0]);
            } else if (id == area_state.area) {
                func_8002732c(MESSAGE(0x2bc + id), 67, i * 16 + 54, &sp->style[2]);
                func_80027440(DAT_801cba08.unk16E4[sp->unk0.map.ids[i]] * 100 / total[sp->unk0.map.ids[i]] | DIGITS(3),
                              223, i * 16 + 54, &sp->style[6]);
                func_80026ecc(80, 256, i * 16 + 54, &sp->style[2]);
            } else {
                func_8002732c(MESSAGE(0x2bc + id), 67, i * 16 + 54, &sp->style[0]);
                func_80027440(DAT_801cba08.unk16E4[sp->unk0.map.ids[i]] * 100 / total[sp->unk0.map.ids[i]] | DIGITS(3),
                              223, i * 16 + 54, &sp->style[4]);
                func_80026ecc(80, 256, i * 16 + 54, &sp->style[0]);
            }
        }
    } else {
        func_8002732c(MESSAGE(0x258), 72, 53, &sp->style[2]);
        for (i = 1; i < 5; i++)
            func_8002732c(MESSAGE(0x258 + i), 72, i * 30 + 63, &sp->style[2]);
        if (sp->unk0.map.current != -1) {
            func_8002732c(MESSAGE(0x208 + sp->unk0.map.current), 86, 66, &sp->style[0]);
            func_8002732c(MESSAGE(0x2bc + area_state.area), name_x[area_state.area] + 100, 79,
                          &sp->style[0]);
        } else {
            func_8002732c(MESSAGE(0x2bc + area_state.area), 100, 73, &sp->style[0]);
        }
        func_80027440(viewer.unk27 | DIGITS(3), 179, 107, &sp->style[4]);
        func_80026ecc(15, 213, 107, &sp->style[0]);
        func_80027440(99 | DIGITS(3), 221, 107, &sp->style[4]);
        func_80027440(viewer.unk28 | DIGITS(3), 179, 138, &sp->style[4]);
        func_80026ecc(15, 213, 138, &sp->style[0]);
        func_80027440(265 | DIGITS(3), 221, 138, &sp->style[4]);
        func_80027440(viewer.unk30 | DIGITS(3), 179, 168, &sp->style[4]);
        func_80026ecc(15, 213, 168, &sp->style[0]);
        func_80027440(630 | DIGITS(3), 221, 168, &sp->style[4]);
        time = (u32)viewer.unk34 / 60;
        hours = time / 60;
        minutes = time - hours * 60;
        func_80027440((u16)hours | DIGITS(2), 199, 198, &sp->style[4]);
        func_80026ecc(27, 224, 198, &sp->style[0]);
        func_80027440((u16)(minutes / 10) | DIGITS(1), 231, 198, &sp->style[4]);
        func_80027440((u16)(minutes % 10) | DIGITS(1), 241, 198, &sp->style[4]);
    }
    DAT_80087f34 = (DAT_80087f34 + 1) % 32;
    if (DAT_80087f34 < 16)
        level = 8 - (u16)DAT_80087f34;
    else
        level = (u16)DAT_80087f34 - 24;
    arrow.b = arrow.g = arrow.r = level * 4 + 96;
    if (DAT_800881a8 != 0) {
        if (DAT_8008818c != 0) {
            sp->unk33C.x = 160;
            sp->unk33C.y = 43;
            sp->unk33C.w = 16;
            sp->unk33C.h = 8;
            func_8002916c(&sp->unk33C, &sp->unk32C, 3072);
        }
        if (DAT_8008818c != 25) {
            sp->unk33C.x = 160;
            sp->unk33C.y = 220;
            sp->unk33C.w = 16;
            sp->unk33C.h = 8;
            func_8002916c(&sp->unk33C, &sp->unk32C, 1024);
        }
    }
    sp->unk33C.x = 28;
    sp->unk33C.y = 48;
    sp->unk33C.w = 264;
    sp->unk33C.h = 168;
    sp->unk338.r = 80;
    sp->unk338.g = 88;
    sp->unk338.b = 96;
    if (DAT_800881a8 != 0)
        func_8002916c(&prev, &sp->unk32C, 2048);
    else
        func_8002916c(&next, &sp->unk32C, 0);
    func_800286a8(1, 0, 0);
    func_80027d18(&sp->unk33C, &sp->unk338, 257, 1);
    PAGE_END();
}

/*
 * The option page.
 *
 * Not exact: retail copies `wash` ({32, 32, 32}) from the .sdata constant
 * 0x80087f2c that st.text's SDATA claim owns (GCC merged it with
 * func_8002948c's, so the two share a translation unit); a copy here would
 * add 4 bytes to this unit's .sdata, so the source leaves it uninitialised.
 * Retail also schedules both `i = 0` stores and the first label loop's y
 * step differently.
 */
ADDRESS(0x8002b2e8, 0x564)
void func_8002b2e8(void)
{
    CVECTOR black = {0};
    CVECTOR shadow = {24, 24, 24};
    CVECTOR light = {96, 96, 96};
    /* Retail initialises it from st.text's .sdata constant 0x80087f2c. */
    CVECTOR wash;
    DVECTOR row[4] = {{48, 84}, {114, 84}, {32, 94}, {130, 94}};
    CVECTOR lit[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {128, 64, 0, 0}, {128, 64, 0, 0}};
    CVECTOR dim[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 64, 128, 0}, {0, 64, 128, 0}};
    u16 label_x[6] = {15, 12, 12, 19, 19, 5};
    MenuScratch *sp;
    u8 *on;
    int i;
    int y;

    func_800174c4();
    y = 80;
    sp = MENU_SCRATCH;
    sp->style[0].r = 72;
    sp->style[0].g = 72;
    sp->style[0].b = 72;
    sp->style[2].r = 98;
    sp->style[2].g = 66;
    sp->style[2].b = 48;
    sp->style[3].r = 24;
    sp->style[3].g = 24;
    sp->style[3].b = 24;
    sp->style[3].cd = 57;
    sp->style[2].cd = 57;
    sp->style[0].cd = 57;
    sp->unk358.vx = 600;
    sp->unk358.vy = 416;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.y = 83;
    sp->unk33C.w = 5;
    sp->unk33C.h = 5;
    sp->unk33C.x = DAT_801cd121 * 83 / 63 + 180;
    i = 0;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 0);
    sp->unk358.vx = 576;
    sp->unk33C.x = 176;
    sp->unk33C.y = 78;
    sp->unk33C.w = 96;
    sp->unk33C.h = 14;
    sp->unk358.vy = 416;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 1);
    for (; i < 6; i++, y += 20)
        func_8002732c(MESSAGE(0xc9 + i), label_x[i] + 32, y, &sp->style[2]);
    for (on = DAT_801cd122, i = 1; i < 5; on++, i++) {
        if (*on == 1) {
            func_8002732c(MESSAGE(0xcf), 193, i * 20 + 80, &sp->style[2]);
            func_8002732c(MESSAGE(0xd0), 241, i * 20 + 80, &sp->style[3]);
        } else {
            func_8002732c(MESSAGE(0xcf), 193, i * 20 + 80, &sp->style[3]);
            func_8002732c(MESSAGE(0xd0), 241, i * 20 + 80, &sp->style[2]);
        }
    }
    i = 0;
    func_800276bc(DAT_801cd121 | DIGITS(2), 280, 80, &sp->style[0]);
    func_8002732c(MESSAGE(0xe0 + sp->unk2E0), 64, 208, &sp->style[0]);
    sp->unk358.vx = 596;
    sp->unk358.vy = 448;
    sp->unk35C.vx = 544;
    sp->unk35C.vy = 511;
    sp->unk33C.x = 32;
    sp->unk33C.y = 44;
    sp->unk33C.w = 70;
    sp->unk33C.h = 16;
    func_80028364(&sp->unk33C, &sp->unk358, &sp->unk35C, NULL, 3);
    for (; i < 6; i++) {
        if (i == sp->unk2E0)
            func_80028c2c(row, lit, 1);
        else
            func_80028c2c(row, dim, 1);
        row[0].vy += 20;
        row[1].vy += 20;
        row[2].vy += 20;
        row[3].vy += 20;
    }
    func_800286a8(0, 0, 0);
    PAGE_END();
}
