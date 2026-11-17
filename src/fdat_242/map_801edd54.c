/*
 * FDAT_242.BIN: three handlers after fdat_801ed4a0 and the two scene words
 * they step (WIP module; a unit of its own until its neighbours in the run
 * are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/gfx.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/fdat.h>

/* Frames the texture animation has run (func_801edebc). */
DATA(0x801ee0b0, 0x4, ".bss")
static int fdat_801ee0b0;

/* The exit scene state: 3 once the animation ends, 4 played. */
DATA(0x801ee0b4, 0x4, ".bss")
static int fdat_801ee0b4;

void func_801edb10(void);

/*
 * The step of the overlay's entries, like fdat_801ed6e4: entry record 0
 * restarts script 0 at the end of a run until stream counter 12 starts.
 */
ADDRESS(0x801edd54, 0xe8)
void func_801edd54(void)
{
    Vis212 *e = vis_table_a.unk6138;

    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        if (e->unk02 == 0 && DAT_801cba08.unk1448[12] == 0) {
            fdat_801eb7b4(e, 0);
        } else {
            e->unk1A = 0xff;
            func_801edb10();
        }
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}

/* In area 0, unless the scene was played, hands table A entry 48 to its
 * exit once its record's stream counter has started. */
ADDRESS(0x801ede3c, 0x80)
void func_801ede3c(void)
{
    Vis212 *e;
    Vis192 *r;

    e = &vis_table_a.entries[48];
    r = &vis_table_a.unk3500[0];

    if (DAT_801cba08.unk1448[r->unk09] != 0 && fdat_801ee0b4 != 4
        && vis_table_a.unk6168 == 0) {
        DAT_801cba08.unk1448[r->unk09] = 1;
        e->unk70 |= 0x2000000;
        fdat_801ed388(e, 1, 1);
    }
}

/* Slows texture animations 0 and 1 every 4th frame for 32 frames, then
 * stops them and ends the scene. */
ADDRESS(0x801edebc, 0x70)
void func_801edebc(void)
{
    StTexAnim *t0 = &world_state.texanim[0];
    StTexAnim *t1 = &world_state.texanim[1];

    if (fdat_801ee0b0 < 32) {
        if ((fdat_801ee0b0 & 3) == 0) {
            t0->period++;
            t1->period++;
        }
        fdat_801ee0b0++;
    } else {
        t0->area = 0xfe;
        t1->area = 0xfe;
        fdat_801ee0b4 = 3;
    }
}
