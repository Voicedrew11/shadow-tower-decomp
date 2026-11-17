/*
 * FDAT_102.BIN: its item-use handler (WIP module; a unit of its own after
 * the not-yet-exact func_801ed554).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/fdat.h>

DATA(0x801eda7c, 0x4, ".bss")
int fdat_801eda7c;

/* Item 0x12b used in front of table A entry 0x31 sets fdat_801eda7c; the
 * result is 1 then, else 2. */
ADDRESS(0x801ed6b8, 0xcc)
void func_801ed6b8(short id)
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
    if (id == 0x12b) {
        if (func_8004bb88((VisLists *)&req, 1) == 0x31) {
            fdat_801eda7c = 1;
            DAT_801cd0e0 = 1;
        } else {
            DAT_801cd0e0 = 2;
        }
    }
}
