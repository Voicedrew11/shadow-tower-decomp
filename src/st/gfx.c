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
#include <st/state.h>
#include <st/game.h>

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
    SetDefDrawEnv(&DAT_8015d090.draw[0], 0, 0, 320, h);
    SetDefDispEnv(&DAT_8015d090.disp[0], 0, h, 320, h);
    SetDefDrawEnv(&DAT_8015d090.draw[1], 0, h, 320, h);
    SetDefDispEnv(&DAT_8015d090.disp[1], 0, 0, 320, h);
    DAT_8015d090.draw[0].dtd = 1;
    DAT_8015d090.draw[1].dtd = 1;
    DAT_8015d090.draw[0].isbg = 1;
    DAT_8015d090.draw[1].isbg = 1;
    setRGB0(&DAT_8015d090.draw[0], 0, 0, 0);
    setRGB0(&DAT_8015d090.draw[1], 0, 0, 0);
    PutDispEnv(&DAT_8015d090.disp[0]);
    PutDrawEnv(&DAT_8015d090.draw[0]);
    gte_SetBackColor(80, 80, 80);
    DAT_8015d090.id = 255;
    DAT_8015d090.unkE0 = DAT_8011c488;
    DAT_8015d090.packets[0].start = DAT_8015d090.unkE0;
    DAT_8015d090.packets[0].end = DAT_8015d090.packets[0].start + PACKET_WORDS;
    DAT_8015d090.packets[1].start = DAT_8015d090.packets[0].end;
    DAT_8015d090.packets[1].end = DAT_8015d090.packets[1].start + PACKET_WORDS;
    DAT_8015d090.unk1ECCF = 0;
    DAT_8015d090.unk1ECCD = 0;
    DAT_8015d090.unk1ECCE = 0;
    p = DAT_8015d090.unk1ECC5;
    i = 8;
    while (i--)
        *p++ = 255;
    func_80016878();
    DAT_8015d090.unk1ECD2 = 255;
    DAT_8015d090.unk1ECC3 = 13;
    DAT_8015d090.unk1EE15 = 0;
    DAT_8015d090.unk1EDF0 = 0;
    DAT_8015d090.unk1EDF4 = 0;
    DAT_8015c488 = 0;
    func_800340c4();
    func_80034f48();
    DAT_8015d090.unk1EDFC = 17;
    DAT_8015d090.unk1EDFD = 0;
    DAT_8015d090.unk1EDFE = 0;
    DAT_8015d090.unk1EE16 = 0;
}

ADDRESS(0x800171c8, 0xf4)
void func_800171c8(void)
{
    int h = 480;

    gte_SetGeomOffset(320, 240);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&DAT_8015d090.draw[0], 0, 0, 640, h);
    SetDefDispEnv(&DAT_8015d090.disp[0], 0, h, 640, h);
    SetDefDrawEnv(&DAT_8015d090.draw[1], 0, 0, 640, h);
    SetDefDispEnv(&DAT_8015d090.disp[1], 0, h, 640, h);
    DAT_8015d090.disp[0].isinter = 1;
    DAT_8015d090.draw[0].dfe = 0;
    DAT_8015d090.draw[0].dtd = 1;
    DAT_8015d090.draw[1].dtd = 1;
    DAT_8015d090.draw[0].isbg = 1;
    DAT_8015d090.draw[1].isbg = 1;
    setRGB0(&DAT_8015d090.draw[0], 0, 0, 0);
    setRGB0(&DAT_8015d090.draw[1], 0, 0, 0);
}

ADDRESS(0x800172bc, 0xec)
void func_800172bc(void)
{
    int h = 240;

    gte_SetGeomOffset(320, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&DAT_8015d090.draw[0], 0, 0, 640, h);
    SetDefDispEnv(&DAT_8015d090.disp[0], 0, h, 640, h);
    SetDefDrawEnv(&DAT_8015d090.draw[1], 0, h, 640, h);
    SetDefDispEnv(&DAT_8015d090.disp[1], 0, 0, 640, h);
    DAT_8015d090.draw[0].dtd = 1;
    DAT_8015d090.draw[1].dtd = 1;
    DAT_8015d090.draw[0].isbg = 1;
    DAT_8015d090.draw[1].isbg = 1;
    setRGB0(&DAT_8015d090.draw[0], 0, 0, 0);
    setRGB0(&DAT_8015d090.draw[1], 0, 0, 0);
}

