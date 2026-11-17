/*
 * VS.EXE: the round's frame (WIP module split from src/vs/main.c: it is not exact).
 * $gp-relative accesses to the round state, so -G8.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/main.h>
#include <vs/pad.h>
#include <vs/funcs.h>

/* Not reconstructed callees (src/vs/hud_80029f28.c and others). */
void func_8002a360(int kind, int level);
void func_8002ab58(int level, int wide);
void func_800297fc(int time);
void func_80029b10(int level);
void func_80029f28(int phase, int scale);
void func_80028fcc(int hold);
void func_8002bfe8(int n, int scale);
void func_8002d998(void);
void func_800237e4(void);
void func_800238c0(void);
void func_80028074(void);
void func_800280e0(void);
void func_800129bc(short volume);
int func_8002b078(int frame);
void func_8002ad40(int level);
void func_80025204(void);
void func_80028914(void);
void func_8002c2c4(int n);
void func_80035b68(void);
void func_800291d4(u_long *model, int flags, SVECTOR *a, SVECTOR *b, SVECTOR *c);

/* The callback's frame clock and the current screen's starting frame. */
#define FRAME_CLOCK() (DAT_801603c0.unk3C)
#define SCREEN_START() (DAT_80063314)
#define ELAPSED() (FRAME_CLOCK() - SCREEN_START())

/* Arena pieces beyond this distance from the centre are hidden. */
#define ARENA_EDGE 12000

/* The phase switch table. */
JUMPTABLES(0x800110c0, 0x50)

s32 DAT_80063220;
s32 DAT_80063224;
s32 DAT_80063228;
VsFighter *DAT_80063300;
u32 DAT_80063314;
s32 DAT_8006331c;
s32 DAT_80063324;
s32 DAT_80063338;
s32 DAT_80063350;
s32 DAT_80063360;
u32 DAT_80063364;
s32 DAT_8006336c;

/*
 * Draws one frame of the round: swaps the buffers, runs the round start
 * and the screen of the current phase (DAT_80063324: the captions and
 * menus func_8002a360 fades by level), the clock, both fighters, their
 * gauges and the arena (the pieces past the camera hidden), then shows the
 * frame and measures its length in fields (DAT_80063220, at most 5).
 * Not exact: caption 17's fade orders its elapsed/scroll arithmetic
 * differently, and cases 6/7 share a different caption tail (8 bytes short).
 */
