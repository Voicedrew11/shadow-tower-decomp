/*
 * VS.EXE: a fighter's walking moves (WIP module). -G8: the pad word the
 * moves test is a $gp-relative .sbss static.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <vs/data.h>

/* The buttons the moves test (0x10 forward, 0x40 back). */
DATA(0x800633a8, 0x4, ".sbss")
static s32 DAT_800633a8;

/* Moves the fighter a step along its facing, forward or back. */
#define VS_WALK(f, d, op)                                                       \
    do {                                                                        \
        (f)->unk4DEF0.vx op rsin((f)->unk4DF02 + 2048) * (d)->unk1AC * DAT_80063220 / 4096; \
        (f)->unk4DEF0.vz op rcos((f)->unk4DF02 + 2048) * (d)->unk1AC * DAT_80063220 / 4096; \
    } while (0)

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
/* Not exact: retail loads the facing angle before the state store in
 * each branch (lh, li, sh); same size. */
ADDRESS(0x80030b3c, 0x1ac)
void func_80030b3c(VsFighter *f)
{
    VsFighterData *d = &f->unk4DF18;

    f->unk4DEEA &= 0xfff;
    if (DAT_800633a8 & 0x10) {
        f->unk4DF0E = 1;
        VS_WALK(f, d, +=);
    } else if (DAT_800633a8 & 0x40) {
        f->unk4DF0E = 1;
        VS_WALK(f, d, -=);
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