ADDRESS(0x800173a8, 0x11c)
void func_800173a8(void)
{
    int h = 240;

    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(300);
    SetDefDrawEnv(&DAT_8015d090.draw[0], 0, 0, 320, h);
    SetDefDispEnv(&DAT_8015d090.disp[0], 0, h, 320, h);
    SetDefDrawEnv(&DAT_8015d090.draw[1], 0, h, 320, h);
    SetDefDispEnv(&DAT_8015d090.disp[1], 0, 0, 320, h);
    DAT_8015d090.draw[0].dtd = 1;
    DAT_8015d090.draw[1].dtd = 1;
    setRGB0(&DAT_8015d090.draw[0], 0, 0, 0);
    setRGB0(&DAT_8015d090.draw[1], 0, 0, 0);
    PutDrawEnv(&DAT_8015d090.draw[0]);
    PutDrawEnv(&DAT_8015d090.draw[1]);
    PutDispEnv(&DAT_8015d090.disp[0]);
    PutDispEnv(&DAT_8015d090.disp[1]);
}

ADDRESS(0x800174c4, 0xe0)
void func_800174c4(void)
{
    DAT_8015d090.id = !DAT_8015d090.id;
    DAT_8015d090.packet = &DAT_8015d090.packets[DAT_8015d090.id];
    DAT_8015d090.packet->cur = DAT_8015d090.packet->start;
    DAT_8015d090.ot_cur = DAT_8015d090.ot[DAT_8015d090.id];
    ClearOTagR(DAT_8015d090.ot_cur, OT_LENGTH);
    DAT_8015d090.ot2_cur = DAT_8015d090.ot2[DAT_8015d090.id];
    ClearOTagR(DAT_8015d090.ot2_cur, OT2_LENGTH);
    DAT_8015d090.unk831C[0] = DAT_8015d090.unk831C[1] =
        DAT_8015d090.unk831C[2] = DAT_8015d090.unk831C[3] = 0;
    DAT_8015d090.unk832C = 0;
    DAT_8015d090.unk8330 = 0;
    DAT_8015d090.unk8338[0] = 0;
    DAT_8015d090.unk8338[1] = 0;
    DAT_8015d090.unk8338[2] = 0;
    DAT_8015d090.unk8338[3] = 0;
    DAT_8015d090.unk8338[4] = 0;
}

ADDRESS(0x800175a4, 0x90)
void func_800175a4(void)
{
    DrawSync(0);
    VSync(3);
    PutDispEnv(&DAT_8015d090.disp[DAT_8015d090.id]);
    PutDrawEnv(&DAT_8015d090.draw[DAT_8015d090.id]);
}

ADDRESS(0x80017634, 0x4c)
void func_80017634(void)
{
    AddPrims(DAT_8015d090.ot_cur, &DAT_8015d090.ot2_cur[OT2_LENGTH - 1],
             DAT_8015d090.ot2_cur);
    DrawOTag(&DAT_8015d090.ot_cur[OT_LENGTH - 1]);
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
        DAT_8015d090.unk1EDC8 = *pos;
    if (rot)
        DAT_8015d090.unk1EDD8 = *rot;
    angle.vx = DAT_8015d090.unk1EDD8.vx;
    angle.vz = DAT_8015d090.unk1EDD8.vz;
    angle.vy = DAT_8015d090.unk1EDD8.vy;
    RotMatrix(&angle, &DAT_8015d090.unk1ECE8);
    func_80049cd4(angle.vx, &DAT_8015d090.unk1EDA8);
    func_800498cc(&DAT_8015d090.unk1ECE8, &DAT_8015d090.unk1ED28);
}
