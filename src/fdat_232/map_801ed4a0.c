/*
 * FDAT_232.BIN: its first two handlers and the three scene words after its code (WIP module; a unit of its own until its neighbours in
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

/* Table A entry 32 beaten: 0 not yet, 1 this visit, 0xff (flag 28). */
DATA(0x801edb94, 0x4, ".bss")
static int fdat_801edb94;

/* 1 while the viewer's status 0x188 is held at 30. */
DATA(0x801edb98, 0x4, ".bss")
static int fdat_801edb98;

/* The entry scene: 0 waiting for the fade, 1 due, 0xff done. */
DATA(0x801edb9c, 0x4, ".bss")
static int fdat_801edb9c;

/* Sets up the area by flag 28 and record 12's stream counter. */
ADDRESS(0x801ed4a0, 0xfc)
void func_801ed4a0(void)
{
    Vis192 *r = &vis_table_a.unk3500[12];

    if (!(DAT_801cba08.unk1440.word >> 28 & 1))
        fdat_801edb94 = 0;
    else
        fdat_801edb94 = 0xff;
    if (DAT_801cba08.unk1448[r->unk09] >= 2)
        r->unk1C &= ~0x180;
    if (DAT_801cba08.unk1448[r->unk09] == 0) {
        viewer.unk218 = 5;
        fdat_801edb9c = 0;
    } else {
        fdat_801edb9c = 0xff;
    }
    if (DAT_801cba08.unk1448[r->unk09] != 0 && !(DAT_801cba08.unk1440.word >> 28 & 1))
        fdat_801edb98 = 1;
    else
        fdat_801edb98 = 0;
}

/*
 * Runs the area: once the fade is done table A entry 32 wakes; the
 * viewer's status 0x188 is held at 30 meanwhile; the viewer passing (x
 * 0x4c000, z 0x55000) while record 12's counter is 1 sends it; beaten, it
 * leaves (flag 28).
 */
ADDRESS(0x801ed59c, 0x23c)
void func_801ed59c(void)
{
    Vis212 *e = &vis_table_a.entries[32];
    Vis192 *r = &vis_table_a.unk3500[12];
    int n;

    if (fdat_801edb9c == 0 && area_state.unk18 == -1 && area_state.unk2 != 0)
        fdat_801edb9c = 1;
    if (fdat_801edb9c == 1) {
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 0);
        fdat_801edb98 = 1;
        fdat_801edb9c = 0xff;
        viewer.unk218 = 0;
    }
    if (fdat_801edb98 == 1 && !(viewer.unk2C & 0x800)) {
        viewer.unk188 = 30;
        viewer.unk182 = 0;
    }
    n = DAT_801cba08.unk1448[r->unk09];
    if (n == 1 && viewer.position.vx > 0x4c000 && viewer.position.vz > 0x55000) {
        viewer.unk218 = 5;
        fdat_801ed388(e, 0, 0);
        e->unk64 = n;
        viewer.unk218 = 0;
        e->unk70 &= ~0x2000000;
        r->unk1C &= ~0x180;
    }
    if (fdat_801edb94 == 0 && e->unk2C == 0) {
        fdat_801edb94 = 1;
        DAT_801cba08.unk1440.word |= 0x10000000;
    }
    if (fdat_801edb94 == 1 && e->unk05 == 3) {
        viewer.unk218 = 5;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 0);
        fdat_801edb94 = 0xff;
        fdat_801edb98 = 0;
        viewer.unk218 = 0;
    }
}
