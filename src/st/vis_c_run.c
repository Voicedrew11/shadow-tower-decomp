/*
 * ST.EXE: the per-frame table C pass: measures each item of the current
 * area, lets dropped items fall and bounce, and runs a pick-up (WIP module;
 * the pick-up animation func_8005335c before it is not reconstructed).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/data.h>
#include <st/funcs.h>
#include <st/vis.h>

JUMPTABLES(0x80012738, 0x14)

/* The per-frame table C pass: drops fall and bounce, a pick-up runs.
 * Not exact: retail keeps the entry pointer in s1 and the record/height in
 * s2; this build swaps them (16 words). */
ADDRESS(0x80053aa4, 0x294)
void func_80053aa4(void)
{
    VisRequest req;
    Vis48 *c = vis_table_c.entries;
    Vis44 *rec;
    int i = 64;
    int h;

    while (i--) {
        if (c->unk00 != 0xffff && c->unk2c == area_state.unk4) {
            rec = &vis_table_c.unk0118[c->unk00];
            c->unk24 = 0;
            c->unk28 = SquareRoot0((c->unk08.vx - viewer.x) * (c->unk08.vx - viewer.x)
                                   + (c->unk08.vz - viewer.z) * (c->unk08.vz - viewer.z));
            switch (c->unk22) {
            case 0:
            case 2:
            case 4:
                break;
            case 1:
                if (c->unk25 < 4) {
                    req.unk4 = c->unk08;
                    req.unk1C = *(u16 *)&rec->unk5[9];
                    req.unk1E = 0x200;
                    req.unk0 = 2;
                    req.unk14.vz = 0;
                    req.unk14.vx = 0;
                    req.unk4.vy -= 0x200;
                    req.unk14.vy = c->unk26;
                    req.unk22 = 0;
                    req.unk20 = 0;
                    func_80048ad0((VisLists *)&req);
                    h = vis_lists.unk2C - 0x200;
                    if (h > 0) {
                        c->unk08.vy += c->unk26;
                        if (c->unk26 < 256)
                            c->unk26 += 8;
                    } else {
                        h = 0;
                        c->unk25++;
                        c->unk08.vy -= 0x200 - vis_lists.unk2C;
                        c->unk26 = -(c->unk26 >> 1);
                    }
                    if (viewer.y + 0x8000 < c->unk08.vy)
                        func_80052e50(c);
                    if (c->unk00 < 71)
                        c->unk18 = h;
                } else if (c->unk25 == 4) {
                    c->unk22 = 2;
                    c->unk18 = 0;
                }
                break;
            case 3:
                func_8005335c(c, 0);
                viewer.unk218 = 0;
                break;
            }
        }
        c++;
    }
}
