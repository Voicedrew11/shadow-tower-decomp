/*
 * ST.EXE: loading the current area set again and waiting for it, and
 * moving the viewer (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/area.h>

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
    viewer.x += dx;
    viewer.z += dz;
    viewer.y += dy;
}
