/*
 * FDAT_032.BIN: the first of its own handlers (WIP module; a unit of its own
 * until its neighbours in the run are exact). Its RECT initializer is the
 * first entry of the overlay's read-only run after the handler tables, so
 * the unit claims that slice.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/gfx.h>
#include <st/fdat.h>

RODATA(0x801eb72c, 0x8)

/* Starts the area's texture animation on the 64x64 page at (768, 0). */
ADDRESS(0x801ed4a8, 0x64)
void func_801ed4a8(void)
{
    RECT rect = {768, 0, 64, 64};

    func_80016878();
    func_800168e0(&rect, 0, 4, 2, 3);
}
