/*
 * FDAT_172.BIN: its per-entry handler, the item-use handler and the scene word fdat_801edfd0 (WIP module; a unit of its own until its neighbours in
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

/* 0 until item 0x12a is used (1), 0xff once its scene ran (flag 26). */
DATA(0x801edfd0, 0x4, ".bss")
int fdat_801edfd0;

void func_801ed93c(void);

/* The current entry's handler: entries below 56 other than kind 1 are kept
 * in range (unk03 3) until the item scene ran; then the animation steps as
 * fdat_801ed6e4 does, kind 1 replaying animation 9 while entry 48 sleeps. */
ADDRESS(0x801edbf8, 0x140)
void func_801edbf8(void)
{
    Vis212 *e = vis_table_a.unk6138;

    if (fdat_801edfd0 != 0xff && e->unk02 != 1 && e < &vis_table_a.entries[56] && e->unk05 != 5)
        e->unk03 = 3;
    else
        e->unk03 = 0;
    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 1 && (fdat_801edfd4 == 0 || fdat_801edfd4 == 0xfd)) {
            fdat_801eb7b4(e, 9);
        } else {
            e->unk1A = 0xff;
            func_801ed93c();
        }
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}

/* Item 0x12a used in front of table B entry 12 sets fdat_801edfd0; the
 * result is 1 then, else 2. */
ADDRESS(0x801edd38, 0xc8)
void func_801edd38(short id)
{
    VECTOR p;
    VisRequest req;

    if (id == 0x12a) {
        p.vx = viewer.position.vx;
        p.vy = viewer.position.vy + 1600;
        p.vz = viewer.position.vz;
        req.unk0 = 0;
        req.unk4 = p;
        req.unk1C = 1056;
        req.unk14.vx = 0;
        req.unk14.vy = 0;
        req.unk14.vz = 0;
        req.unk22 = 0;
        req.unk1E = 1600;
        req.unk20 = 0;
        if (func_8004f53c((VisLists *)&req) == 12) {
            DAT_801cd0e0 = 1;
            fdat_801edfd0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}
