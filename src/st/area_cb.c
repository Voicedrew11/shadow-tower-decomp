/*
 * ST.EXE: the area loader's step setters and CD completion callbacks, and
 * the commit of a finished area request (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/vis.h>
#include <st/area.h>

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
        DAT_801dd1cc[0] = 0;
        area_state.unk26 = DAT_801cd120[0];
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
