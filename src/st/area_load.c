/*
 * ST.EXE: the area load state machine, one step per call: the area's
 * script and tables, its character file table, the object tables and
 * sound banks (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBSND.H>
#include <st/cd.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/memory.h>
#include <st/anim.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/tim.h>
#include <st/save.h>
#include <st/funcs.h>
#include <st/area.h>

RODATA(0x800122a8, 0xc)

ADDRESS(0x800456f8, 0x898)
void func_800456f8(void)
{
    Vis212 *a;
    Vis88 *b;
    Vis48 *c;
    StVabSlot *slot;
    u_long *p;
    int i;

    if (area_state.unk0 == 0)
        return;
    if (area_state.unk0 != 1)
        return;
    switch ((s16)area_state.unk2) {
    case 0:
        if (area_state.unk10 != 0xff) {
            if (func_80015ec8(1, area_state.unk10 * 10))
                func_80045f90(1, area_state.unk10 * 10, func_80045648);
            area_state.unk2 = 240;
            b = vis_table_b.entries;
            for (i = 239; i != -1; i--) {
                if (b->unk05 == 236 && b->unk54 != -1) {
                    func_8005850c(b->unk54, b->unk4A.snd.unk4A + 1108);
                    b->unk54 = -1;
                }
                b->unk05 = 0xff;
                b++;
            }
            func_80015d54(1, (area_state.unk10 * 10) | 1, (u8 *)DAT_80088c88, func_8004557c);
        }
        break;
    case 1:
        area_state.unk2 = 240;
        area_state.unkC = &DAT_8008038c;
        func_80015d54(1, area_state.unk10 * 10 + 2, DAT_800881dc, func_80045594);
        viewer.unk174 = NULL;
        viewer.unk178 = -1;
        func_80014cfc((u32 *)vis_records, (u32 *)DAT_80088c88, 0x3e00);
        if ((s8)area_state.unk20 != 127) {
            area_state.unk1C = 1;
            func_80045010((s8)area_state.unk20 * 4096 + (s8)area_state.unk23 * 128,
                          (s8)area_state.unk22 * 4096 + (s8)area_state.unk24 * 128,
                          (s8)area_state.unk21 * 4096 + (s8)area_state.unk25 * 128);
        }
        area_state.area = area_state.unk10;
        break;
    case 2:
        area_state.unk2 = 240;
        func_80015d54(1, area_state.unk10 * 10 + 3, (u8 *)DAT_80088c88, func_800455ac);
        break;
    case 3:
        if (area_state.unk11 != 0xff) {
            char name[] = "CHRX\\MXX.T";

            func_800152f8(cd_state.archives[2].sectors);
            cd_state.archives[2].sectors = NULL;
            name[3] = area_state.unk10 / 10 + '0';
            name[6] = area_state.unk10 / 10 + '0';
            name[7] = area_state.unk10 % 10 + '0';
            func_80016294(2, name);
            area_state.unk2 = 240;
            func_80015d54(1, area_state.unk11 * 10 + 4, (u8 *)DAT_800fc488, func_800455c4);
        }
        if (area_state.unk10 != 0xff) {
            a = vis_table_a.entries;
            for (i = 63; i != -1; i--) {
                if (a->unkA4) {
                    anim_slot_free((StAnimSlot *)a->unkA4);
                    if (a->unkA4)
                        anim_set_free((StAnimSet *)a->unkA4);
                }
                a->unk00 = 0xff;
                a++;
            }
            b = vis_table_b.entries;
            for (i = 239; i != -1; i--) {
                if (*(StAnimSlot **)b->unk44) {
                    anim_slot_free(*(StAnimSlot **)b->unk44);
                    if (*(StAnimSlot **)b->unk44)
                        anim_set_free(*(StAnimSet **)b->unk44);
                }
                b->unk0a = 0xffff;
                b++;
            }
            c = vis_table_c.entries;
            for (i = 63; i != -1; i--) {
                if (*(StAnimSlot **)c->unk04) {
                    anim_slot_free(*(StAnimSlot **)c->unk04);
                    if (*(StAnimSlot **)c->unk04)
                        anim_set_free(*(StAnimSet **)c->unk04);
                }
                c++;
            }
            func_80052d10();
            for (i = 15; i != -1; i--) {
                if (world_state.unk834C[128 + i])
                    func_80015230(world_state.unk834C[128 + i]);
            }
            for (i = 127; i != -1; i--) {
                if (world_state.unk8C7C[i])
                    func_80015230(world_state.unk8C7C[i]);
            }
            func_80014cfc((u32 *)vis_table_a.unk3500, (u32 *)&DAT_80088c88[1], 2816);
            func_8004e8ec();
            p = &DAT_80088c88[1] + DAT_80088c88[0] / 4;
            func_8004e960((VisAFileEntry *)(p + 1));
            p = (u_long *)((u8 *)p + *p + 4);
            func_80014cfc((u32 *)&vis_table_b.unk0000[300], (u32 *)(p + 1), 192);
            p = (u_long *)((u8 *)p + *p + 4);
            func_8004eebc((u8 *)(p + 1));
            p = (u_long *)((u8 *)p + *p + 4);
            func_80052f30((u8 *)(p + 1));
            p = (u_long *)((u8 *)p + *p + 4);
            placements_expand((StPlacement *)(p + 1));
            func_80055f9c(area_state.unk10);
            DAT_801cba08.unk1478[area_state.unk10] |= 1;
            area_state.unkC = (AreaHooks *)DAT_800881d8;
            func_80016878();
            area_state.unkC->unk14();
            vis_table_a.unk6168 = 254;
            vis_table_a.unk6158[3] = 0xff;
            vis_table_a.unk6158[2] = 0xff;
            vis_table_a.unk6158[1] = 0xff;
            vis_table_a.unk6158[0] = 0xff;
            slot = &DAT_801dd1c0.slots[8];
            for (i = 15; i != -1; i--) {
                if (slot->block) {
                    while (slot->block[2] == 3)
                        func_8001531c();
                    SsVabClose(slot->vab);
                    slot->vab = -1;
                    slot->block = NULL;
                }
                slot++;
            }
            viewer.unk21C = 1;
            area_state.unk1E = 1;
        }
        if (area_state.unk11 != 0xff)
            break;
        /* fall through */
    case 5:
        if (area_state.unk12 != 0xff) {
            area_state.unk2 = 240;
            func_80045f90(1, area_state.unk12 * 10 + 6, func_800455f4);
        }
        func_80015ad4();
        area_state.unk1D = 0;
        area_state.unk1F = 0;
        if (area_state.unk13 == 0xff && area_state.unk14 == 0xff) {
            area_state.unk2 = 7;
            goto done;
        }
        area_state.unk2 = 6;
        /* fall through */
    case 6:
        if (DAT_801dd1c0.slots[1].vab != -1) {
            SsVabClose(DAT_801dd1c0.slots[1].vab);
            DAT_801dd1c0.slots[1].vab = -1;
            DAT_801dd1c0.slots[1].block = NULL;
        }
        if (area_state.unk13 != 0xff) {
            area_state.unk2 = 240;
            func_80058a0c(1, area_state.unk13 * 10 + 7, 1);
        }
        if (area_state.unk14 == 0xff)
            goto done;
        area_state.unk2 = 7;
        break;
    case 4:
        if (area_state.unk11 != 0xff) {
            area_state.unk2 = 240;
            func_80015d54(1, area_state.unk11 * 10 + 5, (u8 *)DAT_800f2c88, func_800455dc);
        }
        break;
    case 7:
    case 8:
    done:
        func_8004567c();
        break;
    case 240:
        /* a CD read is pending */
        break;
    }
}
