/*
 * VS.EXE: the item list page, its caption bar and detail panel, and the
 * two-tile emblem panel (WIP module). func_80022794 is VS's copy of
 * ST.EXE's func_800319a8 (src/st/ui.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/card.h>
#include <vs/menu.h>
#include <vs/funcs.h>

/* The module's initializers (the light grey shared by func_80021cfc and
 * func_80022794): at most 8 bytes, so .sdata under -G8. */
SDATA(0x80063290, 0x28)

/*
 * The item list page: the cursor bar, the scroll arrows and thumb, the
 * header panel, then eleven rows from the list's first shown row: each
 * item's name (500 for the -2 entry, a plain message) and its icon from
 * the kind its id range gives.
 */
ADDRESS(0x80021cfc, 0x52c)
void func_80021cfc(void)
{
    RECT unused0;
    CVECTOR unused1 = {96, 96, 96};
    RECT up = {26, 24, 24, 8};
    RECT down = {26, 220, 24, 8};
    RECT thumb = {16, 31, 12, 3};
    CVECTOR dark = {40, 40, 48};
    RECT edge;
    RECT unused2;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int i;
    u8 kind;

    sp->unk218.x = 48;
    sp->unk218.w = 270;
    sp->unk218.h = 16;
    sp->unk218.y = sp->unk1D0 * 16 + 35;
    func_8001de8c(&sp->unk218, &sp->unk214, 513, 1);
    if (sp->unk0.menu.grid.row0 > 0)
        func_8001eef4(&up, &sp->unk214, 3072);
    if (sp->unk0.menu.grid.row0 < sp->unk0.menu.grid.row1 - 11)
        func_8001eef4(&down, &sp->unk214, 1024);
    thumb.y = sp->unk0.menu.grid.row0 * 190 / sp->unk0.menu.grid.row1 + 31;
    func_8001e3d8(&thumb, &dark, 1);
    func_8001e890(1, 0, 0);
    edge.x = thumb.x - 2;
    edge.y = thumb.y - 1;
    edge.w = thumb.w;
    edge.h = thumb.h;
    func_8001e3d8(&edge, &dark, 1);
    func_8001e890(2, 0, 0);
    sp->unk224.x = 60;
    sp->unk224.y = 12;
    sp->unk224.w = 196;
    sp->unk224.h = 16;
    sp->unk240.vx = 896;
    sp->unk240.vy = 448;
    sp->unk244.vx = 544;
    sp->unk244.vy = 502;
    func_8001e530(&sp->unk224, &sp->unk240, &sp->unk244, NULL, 3);
    sp->unk238.x = 50;
    sp->unk238.y = 35;
    sp->unk238.w = 196;
    sp->unk238.h = 16;
    for (i = 0; i < 11; i++) {
        int id = sp->unk0.menu.grid.ids0[sp->unk0.menu.grid.row0 + i];
        if (id == -2)
            id = 500;
        if (id != -1) {
            if (id != 500) {
                sp->style[0].cd |= 0x80;
                func_8001d8b8(VS_ITEM_NAME(id), sp->unk238.x, sp->unk238.y + 2, &sp->style[0]);
                sp->style[0].cd &= 0x7f;
            } else {
                func_8001d8b8(VS_MESSAGE(500), sp->unk238.x + 39, sp->unk238.y + 2,
                              &sp->style[0]);
            }
        }
        sp->unk238.y += 16;
    }
    sp->unk238.x = 34;
    sp->unk238.y = 35;
    sp->unk238.w = 14;
    sp->unk238.h = 13;
    for (i = 0; i < 11; i++) {
        int id = sp->unk0.menu.grid.ids0[sp->unk0.menu.grid.row0 + i];
        if (id < 70)
            kind = 0;
        else if (id < 91)
            kind = 5;
        else if (id < 121)
            kind = 1;
        else if (id < 141)
            kind = 8;
        else if (id < 161)
            kind = 7;
        else if (id < 196)
            kind = 2;
        else if (id < 231)
            kind = 3;
        else if (id < 251)
            kind = 6;
        else if (id < 266)
            kind = 4;
        if (kind >= 5) {
            sp->unk240.vx = (kind - 5) * 14 + 640;
            sp->unk240.vy = 256;
        } else {
            sp->unk240.vx = kind * 14 + 640;
            sp->unk240.vy = 240;
        }
        if (id != -1 && id != -2)
            func_8001e530(&sp->unk238, &sp->unk240, NULL, NULL, 16);
        sp->unk238.y += 16;
    }
    func_8001f23c(10, 9, 312, 222);
}

