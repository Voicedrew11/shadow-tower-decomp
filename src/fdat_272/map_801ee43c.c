/*
 * FDAT_272.BIN: the table B entry 16 trigger of the scene (WIP module; a unit of its own until its neighbours in
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

DATA(0x801ef92c, 0x4, ".bss")
int fdat_801ef92c;

void func_801ee610(void);
void func_801ee7d4(void);
void func_801ee948(void);

/* Standing at table B entry 16 with item slot 3 below 50 and the scene not
 * yet run: walks up to it and plays the scene (scene stage 2). */
ADDRESS(0x801ee43c, 0x108)
void func_801ee43c(void)
{
    Vis212 *e51 = &vis_table_a.entries[51];
    Vis88 *d = vis_table_b.unk71B0;
    VECTOR pos;
    SVECTOR rot;

    if (d == &vis_table_b.entries[16] && viewer.unk38[3] < 50 && d->unk0C == 1
        && fdat_801ef92c == 0) {
        pos.vx = d->unk24.vx;
        pos.vz = d->unk24.vz - 0x600;
        rot.vx = 0;
        rot.vy = 0;
        viewer.unk218 = 5;
        fdat_801ecfc4(&pos, &rot);
        func_801ee610();
        fdat_801ed1e4(e51);
        func_801ee7d4();
        func_801ee948();
        viewer.unk218 = 0;
        fdat_801ef924 = 2;
        fdat_801ef92c = 2;
        DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x300) | 0x200;
    }
}
