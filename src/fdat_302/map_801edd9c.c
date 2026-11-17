/*
 * FDAT_302.BIN: a handler after its other ones (WIP module; a unit of its
 * own until its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/effect.h>
#include <st/fdat.h>

/* Table A entry 5's stage: 0, 1 hit, 2 fleeing, 3 beaten, 0xff gone;
 * defined here only because each map unit places one COMMON word
 * (src/fdat_302/map_801ed4a0.c and map_801ee0b8.c use it). */
DATA(0x801ee480, 0x4, ".bss")
int fdat_801ee480;

/* Moves the viewer, every live table A entry and every live effect 0x6000
 * along z (`up`) or back. */
ADDRESS(0x801edd9c, 0x104)
void func_801edd9c(int up)
{
    Vis212 *e = vis_table_a.entries;
    StEffect *fx = effects.slots;
    int dz = 0x6000;
    int i;

    if (up == 0)
        dz = -0x6000;
    viewer.z += dz;
    i = 64;
    while (i--) {
        if (e->unk00 != 0xff) {
            e->unk74.vz += dz;
            func_8004c1b4(e);
        }
        e++;
    }
    i = ST_EFFECT_SLOTS;
    while (i--) {
        if (fx->unk0 != ST_EFFECT_FREE)
            fx->unk20.vz += dz;
        fx++;
    }
}
