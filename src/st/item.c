/*
 * ST.EXE: item counts and the examine action (WIP module,
 * 0x8005529c..0x80055990): count tests, taking and adding one, and what
 * the player's examine/use reaches at a position: a table C item, a table
 * A entry to talk to, or the table B objects in reach (doors, images,
 * transitions).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/funcs.h>
#include <st/player.h>
#include <st/pad.h>
#include <st/xa.h>
#include <st/save.h>
#include <st/item.h>

ADDRESS(0x8005529c, 0x20)
int func_8005529c(int id)
{
    return DAT_801c7590.counts[id].count != 0;
}

/* Takes one of item `id`; nonzero when there was none. */
ADDRESS(0x800552bc, 0x44)
int func_800552bc(int id)
{
    if (DAT_801c7590.counts[id].count != 0) {
        DAT_801c7590.counts[id].count--;
        return 0;
    }
    return 1;
}

/* Adds one of item `id`; nonzero when 99 are already held. */
ADDRESS(0x80055300, 0x74)
int func_80055300(int id)
{
    if (DAT_801c7590.counts[id].count < 99) {
        DAT_801c7590.counts[id].count++;
        area_state.unkC->unk18(id);
        return 0;
    }
    return 1;
}

/*
 * Not exact: the switch's compare tree splits the cases above 51 at
 * 225 where retail splits at 226. The initial item-record address and
 * several branch delay slots also use different instruction ordering.
 */
/* The examine action at `pos` facing `rot` (vx: yaw, vy: pitch). */
ADDRESS(0x80055374, 0x61c)
void func_80055374(long *pos, SVECTOR *rot)
{
    VECTOR p;
    VisRequest req;
    int i;
    int first_result;
    int up;
    int down;
    int kind;
    u16 r;
    int n;
    int d;
    Vis48 *c;
    Vis212 *a;
    Vis88 *b;

    p.vx = pos[0];
    p.vy = pos[1] + 1600;
    p.vz = pos[2];
    DAT_801cd0e0 = 0;
    req.unk0 = 0;
    req.unk4 = p;
    req.unk1C = 1056;
    req.unk14.vx = 0;
    req.unk14.vy = 0;
    req.unk14.vz = 0;
    req.unk22 = 0;
    req.unk1E = 1600;
    req.unk20 = 0;
    i = func_80053174((VisLists *)&req, 1);
    first_result = i;
    if (i != -1) {
        c = &vis_table_c.entries[i];
        kind = c->unk22;
        c->unk22 = 3;
        c->unk23 = kind;
        return;
    }
    /* VisLists starts with the request fields func_8004bb88 reads. */
    i = func_8004bb88((VisLists *)&req, 1);
    if (i != first_result) {
        a = &vis_table_a.entries[i];
        up = ratan2(a->unk74.vy - a->unk30 - viewer.position.vy, a->unk40);
        down = ratan2(a->unk74.vy - (a->unk30 >> 1) - viewer.position.vy, a->unk40);
        if (up > 2048)
            up -= 2048;
        if (down > 2048)
            up -= 2048;
        if (up > 96)
            up = 96;
        if (down < 160)
            down = 160;
        if (func_8004a184((short)down, rot->vx) && func_8004a184(rot->vx, (short)up)) {
            if (func_8004a148(rot->vy, a->unk8E, 512))
                func_80054368(a, 0, 0);
            return;
        }
    }
    i = 0;
    req.unk1C = 256;
    vis_lists.unk4 = p;
    vis_lists.unk1C = 256;
    vis_lists.unk1E = 1600;
    vis_lists.unk14.vx = 0;
    vis_lists.unk14.vy = 0;
    vis_lists.unk14.vz = 0;
    vis_lists.unk22 = 0;
    vis_lists.unk20 = 0;
    vis_lists.unk0 = 0;
    for (; (i = func_8004f718(i, &vis_lists)) != -1; i++) {
        b = &vis_table_b.entries[i];
        DAT_801cd0e0 = 1;
        b->unk0E = 1;
        kind = vis_table_b.unk0000[b->unk0a].unk00[0];
        switch (kind) {
        case 0:
        case 1:
        case 50:
        case 51:
            if (b->unk4A.door.unk4F == 251) {
                r = b->unk22;
                if ((short)b->unk22 > 2048)
                    r -= 4096;
                /* The offset window wraps at the halfword boundary. */
                if ((u16)(r + 1023) < 2047) {
                    func_80057d58();
                    b->unk4A.door.unk4F = 255;
                }
            } else if (b->unk4A.door.unk4F == 252) {
                r = b->unk22;
                if ((short)b->unk22 > 2048)
                    r -= 4096;
                if ((u16)(r + 1024) >= 2049) {
                    func_80057d58();
                    b->unk4A.door.unk4F = 255;
                }
            }
            if (b->unk4A.door.unk4F == 255) {
                if (b->unk0C == 0)
                    b->unk0C = 1;
                if (b->unk0C == 7)
                    b->unk0C = 8;
            } else if (b->unk4A.door.unk4F < 0xf0 || b->unk4A.door.unk4F > 0xf3) {
                func_80057d7c();
            }
            break;
        case 52:
            if (b->unk4A.door.unk4F == 255) {
                if (b->unk0C == 0) {
                    b->unk0C = 1;
                    b->unk02 = 0;
                }
                if (b->unk0C == 255 && b->unk49 != 252) {
                    b->unk0C = 3;
                    b->unk02 = 1;
                }
            }
            break;
        case 225:
            d = (b->unk36 - b->unk20) & 0xfff;
            if (d < 512 || d > 3584)
                func_80054858(b->unk48);
            break;
        case 226:
            viewer.unk2 = viewer.unk0;
            func_800582f0(8, 128);
            break;
        case 254:
            if (b->unk0C == 0)
                b->unk0C = 1;
            break;
        case 14:
            if (area_state.unk0 != 1 && area_state.unk18 == -1) {
                lighting.unkA = 1;
                func_80015ad4();
                func_80055cf4(area_state.unk4);
                func_80057cc8();
                func_80042cfc();
                func_80057a3c();
                func_80057a68(0);
                b = vis_table_b.entries;
                n = 240;
                while (n--) {
                    if (b->unk05 == 236 && b->unk54 != -1) {
                        func_8005850c(b->unk54, b->unk4A.snd.unk4A + 1108);
                        b->unk54 = -1;
                    }
                    b++;
                }
                func_80058dc4(1, 1);
                func_80062c28(1);
                lighting.unkA = 2;
                func_80057a3c();
                func_80058ed4();
            }
            break;
        }
    }
    area_state.unkC->unk0(&p, rot);
}
