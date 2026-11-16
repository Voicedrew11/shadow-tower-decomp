/*
 * ST.EXE player stats: clearing, equipment bonus totals, hit and magic points
 * (WIP module; part of src/st/player.c's translation unit).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <kf/lib/null.h>
#include <RAND.H>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <INLINE_C.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/funcs.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/player.h>
#include <st/scratch.h>
#include <st/vis.h>

RODATA(0x80011cfc, 0x20)

ADDRESS(0x8003c95c, 0x188)
void func_8003c95c(int r, int g, int b, int arg3, int arg4)
{
    func_80015ad4();
    do {
        func_8001531c();
        func_800456f8();
    } while (area_state.unk0 != 0);
    area_state.unk4 = 99;
    area_state.unk5 = 99;
    area_state.unk6 = 99;
    area_state.unk7 = 99;
    area_state.unk8 = 99;
    func_80045044(r, g, b, 255, 255, 127, 127, 127, 0, 0, 0);
    do {
        func_8001531c();
        func_800456f8();
    } while (area_state.unk0 != 0);
    func_80015ad4();
    VSync(2);
    func_8003eaf8();
    lighting_load_defaults();
    func_80016508();
    lighting_build_matrices();
    func_800500ac(130, 4096, 4096, 0);
    func_80015ad4();
    area_state.unk18 = cd_state.unk4;
    func_80045044(255, 255, 255, arg3, arg4, 127, 127, 127, 0, 0, 0);
    lighting_clear_requests();
}

ADDRESS(0x8003cae4, 0xa4)
void func_8003cae4(void)
{
    int i;

    viewer.unk8 = 0;
    i = 14;
    while (i--) {
        viewer.unk9A[i] = 50;
        viewer.unk62[i] = 50;
        viewer.unk7E[i] = 0;
    }
    viewer.unk26 = 0;
    viewer.unk4 = 0;
    viewer.unk0 = 0;
    viewer.unk38[20] = 0xffff;
    viewer.unk38[19] = 0xffff;
    viewer.unk38[18] = 0xffff;
    viewer.unk38[4] = 0xffff;
    viewer.unk38[3] = 0xffff;
    viewer.unk38[2] = 0xffff;
    viewer.unk38[1] = 0xffff;
    viewer.unk38[0] = 0xffff;
    viewer.unk38[6] = 0xffff;
    viewer.unk38[11] = 0xffff;
    viewer.unk38[10] = 0xffff;
    viewer.unk38[9] = 0xffff;
    viewer.unk38[8] = 0xffff;
    viewer.unk38[7] = 0xffff;
    viewer.unk38[12] = 0xffff;
    viewer.unk38[5] = 0xffff;
    viewer.unk38[17] = 0xffff;
    viewer.unk38[16] = 0xffff;
    viewer.unk38[15] = 0xffff;
    viewer.unk38[14] = 0xffff;
    viewer.unk38[13] = 0xffff;
}

/*
 * Totals the bonuses of the equipped items (equipment slots other than 4..6,
 * 11, 12 and 15) into the viewer's derived stats.
 */
ADDRESS(0x8003cb88, 0x354)
void player_equipment_bonuses(void)
{
    int kind[4];
    int amount[3];
    int i;
    int j;
    int a1;
    int a2;
    int a3;
    int a4;
    int a5;
    int a6;
    int a7;
    int a8;
    u16 *slot;
    Vis44 *item;

    slot = viewer.unk38;
    a1 = 0;
    a2 = 0;
    a3 = 0;
    a4 = 0;
    a5 = 0;
    a6 = 0;
    a7 = 0;
    a8 = 0;
    for (i = 0; i < 17; i++, slot++) {
        if (i >= 4 && i <= 6) {
            continue;
        }
        if (i == 11) {
            continue;
        }
        if (i == 12) {
            continue;
        }
        if (i == 15) {
            continue;
        }
        if (*slot == 0xffff) {
            continue;
        }
        item = &vis_table_c.unk0118[*slot];
        kind[0] = item->kind0;
        kind[1] = item->kind1;
        kind[2] = item->kind2;
        amount[0] = item->amount0;
        amount[1] = item->amount1;
        amount[2] = item->amount2;
        for (j = 0; j < 3; j++) {
            switch (kind[j]) {
            case 1:
                a1 += amount[j];
                if (a1 > 100) {
                    a1 = 100;
                }
                break;
            case 2:
                a2 += amount[j];
                if (a2 > 100) {
                    a2 = 100;
                }
                break;
            case 3:
                a3 += amount[j];
                if (a3 > 20) {
                    a3 = 20;
                }
                break;
            case 4:
                if (a4 >= 0) {
                    if (amount[j] == 1) {
                        a4 = 1;
                    } else if (amount[j] == 2) {
                        a4 = -1;
                    }
                }
                break;
            case 5:
                a5 += amount[j];
                if (a5 > 100) {
                    a5 = 100;
                }
                break;
            case 6:
                a6 += amount[j];
                if (a6 > 100) {
                    a6 = 100;
                }
                break;
            case 7:
                a7 += amount[j];
                if (a7 > 100) {
                    a7 = 100;
                }
                break;
            case 8:
                a8 = 1;
                break;
            }
        }
    }
    if (a1) {
        viewer.unk1C4 = 100 / a1;
    } else {
        viewer.unk1C4 = 0;
    }
    if (a2) {
        viewer.unk1C5 = 100 / a2;
    } else {
        viewer.unk1C5 = 0;
    }
    viewer.unk1C6 = a3 * 5;
    if (a4 < 0) {
        viewer.unk1C7 = -1;
    } else if (a4 > 0) {
        viewer.unk1C7 = 1;
    } else {
        viewer.unk1C7 = 0;
    }
    viewer.unk1C8 = 100 - a5 * 80 / 100;
    viewer.unk1C9 = a6;
    viewer.unk1CA = a7;
    viewer.unk1CB = a8;
}

ADDRESS(0x8003cedc, 0x4c)
void func_8003cedc(void)
{
    player_equipment_bonuses();
    func_8005a3a8(0);
    PLAYER_CLAMP_HP();
}

ADDRESS(0x8003cf28, 0x6c)
void player_add_hp(int delta)
{
    int hp;

    hp = viewer.unk2 + delta;
    if (hp <= 0) {
        hp = 0;
        player_kill(hp);
    } else if (viewer.unk0 < hp) {
        hp = viewer.unk0;
    }
    viewer.unk2 = hp;
}

ADDRESS(0x8003cf94, 0x48)
void player_add_mp(int delta)
{
    int mp;

    mp = viewer.unk6 + delta;
    if (mp <= 0) {
        mp = 0;
    } else if (viewer.unk4 < mp) {
        mp = viewer.unk4;
    }
    viewer.unk6 = mp;
}

ADDRESS(0x8003cfdc, 0x54)
void func_8003cfdc(int id)
{
    int v;

    v = vis_table_c.unk0118[id].unk28;
    if (v != 0) {
        if (v < 4) {
            viewer.unk2C |= 0x1000000;
        }
    }
}
