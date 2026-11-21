/*
 * ST.EXE: a table C entry's pick-up animation: it moves in front of the
 * viewer to be looked at, then into the viewer or back to its place
 * (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <INLINE_C.H>
#include <st/data.h>
#include <st/funcs.h>
#include <st/vis.h>

/* Draws the HUD (src/st/hud.c). Retail's caller saw it without a
 * declaration, as a function returning a word: nothing reads the result,
 * but v0 counts as used, which keeps it out of reach of reload. */
int func_80026afc();

/* Shows entry c in front of the viewer until a button takes it (bit 10)
 * or puts it back (bit 11, unless forced). */
ADDRESS(0x8005335c, 0x748)
void func_8005335c(Vis48 *c, int forced)
{
    Vis48 save;
    VECTOR a;
    VECTOR b;
    SVECTOR r;
    SVECTOR r2;
    Vis44 *info;
    int take;
    int i;
    int j;

    info = &vis_table_c.unk0118[c->unk00];
    save = *c;
    viewer.unk218 = 3;
    gte_SetRotMatrix(&viewer.unk298);
    gte_ldv0((SVECTOR *)&info->unk5[3]);
    gte_rtv0();
    gte_stlvnl(&a);
    {
        VECTOR *position = &viewer.position;

        a.vx += position->vx;
        a.vy += position->vy;
        a.vz += position->vz;
    }
    switch (info->unk5[2]) {
    case 0:
        r.vx = -viewer.unk268;
        r.vy = r.vz = 0;
        break;
    case 1:
        r.vx = 1024 - viewer.unk268;
        r.vy = r.vz = 0;
        break;
    case 2:
        r.vx = -1024 - viewer.unk268;
        r.vy = r.vz = 0;
        break;
    case 3:
        r.vx = 0;
        r.vy = 1024;
        r.vz = viewer.unk268 - 256;
        break;
    }
    r.vy += viewer.unk26A;
    if (r.vx - c->unk18 > 2048)
        r.vx -= 4096;
    if (r.vx - c->unk18 < -2048)
        r.vx += 4096;
    if (r.vy - c->unk1A > 2048)
        r.vy -= 4096;
    if (r.vy - c->unk1A < -2048)
        r.vy += 4096;
    i = 16;
    c->unk2D -= 8;
    while (1) {
        if (forced == 0 && (DAT_801cd180.unk4 & DAT_801cd180.unk22[11])) {
            take = 0;
            a = c->unk08;
            b = save.unk08;
            r = *(SVECTOR *)&c->unk18;
            r2 = *(SVECTOR *)&save.unk18;
            i = 16 - i;
            break;
        }
        if ((DAT_801cd180.unk4 & DAT_801cd180.unk22[10]) && !DAT_801cd180.unk12) {
            take = 1;
            a = c->unk08;
            b = viewer.position;
            b.vy += 800;
            r = *(SVECTOR *)&c->unk18;
            r2 = r;
            i = 16;
            break;
        }
        j = 16 - i;
        c->unk08.vx = (i * save.unk08.vx + j * a.vx) >> 4;
        c->unk08.vy = (i * save.unk08.vy + j * a.vy) >> 4;
        c->unk08.vz = (i * save.unk08.vz + j * a.vz) >> 4;
        if (i >= 9) {
            c->unk18 = save.unk18;
            c->unk1A = ((i - 8) * save.unk1A + j * r.vy) >> 3;
            c->unk1C = save.unk1C;
        } else {
            c->unk18 = (i * save.unk18 + (8 - i) * r.vx) >> 3;
            c->unk1A = r.vy;
            c->unk1C = (i * save.unk1C + (8 - i) * r.vz) >> 3;
        }
        func_80026afc(0, 0);
        player_update_status(0);
        if (i > 0)
            i--;
    }
    DAT_801cd180.unk4 = 0;
    while (i--) {
        j = 16 - i;
        c->unk08.vx = (i * a.vx + j * b.vx) >> 4;
        c->unk08.vy = (i * a.vy + j * b.vy) >> 4;
        c->unk08.vz = (i * a.vz + j * b.vz) >> 4;
        if (i >= 9) {
            c->unk18 = ((i - 8) * r.vx + j * r2.vx) >> 3;
            c->unk1A = r.vy;
            c->unk1C = ((i - 8) * r.vz + j * r2.vz) >> 3;
        } else {
            c->unk18 = r2.vx;
            c->unk1A = (i * r.vy + (8 - i) * r2.vy) >> 3;
            c->unk1C = r2.vz;
        }
        func_80026afc(0, 0);
        player_update_status(0);
    }
    c->unk2D += 8;
    if (!take) {
        *c = save;
        c->unk22 = c->unk23;
    } else {
        func_80052d98(c);
        func_80052e50(c);
    }
}
