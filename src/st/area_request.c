/*
 * ST.EXE: requesting a new area set (five area ids, 255 keeps one) with
 * its screen fade (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <st/cd.h>
#include <st/data.h>
#include <st/save.h>
#include <st/area.h>

/* Starts loading areas a0..a4 unless they are already loaded or being
 * loaded; f0..f2 is the fade colour and f3..f5 the next fade's (WIP). */
/* All nine direct callers discard the word result; the return edges keep
 * v0 live. */
ADDRESS(0x80045044, 0x524)
KF_VALUELESS_S32 func_80045044(u8 a0, u8 a1, u8 a2, u8 a3, u8 a4, s8 f0, s8 f1, s8 f2, u8 f3, u8 f4,
                   u8 f5)
{
    u8 n0, n1, n2, n3, n4;
    u8 m0, m1, m2, m3, m4;

    if (a0 == 0xff) {
        n0 = area_state.unk4;
        m0 = area_state.unk10;
        if (a1 == 0xff) {
            n1 = area_state.unk5;
            m1 = area_state.unk11;
        } else {
            n1 = m1 = a1;
        }
        if (a2 == 0xff) {
            n2 = area_state.unk6;
            m2 = area_state.unk12;
        } else {
            n2 = m2 = a2;
        }
        if (a3 == 0xff) {
            n3 = area_state.unk7;
            m3 = area_state.unk13;
        } else {
            n3 = m3 = a3;
        }
        if (a4 == 0xff) {
            n4 = area_state.unk8;
            m4 = area_state.unk14;
        } else {
            n4 = m4 = a4;
        }
    } else {
        m1 = a1;
        m4 = a4;
        m0 = a0;
        n0 = a0;
        n1 = area_state.unk5;
        n2 = area_state.unk6;
        n3 = area_state.unk7;
        n4 = area_state.unk8;
        m2 = a2;
        m3 = a3;
    }
    if (area_state.unk0 != 0)
        goto busy;
    if (area_state.unk4 == n0 && area_state.unk5 == n1 && area_state.unk6 == n2 &&
        area_state.unk7 == n3 && area_state.unk8 == n4)
        return;
start:
    if (area_state.unk4 != 99 && area_state.unk4 != n0)
        func_80055cf4(area_state.unk4);
    area_state.unk0 = 1;
    area_state.unk2 = 0;
    area_state.unk10 = a0;
    area_state.unk11 = a1;
    area_state.unk12 = a2;
    area_state.unk13 = a3;
    area_state.unk14 = a4;
    area_state.unk20 = f0;
    area_state.unk21 = f1;
    area_state.unk22 = f2;
    area_state.unk23 = f3;
    area_state.unk24 = f4;
    area_state.unk25 = f5;
    area_state.unk1C = 0;
    if (a0 < 44)
        area_state.unk18 = cd_state.unk4;
    else
        area_state.unk18 = -1;
    if (a1 == 0xff) {
        area_state.unk1D = 0;
    } else {
        area_state.unk1D = 1;
        area_state.unk1F = 1;
    }
    return;
busy:
    if ((area_state.unk0 != 1 || area_state.unk10 == m0) && area_state.unk11 == m1 &&
        area_state.unk12 == m2 && area_state.unk13 == m3 && area_state.unk14 == m4)
        return;
    if (area_state.unk10 == 0xff && area_state.unk4 == n0)
        return;
    if (area_state.unk11 == 0xff && area_state.unk5 == a1)
        return;
    if (area_state.unk12 == 0xff && area_state.unk6 == a2)
        return;
    if (area_state.unk13 == 0xff && area_state.unk7 == a3)
        return;
    if (area_state.unk14 == 0xff && area_state.unk8 == a4)
        return;
    if ((area_state.unk10 != 0xff && a0 == 0xff) || (area_state.unk11 != 0xff && a1 == 0xff) ||
        (area_state.unk12 != 0xff && a2 == 0xff) || (area_state.unk13 != 0xff && a3 == 0xff) ||
        (area_state.unk14 != 0xff && a4 == 0xff)) {
        func_80015ad4();
        while (area_state.unk0) {
            func_8001531c();
            func_800456f8();
        }
    } else {
        while ((s16)area_state.unk2 == 240)
            func_8001531c();
    }
    ExitCriticalSection();
    if (area_state.unk1C && a0 != 0xff && f0 == 127 && area_state.unk10 != a0) {
        f0 = -area_state.unk20;
        f2 = -area_state.unk22;
        f1 = -area_state.unk21;
    }

    goto start;
}
