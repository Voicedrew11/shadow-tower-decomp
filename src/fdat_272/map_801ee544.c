/*
 * FDAT_272.BIN: an item-use handler (WIP module; a unit of its own until its
 * neighbours in the run are exact).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/fdat.h>

DATA(0x801ef93c, 0x4, ".bss")
int fdat_801ef93c;

/* Item 0x12f used in front of table A entry 0x30 sets fdat_801ef93c; the
 * result is 1 then, else 2. */
ADDRESS(0x801ee544, 0xcc)
void func_801ee544(short id)
{
    VECTOR p;
    VisRequest req;

    p.vx = viewer.position.vx;
    p.vy = viewer.position.vy + 1600;
    p.vz = viewer.position.vz;
    req.unk0 = 0;
    req.unk4 = p;
    req.unk1C = 1056;
    req.unk1E = 1600;
    req.unk14.vx = 0;
    req.unk14.vy = 0;
    req.unk14.vz = 0;
    req.unk22 = 0;
    req.unk20 = 0;
    if (id == 0x12f) {
        if (func_8004bb88((VisLists *)&req, 1) == 0x30) {
            fdat_801ef93c = 1;
            DAT_801cd0e0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}
