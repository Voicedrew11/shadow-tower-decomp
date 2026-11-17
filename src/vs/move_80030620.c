/*
 * VS.EXE: a fighter's walking moves (WIP module). -G8: the pad word the
 * moves test is a $gp-relative .sbss static.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <vs/data.h>
#include <vs/pad.h>
#include <vs/buffer.h>
#include <vs/funcs.h>

/* The buttons the moves test (0x10 forward, 0x40 back). */
DATA(0x800633a8, 0x4, ".sbss")
static s32 DAT_800633a8;

/* The buttons newly pressed this frame (func_80031478: 0x1000 and 0x8000
 * start attacks, 0x2000 and 0x4000 chain them). */
DATA(0x800633ac, 0x4, ".sbss")
static s32 DAT_800633ac;

/* Called without prototypes (retail passes int arguments unconverted):
 * src/vs/fighter_8002dec4.c's reach test, src/vs/fighter_8002e114.c's hit
 * and the attack effect func_8002f334 (not reconstructed). */
int func_8002dec4();
void func_8002e114();
void func_8002f334();

/* func_80031478's switch table. */
JUMPTABLES(0x800118a8, 0x1c)

/* Moves the fighter a step along its facing, forward or back. */
#define VS_WALK_ANGLE(f, d, op, angle)                                         \
    do {                                                                        \
        (f)->unk4DEF0.vx op rsin((angle) + 2048) * (d)->unk1AC * DAT_80063220 / 4096; \
        (f)->unk4DEF0.vz op rcos((f)->unk4DF02 + 2048) * (d)->unk1AC * DAT_80063220 / 4096; \
    } while (0)
#define VS_WALK(f, d, op) VS_WALK_ANGLE(f, d, op, (f)->unk4DF02)

/* Nonzero when from is 0 or the animation frame lies in [from, to]. */
#define VS_IN_FRAMES(f, from, to)     ((from) == 0 || ((f)->unk4DEEA >= (from) && (to) >= (f)->unk4DEEA))

/* Walks with the start/loop animation pair 0/1; the step only moves the
 * fighter inside frames [from, to] when from is nonzero. */
ADDRESS(0x80030620, 0x29c)
void func_80030620(VsFighter *f, s16 from, s16 to)
{
    VsFighterData *d = &f->unk4DF18;

    f->unk4DEEA &= 0xfff;
    if (DAT_800633a8 & 0x10) {
        f->unk4DF0E = 1;
        if (f->unk4DEE8 == d->unk0[0].unk0) {
            f->unk4DEE8 = d->unk0[1].unk0;
            f->unk4DEEA = 0;
        }
        f->unk4DEEC = d->unk0[1].unk2;
        if (from != 0 && (f->unk4DEEA < from || to < f->unk4DEEA))
            return;
        VS_WALK(f, d, +=);
    } else if (DAT_800633a8 & 0x40) {
        f->unk4DF0E = 1;
        if (f->unk4DEE8 == d->unk0[0].unk0) {
            f->unk4DEE8 = d->unk0[1].unk0;
            f->unk4DEEA = 4095;
        }
        f->unk4DEEC = -d->unk0[1].unk2;
        if (from != 0 && (f->unk4DEEA < from || to < f->unk4DEEA))
            return;
        VS_WALK(f, d, -=);
    } else {
        f->unk4DF0E = 0;
        if (f->unk4DEE8 == d->unk0[1].unk0) {
            f->unk4DEE8 = d->unk0[0].unk0;
            f->unk4DEEC = d->unk0[0].unk2;
            f->unk4DEEA = 0;
        }
    }
}

