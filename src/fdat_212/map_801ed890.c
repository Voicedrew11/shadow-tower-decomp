/*
 * FDAT_212.BIN: an event handler and the two scene words after the code
 * (WIP module; a unit of its own until its neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/fdat.h>

/* Set by event 0x2e. */
DATA(0x801edcd4, 0x4, ".bss")
static int fdat_801edcd4;

/* Set by event 2. */
DATA(0x801edcd8, 0x4, ".bss")
static int fdat_801edcd8;

/* Records events 0x2e and 2 when they come with flag 0x4000. */
ADDRESS(0x801ed890, 0x4c)
void func_801ed890(int event)
{
    short id = event & ~0x4000;

    if (event & 0x4000) {
        if (id == 0x2e)
            fdat_801edcd4 = 1;
        else if (id == 2)
            fdat_801edcd8 = 1;
    }
}
