/*
 * VS.EXE: the team screen (WIP module): lays out the picture positions,
 * lists the fighters `player` owns, loads the message tables, fonts and
 * pictures, runs the screen (func_800182ec) and then reloads the menu
 * tables and both players' fighter pictures.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <MALLOC.H>
#define VS_PICKED_PAIR
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/card.h>
#include <vs/funcs.h>

int func_8002ee90(int archive, int member, u_long *buf);
u32 *func_8002e630(u32 *dst, u32 *src, int count);
void func_8001d39c(void);
void func_800182ec(VsMenuScratch *sp, int player);

/* The picture after a loaded message file: its byte size, then the file. */
#define TIM_AFTER(buf) ((u_long *)((u8 *)&(buf)[1] + (buf)[0]) + 1)

/* Not exact (one attempt; 83.5%): first divergence at +0x15c, the owned-bit
 * test (retail keeps the 1 in $a0 and i / 32 in $v0); retail also forms
 * the first two pictures' address as (size + 4) + buffer, then + 4, and
 * rematerializes the buffer address afterwards. */
ADDRESS(0x80018c84, 0x764)
void func_80018c84(int player)
{
    DVECTOR tex = {896, 448};
    DVECTOR clut = {544, 502};
    u8 unused[64];
    DVECTOR pos;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u_long *buf;
    u32 *owned;
    int size;
    int n;
    int i;

    sp->unk1D6 = 0;
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
    sp->unk0.team.pos[5].vx = 928;
    sp->unk0.team.pos[5].vy = 0;
    sp->unk0.team.pos[6].vx = 928;
    sp->unk0.team.pos[6].vy = 48;
    sp->unk0.team.pos[7].vx = 928;
    sp->unk0.team.pos[7].vy = 96;
    sp->unk0.team.pos[8].vx = 928;
    sp->unk0.team.pos[8].vy = 144;
    sp->unk0.team.pos[9].vx = 928;
    sp->unk0.team.pos[9].vy = 192;
    sp->unk0.team.pos[10].vx = 640;
    sp->unk0.team.pos[10].vy = 256;
    sp->unk0.team.pos[11].vx = 640;
    sp->unk0.team.pos[11].vy = 304;
    sp->unk0.team.pos[12].vx = 640;
    sp->unk0.team.pos[12].vy = 352;
    sp->unk0.team.pos[13].vx = 640;
    sp->unk0.team.pos[13].vy = 400;
    sp->unk0.team.pos[14].vx = 640;
    sp->unk0.team.pos[14].vy = 448;
    sp->unk0.team.pos[15].vx = 640;
    sp->unk0.team.pos[15].vy = 0;
    sp->unk0.team.kinds = func_8002398c(151);
    sp->unk0.team.count = 0;
    for (i = 0; i < 151; i++)
        sp->unk0.team.kinds[i] = 0xff;
    owned = DAT_801a0a80[player].unk18BC.unk0;
    for (i = 0; i < 151; i++) {
        if ((owned[i / 32] & (1 << (i % 32))) && i != 3 &&
            i != 5 && i != 6 && i != 7 && i != 9 && i != 10 && i != 11 && i != 19 &&
            i != 25 && i != 26 && i != 33 && i != 38 && i != 42 && i != 54 && i != 55 &&
            i != 67 && i != 70 && i != 73 && i != 83 && i != 84 && i != 85 && i != 86 &&
            i != 89 && i != 90 && i != 91 && i != 92 && i != 106 && i != 108 && i != 109 &&
            i != 116 && i != 118 && i != 131 && i != 132 && i != 133 && i != 135 &&
            i != 140 && i != 142 && i != 150)
            sp->unk0.team.kinds[sp->unk0.team.count++] = i;
    }
    sp->unk0.team.rows = sp->unk0.team.count / 4;
    sp->unk0.team.last = sp->unk0.team.count % 4;
    if (sp->unk0.team.last == 0) {
        sp->unk0.team.last = 4;
        sp->unk0.team.rows--;
    }
    sp->unk0.team.items = func_8002398c(0x2000);
    func_8002ee90(10, 400, (u_long *)sp->unk0.team.items);
    buf = DAT_801a4590.unk4A4;
    func_8002ee90(8, 2, buf);
    func_8002e630((u32 *)&DAT_80196230[1], (u32 *)&buf[1], 0x1500);
    func_80012c2c(TIM_AFTER(buf), &sp->unk1E0, &sp->unk1E4);
    DrawSync(0);
    func_8002ee90(8, 1, buf);
    func_8002e630((u32 *)&DAT_80196230[0], (u32 *)&buf[1], 0x1500);
    func_80012c2c(TIM_AFTER(buf), &sp->unk1D8, &sp->unk1DC);
    DrawSync(0);
    func_8002ee90(8, 7, buf);
    func_80012c2c(buf, &tex, &clut);
    DrawSync(0);
    DAT_801a4590.unk4A0 = buf;
    n = 12;
    if (sp->unk0.team.count < 13)
        n = sp->unk0.team.count;
    for (i = 0; i < n; i++) {
        size = func_8002ee90(10, sp->unk0.team.kinds[i], DAT_801a4590.unk4A0) * 512;
        func_80012c2c(DAT_801a4590.unk4A0, &sp->unk0.team.pos[i], NULL);
        DAT_801a4590.unk4A0 += size;
    }
    if (DrawSync(1) > 8)
        DrawSync(0);
    DAT_801a4590.unk4A0 = DAT_801a4590.unk4A4;
    for (i = 0; i < 3; i++) {
        if (DAT_801624b8[player].slot[i] != -1) {
            size = func_8002ee90(10, DAT_801624b8[player].slot[i], DAT_801a4590.unk4A0) * 512;
            func_80012c2c(DAT_801a4590.unk4A0, &sp->unk0.team.pos[12 + i], NULL);
            DAT_801a4590.unk4A0 += size;
        }
    }
    func_8001d39c();
    DrawSync(0);
    sp->unk1CC = 1;
    sp->unk1D0 = 0;
    sp->unk1D2 = 0;
    func_800182ec(sp, player);
    free(sp->unk0.team.kinds);
    free(sp->unk0.team.items);
    buf = DAT_801a4590.unk4A4;
    pos.vx = 640;
    pos.vy = 124;
    DAT_801a4590.unk4A0 = buf;
    func_8002ee90(8, 4, buf);
    func_8002e630((u32 *)&DAT_80196230[1], (u32 *)&buf[1], 0x1500);
    func_80012c2c(TIM_AFTER(buf), &sp->unk1E0, &sp->unk1E4);
    DrawSync(0);
    func_8002ee90(8, 8, buf);
    func_8002e630((u32 *)&DAT_80196230[0], (u32 *)&buf[1], 0x1500);
    func_80012c2c(TIM_AFTER(buf), &sp->unk1D8, &sp->unk1DC);
    DrawSync(0);
    func_8002ee90(8, 7, buf);
    func_80012c2c(buf, &tex, &clut);
    DrawSync(0);
    if (DAT_80063330[0] != -1) {
        func_8002ee90(11, DAT_80191d00.unk0118[DAT_80063330[0]].unk5[0], buf);
        func_80012c2c(buf, &pos, NULL);
        DrawSync(0);
    }
    pos.vy += 28;
    if (DAT_80063330[1] != -1) {
        func_8002ee90(11, DAT_80191d00.unk0118[DAT_80063330[1]].unk5[0], buf);
        func_80012c2c(buf, &pos, NULL);
        DrawSync(1);
    }
    sp->unk1D6 = 0;
}
