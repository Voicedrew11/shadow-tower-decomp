/*
 * VS.EXE: the team screen's picture load (WIP module): the title strips,
 * each team slot's picture and both players' fighter pictures go to VRAM
 * through the CD state's load buffer.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#define VS_PICKED_PAIR
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/funcs.h>

/* The load's RECT initializer: 8 bytes, so .sdata under -G8. */
SDATA(0x8006323c, 0x8)

int func_8002ee90(int archive, int member, u_long *buf);
void func_80021984(void);

/* Not exact: the title loop's index, x and buffer have different register
 * lifetimes; later loops derive the CD base from the buffer address.
 * The byte-extent shifts precede the TIM calls instead of their delay slots. */
ADDRESS(0x8001789c, 0x328)
void func_8001789c(void)
{
    RECT base = {0, 0, 640, 240};
    DVECTOR pos;
    RECT r;
    RECT r2;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    s32 *id;
    int n;
    int x;
    int slot;
    int i;

    sp->unk0.team.pos[0].vx = 640;
    sp->unk0.team.pos[0].vy = 144;
    sp->unk0.team.pos[1].vx = 640;
    sp->unk0.team.pos[1].vy = 192;
    sp->unk0.team.pos[2].vx = 736;
    sp->unk0.team.pos[2].vy = 144;
    sp->unk0.team.pos[3].vx = 736;
    sp->unk0.team.pos[3].vy = 192;
    sp->unk0.team.pos[4].vx = 832;
    sp->unk0.team.pos[4].vy = 144;
    sp->unk0.team.pos[5].vx = 832;
    sp->unk0.team.pos[5].vy = 192;
    sp->unk0.team.pos[6].vx = 928;
    sp->unk0.team.pos[6].vy = 144;
    sp->unk0.team.pos[7].vx = 928;
    sp->unk0.team.pos[7].vy = 172;
    sp->unk0.team.pos[8].vx = 928;
    sp->unk0.team.pos[8].vy = 200;
    sp->unk0.team.pos[15].vx = 640;
    sp->unk0.team.pos[15].vy = 0;
    DAT_801a4590.unk4A0 = DAT_801a4590.unk4A4;
    for (i = 0, x = 320; i < 5; i++, x += 64) {
        func_8002ee90(5, i + 31, DAT_801a4590.unk4A4);
        pos.vx = base.x + x;
        pos.vy = base.y;
        func_80012c2c(DAT_801a4590.unk4A4, &pos, NULL);
        DrawSync(0);
    }
    id = DAT_801624b8[0].slot;
    for (i = 0; i < 6; i++) {
        slot = *id++;
        if (slot == -1) {
            r2.x = sp->unk0.team.pos[i].vx;
            r2.y = sp->unk0.team.pos[i].vy;
            r2.w = 96;
            r2.h = 48;
            ClearImage(&r2, 0, 0, 0);
        } else {
            n = func_8002ee90(10, slot, DAT_801a4590.unk4A0) * 2048;
            func_80012c2c(DAT_801a4590.unk4A0, &sp->unk0.team.pos[i], NULL);
            DAT_801a4590.unk4A0 = (u_long *)((u8 *)DAT_801a4590.unk4A0 + n);
        }
    }
    for (i = 0; i < 2; i++) {
        if (DAT_80063330[i] == -1) {
            r.x = sp->unk0.team.pos[6 + i].vx;
            r.y = sp->unk0.team.pos[6 + i].vy;
            r.w = 56;
            r.h = 28;
            ClearImage(&r, 0, 0, 0);
        } else {
            n = func_8002ee90(11, DAT_80191d00.unk0118[DAT_80063330[i]].unk5[0],
                              DAT_801a4590.unk4A0) * 2048;
            func_80012c2c(DAT_801a4590.unk4A0, &sp->unk0.team.pos[6 + i], NULL);
            DAT_801a4590.unk4A0 = (u_long *)((u8 *)DAT_801a4590.unk4A0 + n);
        }
    }
    r.x = sp->unk0.team.pos[8].vx;
    r.y = sp->unk0.team.pos[8].vy;
    r.w = 56;
    r.h = 28;
    ClearImage(&r, 10, 10, 10);
    DrawSync(0);
    func_80021984();
    func_80021984();
}
