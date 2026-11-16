/*
 * ST.EXE: display setup, ordering-table frames and the camera matrix
 * (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <INLINE_C.H>
#include <st/memory.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/scratch.h>
#include <st/funcs.h>

#define OT_LENGTH 4096
#define OT2_LENGTH 64
#define PACKET_WORDS (0x20000 / 4)

ADDRESS(0x80016fb8, 0x210)
void func_80016fb8(void)
{
    int h = 240;
    int i;
    u8 *p;

    ResetGraph(3);
    SetGraphDebug(0);
    InitGeom();
    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&world_state.draw[0], 0, 0, 320, h);
    SetDefDispEnv(&world_state.disp[0], 0, h, 320, h);
    SetDefDrawEnv(&world_state.draw[1], 0, h, 320, h);
    SetDefDispEnv(&world_state.disp[1], 0, 0, 320, h);
    world_state.draw[0].dtd = 1;
    world_state.draw[1].dtd = 1;
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
    setRGB0(&world_state.draw[0], 0, 0, 0);
    setRGB0(&world_state.draw[1], 0, 0, 0);
    PutDispEnv(&world_state.disp[0]);
    PutDrawEnv(&world_state.draw[0]);
    gte_SetBackColor(80, 80, 80);
    world_state.frame = 255;
    world_state.unkE0 = DAT_8011c488;
    world_state.packets[0].start = world_state.unkE0;
    world_state.packets[0].end = world_state.packets[0].start + PACKET_WORDS;
    world_state.packets[1].start = world_state.packets[0].end;
    world_state.packets[1].end = world_state.packets[1].start + PACKET_WORDS;
    world_state.unk1ECCF = 0;
    world_state.unk1ECCD = 0;
    world_state.unk1ECCE = 0;
    p = world_state.unk1ECC5;
    i = 8;
    while (i--)
        *p++ = 255;
    func_80016878();
    world_state.unk1ECD2 = 255;
    world_state.unk1ECC3 = 13;
    world_state.unk1EE15 = 0;
    world_state.unk1EDF0 = 0;
    world_state.unk1EDF4 = 0;
    DAT_8015c488 = 0;
    func_800340c4();
    func_80034f48();
    world_state.render_mode = 17;
    world_state.unk1EDFD = 0;
    world_state.unk1EDFE = 0;
    world_state.unk1EE16 = 0;
}

ADDRESS(0x800171c8, 0xf4)
void func_800171c8(void)
{
    int h = 480;

    gte_SetGeomOffset(320, 240);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&world_state.draw[0], 0, 0, 640, h);
    SetDefDispEnv(&world_state.disp[0], 0, h, 640, h);
    SetDefDrawEnv(&world_state.draw[1], 0, 0, 640, h);
    SetDefDispEnv(&world_state.disp[1], 0, h, 640, h);
    world_state.disp[0].isinter = 1;
    world_state.draw[0].dfe = 0;
    world_state.draw[0].dtd = 1;
    world_state.draw[1].dtd = 1;
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
    setRGB0(&world_state.draw[0], 0, 0, 0);
    setRGB0(&world_state.draw[1], 0, 0, 0);
}

ADDRESS(0x800172bc, 0xec)
void func_800172bc(void)
{
    int h = 240;

    gte_SetGeomOffset(320, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&world_state.draw[0], 0, 0, 640, h);
    SetDefDispEnv(&world_state.disp[0], 0, h, 640, h);
    SetDefDrawEnv(&world_state.draw[1], 0, h, 640, h);
    SetDefDispEnv(&world_state.disp[1], 0, 0, 640, h);
    world_state.draw[0].dtd = 1;
    world_state.draw[1].dtd = 1;
    world_state.draw[0].isbg = 1;
    world_state.draw[1].isbg = 1;
    setRGB0(&world_state.draw[0], 0, 0, 0);
    setRGB0(&world_state.draw[1], 0, 0, 0);
}

ADDRESS(0x800173a8, 0x11c)
void func_800173a8(void)
{
    int h = 240;

    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&world_state.draw[0], 0, 0, 320, h);
    SetDefDispEnv(&world_state.disp[0], 0, h, 320, h);
    SetDefDrawEnv(&world_state.draw[1], 0, h, 320, h);
    SetDefDispEnv(&world_state.disp[1], 0, 0, 320, h);
    world_state.draw[0].dtd = 1;
    world_state.draw[1].dtd = 1;
    setRGB0(&world_state.draw[0], 0, 0, 0);
    setRGB0(&world_state.draw[1], 0, 0, 0);
    PutDrawEnv(&world_state.draw[0]);
    PutDrawEnv(&world_state.draw[1]);
    PutDispEnv(&world_state.disp[0]);
    PutDispEnv(&world_state.disp[1]);
}

ADDRESS(0x800174c4, 0xe0)
void func_800174c4(void)
{
    world_state.frame = !world_state.frame;
    world_state.packet = &world_state.packets[world_state.frame];
    world_state.packet->cur = world_state.packet->start;
    world_state.ot_cur = world_state.ot[world_state.frame];
    ClearOTagR(world_state.ot_cur, OT_LENGTH);
    world_state.ot2_cur = world_state.ot2[world_state.frame];
    ClearOTagR(world_state.ot2_cur, OT2_LENGTH);
    world_state.unk831C[0] = world_state.unk831C[1] =
        world_state.unk8324 = world_state.unk8328 = 0;
    world_state.unk832C = 0;
    world_state.unk8330 = 0;
    world_state.unk8338[0] = 0;
    world_state.unk8338[1] = 0;
    world_state.unk8338[2] = 0;
    world_state.unk8338[3] = 0;
    world_state.unk8338[4] = 0;
}

ADDRESS(0x800175a4, 0x90)
void func_800175a4(void)
{
    DrawSync(0);
    VSync(3);
    PutDispEnv(&world_state.disp[world_state.frame]);
    PutDrawEnv(&world_state.draw[world_state.frame]);
}

ADDRESS(0x80017634, 0x4c)
void func_80017634(void)
{
    AddPrims(world_state.ot_cur, &world_state.ot2_cur[OT2_LENGTH - 1],
             world_state.ot2_cur);
    DrawOTag(&world_state.ot_cur[OT_LENGTH - 1]);
}

ADDRESS(0x80017680, 0x28)
void func_80017680(void)
{
    func_800175a4();
    func_80017634();
}

ADDRESS(0x800176a8, 0xf8)
void func_800176a8(VECTOR *pos, SVECTOR *rot)
{
    SVECTOR angle;

    if (pos)
        world_state.unk1EDC8 = *pos;
    if (rot)
        world_state.unk1EDD8 = *rot;
    angle.vx = world_state.unk1EDD8.vx;
    angle.vz = world_state.unk1EDD8.vz;
    angle.vy = world_state.unk1EDD8.vy;
    RotMatrix(&angle, &world_state.unk1ECE8);
    func_80049cd4(angle.vx, &world_state.unk1EDA8);
    func_800498cc(&world_state.unk1ECE8, &world_state.unk1ED28);
}