/* Walks forward with animation 1 and back with animation 12. */
ADDRESS(0x800308bc, 0x280)
void func_800308bc(VsFighter *f, s16 from, s16 to)
{
    VsFighterData *d = &f->unk4DF18;

    f->unk4DEEA &= 0xfff;
    if (DAT_800633a8 & 0x10) {
        f->unk4DF0E = 1;
        if (f->unk4DEE8 != d->unk0[1].unk0) {
            f->unk4DEE8 = d->unk0[1].unk0;
            f->unk4DEEC = d->unk0[1].unk2;
            f->unk4DEEA = 0;
        }
        if (from != 0 && (f->unk4DEEA < from || to < f->unk4DEEA))
            return;
        VS_WALK(f, d, +=);
    } else if (DAT_800633a8 & 0x40) {
        f->unk4DF0E = 1;
        if (f->unk4DEE8 != d->unk0[12].unk0) {
            f->unk4DEE8 = d->unk0[12].unk0;
            f->unk4DEEC = d->unk0[12].unk2;
            f->unk4DEEA = 0;
        }
        if (from != 0 && (f->unk4DEEA < from || to < f->unk4DEEA))
            return;
        VS_WALK(f, d, -=);
    } else {
        f->unk4DF0E = 0;
        if (f->unk4DEE8 != d->unk0[0].unk0) {
            f->unk4DEE8 = d->unk0[0].unk0;
            f->unk4DEEC = d->unk0[0].unk2;
            f->unk4DEEA = 0;
        }
    }
}

/* Slides along the facing without changing the animation. */
ADDRESS(0x80030b3c, 0x1ac)
void func_80030b3c(VsFighter *f)
{
    VsFighterData *d = &f->unk4DF18;

    f->unk4DEEA &= 0xfff;
    if (DAT_800633a8 & 0x10) {
        int angle = f->unk4DF02;

        f->unk4DF0E = 1;
        VS_WALK_ANGLE(f, d, +=, angle);
    } else if (DAT_800633a8 & 0x40) {
        int angle = f->unk4DF02;

        f->unk4DF0E = 1;
        VS_WALK_ANGLE(f, d, -=, angle);
    } else {
        f->unk4DF0E = 0;
    }
}

/* Walks with animation 1 entered through the start animation 10 and
 * backs off with animations 11/12 by movement kind. */
ADDRESS(0x80030ce8, 0x490)
void func_80030ce8(VsFighter *f, s16 from, s16 to)
{
    VsFighterData *d = &f->unk4DF18;
    s16 kind;

    if (DAT_800633a8 & 0x10) {
        f->unk4DF0E = 1;
        if (f->unk4DEE8 == d->unk0[1].unk0) {
            f->unk4DEEC = d->unk0[1].unk2;
            if (VS_IN_FRAMES(f, from, to))
                VS_WALK(f, d, +=);
        } else if (f->unk4DEE8 == d->unk0[10].unk0) {
            f->unk4DEEC = d->unk0[10].unk2;
            if (f->unk4DEEA >= 4096) {
                f->unk4DEE8 = d->unk0[1].unk0;
                f->unk4DEEC = d->unk0[1].unk2;
                f->unk4DEEA = 0;
            }
            if (d->unk1AA == 5 || d->unk1AA == 6 || d->unk1AA == 13)
                VS_WALK(f, d, +=);
        } else {
            f->unk4DEE8 = d->unk0[10].unk0;
            f->unk4DEEC = d->unk0[10].unk2;
            f->unk4DEEA = 0;
        }
    } else if (DAT_800633a8 & 0x40) {
        f->unk4DF0E = 1;
        kind = d->unk1AA;
        if (kind == 3 || kind == 5 || kind == 13) {
            if (f->unk4DEE8 == d->unk0[1].unk0) {
                f->unk4DEEC = -d->unk0[1].unk2;
                if (VS_IN_FRAMES(f, from, to))
                    VS_WALK(f, d, -=);
            } else if (f->unk4DEE8 == d->unk0[11].unk0) {
                f->unk4DEEC = -d->unk0[11].unk2;
                if (f->unk4DEEA <= 0) {
                    f->unk4DEE8 = d->unk0[1].unk0;
                    f->unk4DEEC = -d->unk0[1].unk2;
                    f->unk4DEEA = 4095;
                }
                if (d->unk1AA == 5 || d->unk1AA == 13)
                    VS_WALK(f, d, -=);
            } else {
                f->unk4DEE8 = d->unk0[11].unk0;
                f->unk4DEEC = -d->unk0[11].unk2;
                f->unk4DEEA = 4095;
            }
        } else {
            if (f->unk4DEE8 == d->unk0[1].unk0) {
                f->unk4DEEC = -d->unk0[1].unk2;
                if (VS_IN_FRAMES(f, from, to))
                    VS_WALK(f, d, -=);
            } else if (f->unk4DEE8 == d->unk0[10].unk0) {
                f->unk4DEEC = d->unk0[10].unk2;
                if (f->unk4DEEA >= 4096) {
                    f->unk4DEE8 = d->unk0[1].unk0;
                    f->unk4DEEC = -d->unk0[1].unk2;
                    f->unk4DEEA = 4095;
                }
                if (d->unk1AA == 6)
                    VS_WALK(f, d, -=);
            } else {
                f->unk4DEE8 = d->unk0[10].unk0;
                f->unk4DEEC = d->unk0[10].unk2;
                f->unk4DEEA = 0;
            }
        }
    } else {
        f->unk4DF0E = 0;
        if (f->unk4DEE8 != d->unk0[0].unk0) {
            f->unk4DEE8 = d->unk0[0].unk0;
            f->unk4DEEC = d->unk0[0].unk2;
            f->unk4DEEA = 0;
        }
    }
    f->unk4DEEA &= 0xfff;
}

