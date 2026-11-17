/*
 * FDAT_272.BIN: the per-entry handler of table A entries 48..51 (gate and scene stages) (WIP module; a unit of its own until its neighbours in
 * the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/anim.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/light.h>
#include <st/xa.h>
#include <st/fdat.h>

DATA(0x801ef924, 0x4, ".bss")
int fdat_801ef924;

RODATA(0x801eb7b8, 0x1c)

void func_801edad8(void);
void func_801eead8(void);
void func_801eebb4(void);
void func_801eec74(void);
void func_801eeeb4(void);
void func_801eea14(void);
void func_801ef07c(void);

/* Not exact (one attempt, retries off; 90.9%): first divergence retail `addiu v0,v0,0`, this source `addiu a2,v0,10176`. */
/*
 * The current entry's handler: kind 4 runs the item 0x12f scene once,
 * kind 0 steps the gate stages (story flag bits 10-12), kind 10 the scene
 * stages in fdat_801ef924, kind 11 holds its pose; then the animation
 * steps as fdat_801ed6e4 does.
 */
ADDRESS(0x801edd24, 0x718)
void func_801edd24(void)
{
    Vis212 *e48 = &vis_table_a.entries[48];
    Vis212 *e50 = &vis_table_a.entries[50];
    Vis212 *e = vis_table_a.unk6138;
    Vis192 *r = vis_table_a.unk6134;
    VECTOR pos;
    SVECTOR rot;

    if (e->unk02 == 4 && fdat_801ef93c == 1) {
        rot.vx = 0;
        rot.vy = -0x400;
        viewer.unk218 = 5;
        pos.vx = e48->unk74.vx - 0xa00;
        pos.vz = e48->unk74.vz;
        fdat_801ecfc4(&pos, &rot);
        func_801eead8();
        rot.vx = 0xc0;
        rot.vy = -0x500;
        fdat_801ecfc4(&pos, &rot);
        func_801eebb4();
        func_801eec74();
        func_80054368(e, 0, 0);
        func_801eeeb4();
        fdat_801ecf44(e);
        fdat_801ef93c = 2;
        viewer.unk218 = 0;
        DAT_801cba08.unk1440.word |= 0x4000;
    }
    if (e->unk02 == 0) {
        switch (DAT_801cba08.unk1440.word >> 10 & 7) {
        case 2:
            viewer.unk218 = 5;
            fdat_801ed388(e, 0, 0);
            fdat_801ed1e4(e);
            func_801ef07c();
            viewer.unk218 = 0;
            e->unk00 = 100;
            e->unk8E = 0x400;
            e->unk28 = 0x31;
            e->unk74.vx = 0x40800;
            e->unk74.vy = 0x41000;
            e->unk74.vz = 0x28800;
            e->unk6A = 0x800;
            e->unk6C = 0;
            e->unk6E = -0x800;
            e->unk06 = vis_records[49].unk01;
            DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0xc00;
            break;
        case 3:
            if (viewer.unk178 == 0x2f) {
                fdat_801ed388(e, 0, 0);
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x1000;
            } else if (viewer.unk178 == 0x16) {
                DAT_801cba08.unk1448[r->unk09]++;
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x1400;
            }
            break;
        case 4:
            if (viewer.unk178 == 0x16)
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x1400;
            break;
        case 5:
            if (DAT_801cba08.unk1440.word >> 29 & 1) {
                DAT_801cba08.unk1448[r->unk09]++;
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x1800;
            } else {
                e->unk70 |= 0x2000000;
                fdat_801ed388(e, 0, 0);
                e->unk1C.bytes[3] = 2;
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x1800;
                e->unk70 &= ~0x2000000;
            }
            break;
        case 6:
            if (e->unk64 != 0) {
                e->unk70 |= 0x2000000;
                func_80054368(e, 0, 0);
                e->unk36 = -1;
                e->unk34 = -1;
                e->unk1C.bytes[3] = 0;
                e->unk1C.bytes[2] &= 0xe2;
                if (e->unk2C == 0)
                    e->unk2C = 1;
                DAT_801cba08.unk1440.word |= 0x1c00;
                e->unk70 &= ~0x2000000;
            }
            break;
        case 1:
        case 7:
            break;
        }
    }
    if (e->unk02 == 10) {
        if (e->unk64 != 0) {
            func_801eea14();
            fdat_801ef924 = 3;
        }
        switch (fdat_801ef924) {
        case 0:
            if (viewer.unk178 == 0xa4) {
                e50->unk36 = -1;
                e50->unk1C.bytes[2] &= 0xf2;
                fdat_801ed388(e, 0, 0);
                fdat_801ef924 = 1;
                e->unk70 &= ~0x2000000;
                DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x300) | 0x100;
            }
            break;
        case 1:
            if (fdat_801ef920 == 0) {
                func_801eea14();
                fdat_801ef924 = 3;
            } else {
                fdat_801ef920--;
            }
            break;
        case 3:
            if ((short)e->unk2A + e->unkB0 >= 0x1000) {
                func_8004ae38(e);
                fdat_801ef924 = 4;
            }
            break;
        }
    }
    if (e->unk02 == 11) {
        if (fdat_801ef924 == 3 && (short)e->unk2A + e->unkB0 >= 0x1000) {
            e->unk2A = 0xfff;
            e->unkB0 = 0;
        }
        if (fdat_801ef924 == 4) {
            if (fdat_801ef92c == 2)
                fdat_801eb7b4(e, 0);
            else
                fdat_801eb7b4(e, 1);
            e->unk2A = 0xfff;
            e->unkB0 = 0;
        }
    }
    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 4 && fdat_801ef93c == 0) {
            fdat_801eb7b4(e, 0);
            e->unkB0 = 0;
        }
        if (e->unk02 == 0) {
            if ((DAT_801cba08.unk1440.word >> 10 & 7) == 1) {
                fdat_801eb7b4(e, 0);
            } else {
                e->unk1A = 0xff;
                func_801edad8();
            }
        } else if (e->unk02 == 10) {
            if (fdat_801ef924 < 2 && fdat_801ef924 >= 0) {
                fdat_801eb7b4(e, 0);
            } else {
                fdat_801eb7b4(e, e->unk18);
                return;
            }
        } else if (e->unk02 == 11) {
            if (fdat_801ef92c == 2)
                fdat_801eb7b4(e, 0);
            else
                fdat_801eb7b4(e, 1);
            e->unkB0 = 0;
        } else {
            e->unk1A = 0xff;
            func_801edad8();
        }
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}
