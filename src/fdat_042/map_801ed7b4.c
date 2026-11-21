/*
 * FDAT_042.BIN: its last two handlers and the scene word fdat_801ed970 (WIP module; a unit of its own until its neighbours in
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

DATA(0x801ed970, 0x4, ".bss")
int fdat_801ed970;

void func_801ed568(void);

/*
 * The current entry's handler: while the scene runs (fdat_801ed970 is 1),
 * table A entry 32 sinks 0x180 a frame down to y 0x40000 and then wakes;
 * then the animation steps as fdat_801ed6e4 does.
 */
ADDRESS(0x801ed7b4, 0x11c)
void func_801ed7b4(void)
{
    Vis212 *e = vis_table_a.unk6138;

    if (fdat_801ed970 == 1 && e == &vis_table_a.entries[32] && e->unkA4 != 0) {
        e->unk2A += e->unkB0;
        if (e->unk74.vy > 0x3ffff) {
            e->unk74.vy -= 0x180;
            return;
        }
        e->unk0E = 0;
        e->unk1C.bytes[3] = 0;
        fdat_801ed970 = 0xff;
    }
    e->unk2A += e->unkB0;
    if (fdat_801ece7c(e) == 1)
        return;
    if (e->unkB0 >= 0 ? (short)e->unk2A >= 4096 : (short)e->unk2A <= 0) {
        e->unk1A = 0xff;
        func_801ed568();
    }
    if (e->unkA8 != NULL) {
        fdat_801ebec4(e);
        e->unk1A = 0xf0;
    }
}

/* The scene: table A entry 32 wakes (animation 2), rises 0xc00 into the
 * current area, and flag 1 is set. */
ADDRESS(0x801ed8d0, 0x9c)
void func_801ed8d0(void)
{
    Vis212 *e = &vis_table_a.entries[32];

    fdat_801ed410(e);
    e->unk36 = -1;
    fdat_801ed970 = 1;
    e->unk1C.bytes[2] &= 0xf2;
    DAT_801cba08.unk1440.word |= 2;
    fdat_801eb7b4(e, 2);
    e->unk0E = 1;
    e->unk1C.bytes[3] = 1;
    fdat_801ed96c = 0;
    e->unk74.vy += 0xc00;
    e->unk07 = vis_table_a.unk6103;
}