/* Walks forward through the start animation 10 (moving only for movement
 * kinds 5 and 6 while it plays) and back with animation 12. */
ADDRESS(0x80031178, 0x300)
void func_80031178(VsFighter *f, s16 from, s16 to)
{
    VsFighterData *d = &f->unk4DF18;

    if (DAT_800633a8 & 0x10) {
        f->unk4DF0E = 1;
        if (f->unk4DEE8 == d->unk0[1].unk0) {
            if (VS_IN_FRAMES(f, from, to))
                VS_WALK(f, d, +=);
        } else if (f->unk4DEE8 == d->unk0[10].unk0) {
            if (f->unk4DEEA >= 4096) {
                f->unk4DEE8 = d->unk0[1].unk0;
                f->unk4DEEC = d->unk0[1].unk2;
                f->unk4DEEA = 0;
            }
            if (d->unk1AA == 5 || d->unk1AA == 6)
                VS_WALK(f, d, +=);
        } else {
            f->unk4DEE8 = d->unk0[10].unk0;
            f->unk4DEEC = d->unk0[10].unk2;
            f->unk4DEEA = 0;
        }
    } else if (DAT_800633a8 & 0x40) {
        f->unk4DF0E = 1;
        if (f->unk4DEE8 == d->unk0[12].unk0) {
            if (VS_IN_FRAMES(f, from, to))
                VS_WALK(f, d, -=);
        } else {
            f->unk4DEE8 = d->unk0[12].unk0;
            f->unk4DEEC = d->unk0[12].unk2;
            f->unk4DEEA = 0;
        }
    } else {
        f->unk4DF0E = 0;
        if (f->unk4DEE8 != d->unk0[0].unk0) {
            f->unk4DEE8 = d->unk0[0].unk0;
            f->unk4DEEC = d->unk0[0].unk2;
            f->unk4DEEA = 0;
        }
    }
    f->unk4DEEA &= 0xfff;
}

/*
 * Runs fighter n's controls for one frame: reads its pad, steps its
 * animation, refills its second gauge, turns it toward the opponent while
 * button 0x800 is held, then by state: 0, 1 and 5 walk (by movement kind),
 * turn and start attacks; 2 and 6 run an attack (combo follow-ups, hit
 * frames and lunges); 3 and 4 drain hit points while hit or down.
 */
