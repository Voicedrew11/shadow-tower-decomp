/*
 * FDAT_302.BIN: a handler after its other ones (WIP module; a unit of its
 * own until its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/anim.h>
#include <st/vis.h>
#include <st/data.h>
#include <st/hud.h>
#include <st/menu.h>
#include <st/fdat.h>

/* Sets the area id of every visible record. */
ADDRESS(0x801ee090, 0x28)
void func_801ee090(u8 area)
{
    VisRecord *r = vis_records;
    int i = 512;

    while (i--) {
        r->unk07 = area;
        r++;
    }
}
