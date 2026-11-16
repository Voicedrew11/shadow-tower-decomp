/*
 * ST.EXE: the option screen and its button configuration page (WIP
 * module; one translation unit with the screens around it, see
 * src/st/menu_vram.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/cd.h>
#include <st/funcs.h>
#include <st/card.h>
#include <st/pad.h>
#include <st/sound.h>

/*
 * Page 1 (unk2DC 1) edits the option bytes DAT_801cd120.unk1 (a 0..63
 * slider) and unk2..unk5 (on/off), row 5 opens page 2. Page 2 lists the
 * twelve button masks (DAT_801cd180.unk22) as button numbers
 * (MenuState.levels); a row waits for the next button (unk2E2), row 12
 * restores the defaults, row 13 returns, and buttons used twice mark
 * levels.flags.
 * Not exact (62.0%, one word long): this build gives the scratchpad base
 * s1 instead of s0 and the button masks a base-plus-34 address instead of
 * retail's pointer at +34, and hoists other constants (retail: s3 12,
 * s4 1, s5 13, s6 5).
 */
ADDRESS(0x80060334, 0x878)
void func_80060334(void)
{
    MenuScratch *sp = MENU_SCRATCH;
    int i;
    int used;
    u8 *option;
    u16 *key;

    sp->unk2E0 = 0;
    sp->unk2DC = 1;
    func_80057a3c();
    while (1) {
        if (sp->unk2DC == 1) {
            if (DAT_801cd180.unk4 & 0x2000) {
                func_80057cec();
                DAT_801cd180.unk4 = 0;
                break;
            }
            if (DAT_801cd180.unkC & 0x10) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 == 0)
                    sp->unk2E0 = 5;
                else
                    sp->unk2E0--;
            }
            if (DAT_801cd180.unkC & 0x40) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 == 5)
                    sp->unk2E0 = 0;
                else
                    sp->unk2E0++;
            }
            if (DAT_801cd180.unk4 & 0x4000) {
                DAT_801cd180.unk4 = 0;
                if (sp->unk2E0 == 5) {
                    func_80057cc8();
                    key = DAT_801cd180.unk22;
                    for (i = 0; i < 12; i++, key++) {
                        if (*key & 0x1000)
                            sp->unk0.levels.level[i] = 0;
                        else if (*key & 0x2000)
                            sp->unk0.levels.level[i] = 1;
                        else if (*key & 0x8000)
                            sp->unk0.levels.level[i] = 2;
                        else if (*key & 0x4000)
                            sp->unk0.levels.level[i] = 3;
                        else if (*key & 0x80)
                            sp->unk0.levels.level[i] = 4;
                        else if (*key & 0x10)
                            sp->unk0.levels.level[i] = 5;
                        else if (*key & 0x20)
                            sp->unk0.levels.level[i] = 6;
                        else if (*key & 0x40)
                            sp->unk0.levels.level[i] = 7;
                        else if (*key & 0x400)
                            sp->unk0.levels.level[i] = 8;
                        else if (*key & 0x100)
                            sp->unk0.levels.level[i] = 9;
                        else if (*key & 0x800)
                            sp->unk0.levels.level[i] = 10;
                        else if (*key & 0x200)
                            sp->unk0.levels.level[i] = 11;
                    }
                    sp->unk2E0 = 0;
                    sp->unk2DC = 2;
                    sp->unk2E2 = 0;
                    sp->unk0.levels.flags = 0;
                }
            }
            if (DAT_801cd180.unkC & 0x20) {
                option = &DAT_801cd120.unk1;
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 > 0) {
                    if (sp->unk2E0 < 5)
                        option[sp->unk2E0] = 0;
                } else if (option[sp->unk2E0] < 63) {
                    option[sp->unk2E0]++;
                }
            }
            if (DAT_801cd180.unkC & 0x80) {
                option = &DAT_801cd120.unk1;
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 > 0) {
                    if (sp->unk2E0 < 5)
                        option[sp->unk2E0] = 1;
                } else if (option[sp->unk2E0] != 0) {
                    option[sp->unk2E0]--;
                }
            }
        } else if (sp->unk2E2 == 0) {
            if (DAT_801cd180.unk4 & 0x2000) {
                if (sp->unk0.levels.flags == 0) {
                    func_8005738c();
                    func_80057cc8();
                    DAT_801cd180.unk4 = 0;
                    sp->unk2DC = 1;
                    sp->unk2E0 = 5;
                } else {
                    func_80057d34();
                    DAT_801cd180.unk4 = 0;
                }
            }
            if (DAT_801cd180.unk4 & 0x4000) {
                DAT_801cd180.unk4 = 0;
                if (sp->unk2E0 < 12) {
                    func_80057cc8();
                    sp->unk2E4 = 0;
                    sp->unk2E2 = 1;
                } else if (sp->unk2E0 == 12) {
                    func_80057cc8();
                    func_800573bc();
                    func_8005738c();
                    sp->unk0.levels.level[0] = 5;
                    sp->unk0.levels.level[1] = 7;
                    sp->unk0.levels.level[2] = 4;
                    sp->unk0.levels.level[3] = 6;
                    sp->unk0.levels.level[4] = 8;
                    sp->unk0.levels.level[5] = 10;
                    sp->unk0.levels.level[6] = 9;
                    sp->unk0.levels.level[7] = 11;
                    sp->unk0.levels.level[9] = 2;
                    sp->unk0.levels.flags = 0;
                    sp->unk0.levels.level[8] = 0;
                    sp->unk0.levels.level[11] = 1;
                    sp->unk0.levels.level[10] = 3;
                } else if (sp->unk0.levels.flags == 0) {
                    func_8005738c();
                    func_80057cc8();
                    DAT_801cd180.unk4 = 0;
                    sp->unk2DC = 1;
                    sp->unk2E0 = 5;
                } else {
                    func_80057d34();
                    DAT_801cd180.unk4 = 0;
                }
            }
            if (DAT_801cd180.unk4 & 8) {
                func_80057d10();
                DAT_801cd180.unk4 = 0;
                sp->unk2E0 = 13;
            }
            if (DAT_801cd180.unkC & 0x10) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 == 0)
                    sp->unk2E0 = 12;
                else if (sp->unk2E0 == 6)
                    sp->unk2E0 = 13;
                else if (sp->unk2E0 == 12)
                    sp->unk2E0 = 5;
                else if (sp->unk2E0 == 13)
                    sp->unk2E0 = 11;
                else
                    sp->unk2E0--;
            }
            if (DAT_801cd180.unkC & 0x40) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 == 12)
                    sp->unk2E0 = 0;
                else if (sp->unk2E0 == 5)
                    sp->unk2E0 = 12;
                else if (sp->unk2E0 == 11)
                    sp->unk2E0 = 13;
                else if (sp->unk2E0 == 13)
                    sp->unk2E0 = 6;
                else
                    sp->unk2E0++;
            }
            if (DAT_801cd180.unkC & 0x20) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 < 6)
                    sp->unk2E0 += 6;
                else if (sp->unk2E0 == 12)
                    sp->unk2E0 = 13;
                else if (sp->unk2E0 == 13)
                    sp->unk2E0 = 12;
                else
                    sp->unk2E0 -= 6;
            }
            if (DAT_801cd180.unkC & 0x80) {
                func_80057d10();
                DAT_801cd180.unkC = 0;
                if (sp->unk2E0 < 7)
                    sp->unk2E0 += 6;
                else if (sp->unk2E0 == 12)
                    sp->unk2E0 = 13;
                else if (sp->unk2E0 == 13)
                    sp->unk2E0 = 12;
                else
                    sp->unk2E0 -= 6;
            }
        } else if (DAT_801cd180.unk4) {
            key = &DAT_801cd180.unk22[sp->unk2E0];
            if (DAT_801cd180.unk4 & 0x1000) {
                sp->unk0.levels.level[sp->unk2E0] = 0;
                *key = 0x1000;
            } else if (DAT_801cd180.unk4 & 0x2000) {
                sp->unk0.levels.level[sp->unk2E0] = 1;
                *key = 0x2000;
            } else if (DAT_801cd180.unk4 & 0x8000) {
                sp->unk0.levels.level[sp->unk2E0] = 2;
                *key = 0x8000;
            } else if (DAT_801cd180.unk4 & 0x4000) {
                sp->unk0.levels.level[sp->unk2E0] = 3;
                *key = 0x4000;
            } else if (DAT_801cd180.unk4 & 0x80) {
                sp->unk0.levels.level[sp->unk2E0] = 4;
                *key = 0x80;
            } else if (DAT_801cd180.unk4 & 0x10) {
                sp->unk0.levels.level[sp->unk2E0] = 5;
                *key = 0x10;
            } else if (DAT_801cd180.unk4 & 0x20) {
                sp->unk0.levels.level[sp->unk2E0] = 6;
                *key = 0x20;
            } else if (DAT_801cd180.unk4 & 0x40) {
                sp->unk0.levels.level[sp->unk2E0] = 7;
                *key = 0x40;
            } else if (DAT_801cd180.unk4 & 0x400) {
                sp->unk0.levels.level[sp->unk2E0] = 8;
                *key = 0x400;
            } else if (DAT_801cd180.unk4 & 0x100) {
                sp->unk0.levels.level[sp->unk2E0] = 9;
                *key = 0x100;
            } else if (DAT_801cd180.unk4 & 0x800) {
                sp->unk0.levels.level[sp->unk2E0] = 10;
                *key = 0x800;
            } else if (DAT_801cd180.unk4 & 0x200) {
                sp->unk0.levels.level[sp->unk2E0] = 11;
                *key = 0x200;
            }
            sp->unk0.levels.flags = 0;
            used = 15;
            for (i = 0; i < 12; i++) {
                if (DAT_801cd180.unk22[i] & used)
                    sp->unk0.levels.flags |= DAT_801cd180.unk22[i];
                else
                    used |= DAT_801cd180.unk22[i];
            }
            DAT_801cd180.unk4 = 0;
            DAT_801cd180.unkC = 0;
            sp->unk2E2 = 0;
        }
        func_800164b4();
        if (sp->unk2DC == 2)
            func_8002b84c();
        else
            func_8002b2e8();
        sp->unk2E4 = (sp->unk2E4 + 1) % 32;
    }
    func_80057a3c();
}