/* The caption bar: the selected entry's item name. */
ADDRESS(0x80022228, 0xb0)
void func_80022228(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int id = sp->unk0.menu.grid.ids0[sp->unk0.menu.grid.row0 * 4 + sp->unk1D0];

    sp->style[0].cd |= 0x80;
    if (id != -1)
        func_8001d8b8(VS_ITEM_NAME(id), 14, 200, &sp->style[0]);
    sp->style[0].cd &= 0x7f;
    func_8001f23c(10, 198, 620, 34);
}

/*
 * The detail panel of the selected entry: the image panels while the CD
 * queue is idle, up to fourteen nonzero part counts of its item record
 * (four columns of four), the count the opponent's save holds, the
 * record's +0x27 value and its weight (+0x24 / 4.54, with tenths).
 */
/* Not exact (0x4c0 of 0x4bc bytes): retail reads the left column x from
 * its .sdata initializer through an absolute address held like a spilled
 * constant (this build addresses the static $gp-relative), and has a
 * 96-byte frame with y and id at 24/32 (this build: 88). */
ADDRESS(0x800222d8, 0x4bc)
void func_800222d8(int page)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    static const DVECTOR pos = {342, 0};
    int y = 135;
    int left = 342;
    s16 id;
    int x;
    int n;
    int row;
    int col;
    int message;
    int line;
    u8 *counts;
    Vis44 *item;
    int weight;

    sp->unk238.w = 192;
    sp->unk238.h = 124;
    sp->unk238.x = 334;
    sp->unk238.y = 9;
    id = sp->unk0.menu.grid.ids0[(s16)(sp->unk0.menu.grid.row0 + sp->unk1D0)];
    if (id != -1 && id != -2) {
        if ((s16)func_8001ca18() == 0) {
            sp->unk240.vx = 640;
            sp->unk240.vy = 0;
            func_8001e530(&sp->unk238, &sp->unk240, NULL, NULL, 16);
            sp->unk238.w = 100;
            sp->unk240.vx = 832;
            sp->unk238.x += 192;
            func_8001e530(&sp->unk238, &sp->unk240, NULL, NULL, 16);
        }
        n = 0;
        message = 2;
        x = left;
        counts = DAT_80191d00.unk0118[id].unk10;
        DAT_8006337a = 1;
        for (row = 0; row < 4; row++) {
            line = y;
            for (col = 0; col < 4; col++) {
                if (n == 14)
                    goto done;
                n++;
                if (*counts != 0) {
                    func_8001d8b8(VS_MESSAGE(200 + message), x, line, &sp->style[0]);
                    func_8001d9cc(*counts | 0x1300000, x + 42, line, &sp->style[3]);
                }
                counts++;
                message++;
                line += 15;
            }
            if (n == 14)
                break;
            x += 70;
        }
    done:
        DAT_8006337a = 0;
        y += 60;
        func_8001d8b8(VS_MESSAGE(501), pos.vx, y, &sp->style[0]);
        func_8001d9cc(DAT_801a0a80[1 - page].unk1BA8[id] | 0x3000000, left + 70, y, &sp->style[3]);
        func_8001d8b8(VS_MESSAGE(502), left + 114, y, &sp->style[0]);
        item = &DAT_80191d00.unk0118[id];
        func_8001d9cc(item->unk27 | 0x3000000, left + 138, y, &sp->style[3]);
        weight = item->unk24;
        y += 15;
        func_8001d8b8(VS_MESSAGE(503), pos.vx, y, &sp->style[0]);
        func_8001d9cc((u8)(weight * 100 / 454) | 0x3000000, left + 70, y, &sp->style[3]);
        func_8001d3f0(18, left + 109, y, &sp->style[0]);
        func_8001d9cc((u8)(weight * 1000 / 454 % 10) | 0x1000000, left + 115, y, &sp->style[3]);
        func_8001d3f0(53, left + 132, y, &sp->style[0]);
    }
    func_8001f23c(334, 9, 296, 222);
}

/* A two-tile emblem with its caption in a framed panel. */
ADDRESS(0x80022794, 0x144)
void func_80022794(void)
{
    CVECTOR light = {96, 96, 96};
    CVECTOR grey = {80, 80, 80};
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    sp->unk240.vx = 896;
    sp->unk240.vy = 480;
    sp->unk244.vx = 544;
    sp->unk244.vy = 503;
    sp->unk238.w = 32;
    sp->unk238.h = 16;
    sp->unk238.x = 353;
    sp->unk238.y = 168;
    func_8001e530(&sp->unk238, &sp->unk240, &sp->unk244, NULL, 0);
    sp->unk240.vx = 904;
    sp->unk244.vy = 504;
    sp->unk238.x = 232;
    sp->unk238.y = 168;
    func_8001e530(&sp->unk238, &sp->unk240, &sp->unk244, NULL, 0);
    func_8001d8b8(VS_MESSAGE(216), 188, 150, &sp->style[0]);
    func_8001f23c(168, 144, 319, 44);
}