ADDRESS(0x80015020, 0xe30)
void func_80015020(void)
{
    SVECTOR rot;
    SVECTOR pos;
    SVECTOR scale;
    StPacketBuffer *packet;
    int i;
    u8 mask;
    s16 a;
    s16 b;

    DAT_801624d0.frame = DAT_801624d0.frame == 0;
    packet = &DAT_801624d0.packets[DAT_801624d0.frame];
    DAT_801624d0.packet = packet;
    packet->cur = packet->start;
    DAT_801624d0.ot_cur = DAT_801624d0.ot[DAT_801624d0.frame];
    ClearOTagR(DAT_801624d0.ot_cur, VS_OT_LENGTH);
    DAT_801624d0.unk8328 = 0;
    DAT_801624d0.unk8324 = 0;
    DAT_801624d0.unk831C[1] = 0;
    DAT_801624d0.unk832C = 0;
    DAT_801624d0.unk8330 = 0;
    DAT_801624d0.unk8338[0] = 0;
    DAT_801624d0.unk8338[1] = 0;
    DAT_801624d0.unk8338[2] = 0;
    DAT_801624d0.unk8338[3] = 0;
    DAT_801624d0.unk8338[4] = 0;
    func_800237e4();
    func_800280e0();
    if (DAT_80063324 == 0 && func_8002b078(DAT_80063364++)) {
        DAT_8006331c = 3;
        DAT_80063324 = 1;
        DAT_80063364 = DAT_801603c0.unk3C + 1800;
        DAT_80063314 = DAT_801603c0.unk3C;
        while (!CdRead2(CdlModeSpeed | CdlModeRT | CdlModeSF))
            ;
        func_800129bc(64);
    }
    if (DAT_80063324 != 0)
        func_8002ad40(32);
    switch (DAT_80063324) {
    case 1: {
        u32 end;

        if (ELAPSED() < 16) {
            u32 fade = SCREEN_START();

            fade += 16;
            func_8002a360(DAT_8006331c, fade - FRAME_CLOCK());
            if (DAT_8006331c == 12) {
                u32 left;
                u32 elapsed = ELAPSED();

                func_8002a360(17, 16 - elapsed - DAT_8006336c + 2);
                func_8002a360(18, DAT_8006336c - ELAPSED() + 6);
                left = SCREEN_START();
                left += 16;
                func_8002ab58(left - FRAME_CLOCK(), 0);
            }
        }
        end = DAT_80063364;
        if (FRAME_CLOCK() < end)
            func_800297fc(end - FRAME_CLOCK());
        else
            func_800297fc(0);
        break;
    }
    case 2:
        if (ELAPSED() < 16) {
            func_8002a360(12, ELAPSED());
            func_8002a360(17, ELAPSED());
            func_8002a360(18, ELAPSED() / 3);
            func_8002ab58(ELAPSED(), 0);
        } else {
            func_8002a360(12, 16);
            func_8002a360(17, 16 - DAT_8006336c);
            func_8002a360(18, DAT_8006336c + 6);
            func_8002ab58(16, 0);
        }
        func_800297fc(DAT_80063364);
        break;
    case 4:
    draw:
        if (ELAPSED() <= 200)
            func_8002a360(0, ELAPSED());
        if (ELAPSED() > 200) {
            if (DAT_80063338 == DAT_80063228 - 1)
                func_8002a360(0, 216 - ELAPSED());
            else
                func_8002a360(0, 16);
        }
        func_800297fc(DAT_80063364);
        break;
    case 5:
        if (ELAPSED() < 30) {
            func_8002a360(5, ELAPSED());
            func_800297fc(0);
            break;
        }
        if (ELAPSED() < 46) {
            u32 left = SCREEN_START();

            left += 46;
            func_8002a360(5, left - FRAME_CLOCK());
            func_800297fc(0);
            break;
        }
        a = (DAT_800633b0.fighter[0].unk4DF08 << 12) / DAT_800633b0.fighter[0].unk4DF18.unk1B8;
        b = (DAT_800633b0.fighter[1].unk4DF08 << 12) / DAT_800633b0.fighter[1].unk4DF18.unk1B8;
        DAT_80063364 = 0;
        DAT_80063314 = DAT_801603c0.unk3C;
        if (a == b) {
            DAT_80063324 = 4;
            goto draw;
        }
        if (b < a) {
            DAT_80063324 = 8;
            func_8001336c(1, 1);
        } else {
            DAT_80063324 = 9;
            func_8001336c(0, 1);
        }
        goto result;
    case 6:
    case 7:
        if (ELAPSED() < 30) {
            func_8002a360(1, ELAPSED());
            func_8002a360(4, ELAPSED());
        } else if (ELAPSED() < 46) {
            u32 left = SCREEN_START();

            left += 46;
            func_8002a360(1, left - FRAME_CLOCK());
            left = SCREEN_START();
            left += 46;
            func_8002a360(4, left - FRAME_CLOCK());
        } else {
            DAT_80063314 = DAT_801603c0.unk3C;
            DAT_80063324 += 2;
            func_8001336c(9 - DAT_80063324, 1);
            goto result;
        }
        func_800297fc(DAT_80063364);
        break;
    case 8:
    case 9:
    result:
        if (ELAPSED() > 30 && ELAPSED() < 159)
            DAT_800633b0.fighter[~DAT_80063324 & 1].unk4DF12 =
                DAT_80063314 - (DAT_801603c0.unk3C - 158);
        if (ELAPSED() > 100 && ELAPSED() <= 200)
            func_8002a360(2, ELAPSED() - 100);
        if (ELAPSED() > 200) {
            if (DAT_80063338 == DAT_80063228 - 1)
                func_8002a360(2, 216 - ELAPSED());
            else
                func_8002a360(2, 16);
        }
        func_800297fc(DAT_80063364);
        break;
    case 16:
        func_8002a360(9, ELAPSED());
        func_8002a360(19, ELAPSED());
        func_8002ab58(ELAPSED(), 1);
        func_800297fc(DAT_80063364);
        break;
    case 17:
        func_8002a360(19, 16 - ELAPSED());
        func_8002a360(9, 16 - ELAPSED());
        func_8002a360(15, ELAPSED());
        func_8002a360(14, ELAPSED());
        func_8002a360(11, ELAPSED());
        func_8002ab58(11, 1);
        func_800297fc(DAT_80063364);
        break;
    case 18:
        if (ELAPSED() >= 20)
            goto menu;
        func_8002a360(19, 16 - ELAPSED());
        func_8002a360(9, 16 - ELAPSED());
        func_8002a360(10, ELAPSED() - 4);
        func_8002a360(16, ELAPSED() - 4);
        func_8002a360(13, ELAPSED() - 4);
        func_8002a360(17, ELAPSED() - 4);
        func_8002a360(18, ELAPSED() / 3);
        func_8002ab58(16, 1);
        func_800297fc(DAT_80063364);
        break;
    case 19:
        func_8002a360(10, 16 - ELAPSED());
        func_8002a360(16, 16 - ELAPSED());
        func_8002a360(13, 16 - ELAPSED());
        func_8002a360(17, 16 - ELAPSED() - DAT_8006336c + 2);
        func_8002a360(18, DAT_8006336c - ELAPSED() + 6);
        func_8002a360(19, ELAPSED() - 4);
        func_8002a360(9, ELAPSED() - 4);
        func_8002ab58(16, 1);
        func_800297fc(DAT_80063364);
        break;
    case 20:
        if (ELAPSED() < 20) {
            func_8002a360(11, 16 - ELAPSED());
            func_8002a360(15, 16 - ELAPSED());
            func_8002a360(14, 16 - ELAPSED());
            func_8002a360(10, ELAPSED() - 4);
            func_8002a360(16, ELAPSED() - 4);
            func_8002a360(13, ELAPSED() - 4);
            func_8002a360(17, ELAPSED() - 4);
            func_8002a360(18, ELAPSED() / 3);
        } else {
        menu:
            func_8002a360(10, 16);
            func_8002a360(16, 16);
            func_8002a360(13, 16);
            func_8002a360(17, 16 - DAT_8006336c);
            func_8002a360(18, DAT_8006336c + 6);
        }
        func_8002ab58(16, 1);
        func_800297fc(DAT_80063364);
        break;
    }
    if (DAT_80063324 != 0) {
        for (i = 0; i < 2; i++) {
            DAT_80063300 = &DAT_800633b0.fighter[i];
            switch (DAT_80063300->unk4DF10) {
            case 1:
                func_80025204();
                break;
            case 2:
                DAT_80063360 = 0;
                func_80028914();
                break;
            }
            if (DAT_80063300->unk4DF14) {
                func_8002c2c4(i);
                DAT_80063300->unk4DF14--;
            }
            func_8002bfe8(i, 4096);
        }
        func_80029b10(32);
        func_80029f28(DAT_801603c0.unk3C % 96, 32);
    }
    func_80028fcc(DAT_80063324 == 2 || DAT_80063324 >= 16);
    rot.vz = 0;
    rot.vy = 0;
    rot.vx = 0;
    pos.vz = 0;
    pos.vy = 0;
    pos.vx = 0;
    scale.vz = 4;
    scale.vy = 4;
    scale.vx = 4;
    func_80035b68();
    func_8002d998();
    DAT_80063224 = 2;
    func_800291d4(DAT_80117708, 0, &rot, &rot, &scale);
    DAT_80063224 = 4;
    func_800291d4((u_long *)DAT_8015c528, 0, &rot, &rot, &scale);
    DAT_80063224 = 3;
    mask = 0;
    if (DAT_801624d0.unk2CA78.vz > ARENA_EDGE)
        mask = 0x83;
    if (DAT_801624d0.unk2CA78.vx > ARENA_EDGE)
        mask |= 0x0e;
    if (DAT_801624d0.unk2CA78.vz < -ARENA_EDGE)
        mask |= 0x38;
    if (DAT_801624d0.unk2CA78.vx < -ARENA_EDGE)
        mask |= 0xe0;
    for (i = 0; i < 8; i++) {
        DAT_80063350 = mask & (1 << i);
        func_800291d4((u_long *)DAT_8015d8b0, 0, &rot, &pos, &scale);
        pos.vy += 512;
    }
    DAT_80063224 = 0;
    DrawSync(0);
    DAT_80063220 = VSync(0) / 256 + 1;
    if (DAT_80063220 >= 6)
        DAT_80063220 = 5;
    FntFlush(-1);
    PutDispEnv(&DAT_801624d0.disp[DAT_801624d0.frame]);
    PutDrawEnv(&DAT_801624d0.draw[DAT_801624d0.frame]);
    DrawOTag(DAT_801624d0.ot_cur + VS_OT_LENGTH - 1);
    func_800238c0();
    func_80028074();
    DAT_801624d0.unk2C970++;
}