ADDRESS(0x80031478, 0xc90)
void func_80031478(int n)
{
    VsFighter *f = &DAT_800633b0.fighter[n];
    VsFighterData *d = &f->unk4DF18;
    VsHitRecord *h;
    VsAttack *a;
    int i;
    int step;

    if (n == 0) {
        DAT_800633a8 = DAT_801603c0.pads[0].unk0;
        DAT_800633ac = DAT_801603c0.pads[0].unk8;
    } else {
        DAT_800633a8 = DAT_801603c0.pads[1].unk0;
        DAT_800633ac = DAT_801603c0.pads[1].unk8;
    }
    f->unk4DEEA += f->unk4DEEC * DAT_80063220;
    if (f->unk4DF0A < d->unk1BA) {
        f->unk4DF0A += DAT_80063220;
        if (d->unk1BA < f->unk4DF0A)
            f->unk4DF0A = d->unk1BA;
    }
    if (f->unk4DF0E != 5 && (DAT_800633a8 & 0x800)) {
        VsFighter *base = DAT_800633b0.fighter;
        VsFighter *o = base + (1 - n);
        s16 target;


        target = ratan2(f->unk4DEF0.vx - o->unk4DEF0.vx, f->unk4DEF0.vz - o->unk4DEF0.vz) & 0xfff;
        if (target < f->unk4DF02) {
            if (f->unk4DF02 - target < 2048) {
                f->unk4DF02 -= d->unk1AE * DAT_80063220 * 2;
                if (f->unk4DF02 < target)
                    f->unk4DF02 = target;
            } else {
                f->unk4DF02 = (f->unk4DF02 + d->unk1AE * DAT_80063220 * 2) & 0xfff;
            }
        } else {
            if (target - f->unk4DF02 < 2048) {
                f->unk4DF02 += d->unk1AE * DAT_80063220 * 2;
                if (target < f->unk4DF02)
                    f->unk4DF02 = target;
            } else {
                f->unk4DF02 = (f->unk4DF02 - d->unk1AE * DAT_80063220 * 2) & 0xfff;
            }
        }
        f->unk4DF02 &= 0xfff;
    }
    switch (f->unk4DF0E) {
    case 2:
        if (f->unk4DEEA < 4096) {
        if (f->unk4DEE8 == d->unk0[2].unk0) {
            h = &d->unk78[0];
            if (f->unk4DEEA / f->unk4DEEC < 20) {
                if (d->unk0[3].unk0 != 255 && (DAT_800633ac & 0x2000)) {
                    f->unk4DEE8 = d->unk0[3].unk0;
                    f->unk4DEEC = d->unk0[3].unk2;
                    f->unk4DEEA = 0;
                    f->unk4DF16 = 1;
                    break;
                }
                if (d->unk0[4].unk0 != 255 && (DAT_800633ac & 0x4000)) {
                    f->unk4DEE8 = d->unk0[4].unk0;
                    f->unk4DEEC = d->unk0[4].unk2;
                    f->unk4DEEA = 0;
                    f->unk4DF16 = 1;
                    break;
                }
            }
        } else if (f->unk4DEE8 == d->unk0[3].unk0) {
            h = &d->unk78[1];
        } else if (f->unk4DEE8 == d->unk0[4].unk0) {
            h = &d->unk78[2];
        }
        i = h->count;
        if (i == 3 && h->hits[0].frame == 0 && h->hits[1].frame == 0 && h->hits[2].frame == 0) {
            if (f->unk4DF16 != 0 && func_8002dec4(n, d->unk1B4, 2048)) {
                func_8002e114(n, &h->hits[0].hit);
                f->unk4DF16--;
            }
        } else {
            for (i--; i != -1; i--) {
                if (f->unk4DEEA - f->unk4DEEC * DAT_80063220 < h->hits[i].frame &&
                    f->unk4DEEA >= h->hits[i].frame &&
                    func_8002dec4(n, h->hits[i].range + d->unk1B4, h->hits[i].arc))
                    func_8002e114(n, &h->hits[i].hit);
            }
        }
        i = h->count2;
        while (i--) {
            if (f->unk4DEEA > h->moves[i].from && f->unk4DEEA < h->moves[i].to) {
                step = h->moves[i].speed * f->unk4DEEC * DAT_80063220 /
                       (h->moves[i].to - h->moves[i].from);
                f->unk4DEF0.vx += rsin(f->unk4DF02 + 2048) * step / 4096;
                f->unk4DEF0.vz += rcos(f->unk4DF02 + 2048) * step / 4096;
            }
        }
        } else {
            f->unk4DF0E = 0;
            f->unk4DEE8 = d->unk0[0].unk0;
            f->unk4DEEC = d->unk0[0].unk2;
            f->unk4DEEA = 0;
        }
        break;
    case 6:
        if (f->unk4DEEA < 4096) {
        if (f->unk4DEE8 == d->unk0[5].unk0) {
            a = &d->unk14A[0];
            if (f->unk4DEEA / f->unk4DEEC < 20) {
                if (d->unk0[6].unk0 != 255 && (DAT_800633ac & 0x2000)) {
                    f->unk4DEE8 = d->unk0[6].unk0;
                    f->unk4DEEC = d->unk0[6].unk2;
                    f->unk4DEEA = 0;
                    break;
                }
                if (d->unk0[7].unk0 != 255 && (DAT_800633ac & 0x4000)) {
                    f->unk4DEE8 = d->unk0[7].unk0;
                    f->unk4DEEC = d->unk0[7].unk2;
                    f->unk4DEEA = 0;
                    break;
                }
            }
        } else if (f->unk4DEE8 == d->unk0[6].unk0) {
            a = &d->unk14A[1];
        } else if (f->unk4DEE8 == d->unk0[7].unk0) {
            a = &d->unk14A[2];
        }
        if (f->unk4DEEA - f->unk4DEEC * DAT_80063220 < a->unkA && f->unk4DEEA >= a->unkA) {
            func_8002f334(n, a);
            f->unk4DF0A = d->unk1BA - a->unkC;
        }
        } else {
            f->unk4DF0E = 0;
            f->unk4DEE8 = d->unk0[0].unk0;
            f->unk4DEEC = d->unk0[0].unk2;
            f->unk4DEEA = 0;
        }
        break;
    case 3:
        if (f->unk4DEEA < 4096) {
            f->unk4DEE8 = d->unk0[8].unk0;
            f->unk4DEEC = d->unk0[8].unk2;
            f->unk4DF08 -= d->unk1B8 >> 4;
            if (f->unk4DF08 < f->unk4DF0C)
                f->unk4DF08 = f->unk4DF0C;
        } else {
            f->unk4DF08 = f->unk4DF0C;
            f->unk4DF0E = 0;
            f->unk4DEE8 = d->unk0[0].unk0;
            f->unk4DEEC = d->unk0[0].unk2;
            f->unk4DEEA = 0;
        }
        break;
    case 4:
        if (f->unk4DEEA < 4096) {
            f->unk4DEE8 = d->unk0[9].unk0;
            f->unk4DEEC = d->unk0[9].unk2;
            f->unk4DF08 -= d->unk1B8 >> 4;
            if (f->unk4DF08 < 0)
                f->unk4DF08 = 0;
        } else {
            f->unk4DF08 = 0;
            f->unk4DEEA = 4095;
        }
        break;
    case 0:
    case 1:
    case 5:
        if (DAT_800633a8 & 0x2000) {
            f->unk4DF0E = 5;
            f->unk4DEEA &= 0xfff;
            if (f->unk4DEE8 != d->unk0[0].unk0) {
                f->unk4DEE8 = d->unk0[0].unk0;
                /* retail sets the step from the animation id */
                f->unk4DEEC = d->unk0[0].unk0;
                f->unk4DEEA = 0;
            }
            break;
        }
        switch (d->unk1AA) {
        case 1:
            if (f->unk4DF10 == 1)
                func_800308bc(f, 0, 0);
            else if (f->unk4DF10 == 2)
                func_80030620(f, 0, 0);
            break;
        case 2:
            func_80030b3c(f);
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            if (f->unk4DF10 == 1)
                func_80031178(f, 0, 0);
            else if (f->unk4DF10 == 2)
                func_80030ce8(f, 0, 0);
            break;
        case 10:
            func_80030620(f, 1400, 3469);
            break;
        case 11:
            func_80030620(f, 1536, 2816);
            break;
        case 12:
            func_800308bc(f, 2048, 3072);
            break;
        case 13:
            func_80030ce8(f, 2048, 4096);
            break;
        case 99:
            f->unk4DEEA &= 0xfff;
            break;
        }
        if (!(DAT_800633a8 & 0x800)) {
            if (DAT_800633a8 & 0x80)
                f->unk4DF02 = (f->unk4DF02 - d->unk1AE * DAT_80063220) & 0xfff;
            if (DAT_800633a8 & 0x20)
                f->unk4DF02 = (f->unk4DF02 + d->unk1AE * DAT_80063220) & 0xfff;
        }
        if ((DAT_800633ac & 0x1000) && d->unk0[2].unk0 != 255) {
            f->unk4DF0E = 2;
            f->unk4DEE8 = d->unk0[2].unk0;
            f->unk4DEEC = d->unk0[2].unk2;
            f->unk4DEEA = 0;
            f->unk4DF16 = 1;
        }
        if ((DAT_800633ac & 0x8000) && d->unk0[5].unk0 != 255 && f->unk4DF0A == d->unk1BA) {
            f->unk4DF0E = 6;
            f->unk4DEE8 = d->unk0[5].unk0;
            f->unk4DEEC = d->unk0[5].unk2;
            f->unk4DEEA = 0;
        }
        break;
    }
}
