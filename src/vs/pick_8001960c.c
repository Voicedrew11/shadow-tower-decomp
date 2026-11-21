/*
 * VS.EXE: the fighter pick list (WIP module): lists the ids `player` may
 * pick from both memory card saves, lets the player scroll it (showing the
 * picture of the id under the cursor) and stores the pick in
 * DAT_80063330[player].
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <MALLOC.H>
#include <MEMORY.H>
#include <LIBAPI.H>
#define VS_PICKED_PAIR
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/pad.h>
#include <vs/sound.h>
#include <vs/funcs.h>

/* func_8001960c's two unused picture paths. */
RODATA(0x80011110, 0x38)

int func_8002ee90(int archive, int member, u_long *buf);
void func_8001d39c(void);
void func_8001cac0(int archive, int member, int x, int y);
void func_800228d8(int page);
void exit(int code);

#define REPEAT (DAT_801603c0.pads[0].unkC | DAT_801603c0.pads[1].unkC)
#define PRESSED (DAT_801603c0.pads[0].unk8 | DAT_801603c0.pads[1].unk8)

#define TOP (sp->unk0.menu.half[0])
#define COUNT (sp->unk0.menu.half[1])
#define IDS (sp->unk0.menu.list.ids)

ADDRESS(0x8001960c, 0xb18)
void func_8001960c(int player)
{
    DVECTOR unused0;
    DVECTOR unused1 = {640, 0};
    char path1[28] = "q:\\st\\itemcg\\icon1\\0000.tim";
    char path5[28] = "q:\\st\\itemcg\\icon5\\0000.tim";
    DVECTOR tex = {896, 448};
    DVECTOR clut = {544, 502};
    DVECTOR pos;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    VsSaveSub *mine;
    VsSaveSub *other;
    u_long *buf;
    s16 shown;
    s16 id;
    int i;
    int j;

    buf = func_8002398c(0x1000);
    func_8002ee90(5, 6, buf);
    func_80012b50(buf, 0, 0);
    DrawSync(0);
    free(buf);
    COUNT = 0;
    IDS = func_8002398c(0x276);
    if (IDS == NULL)
        exit(1);
    memset((u8 *)IDS, 0xff, 0x276);
    mine = &DAT_801a0a80[player].unk18BC;
    other = &DAT_801a0a80[~player & 1].unk18BC;
    COUNT = 1;
    IDS[0] = -2;
    for (i = 0; i < 266; i++) {
        u32 mask;
        u32 word;

        if (mine->unk150[i] != 0 || other->unk150[i] == 0)
            continue;
        mask = 1;
        word = DAT_801a0a80[player].unk14CC[i / 32];
        mask <<= i % 32;
        if ((word & mask) && mine->unk14[i] == 0)
            continue;
        if (other->unk2B4[0] == i || other->unk2B4[1] == i || other->unk2B4[2] == i
            || other->unk2B4[3] == i || other->unk2B4[4] == i || other->unk2B4[5] == i
            || other->unk2B4[6] == i || other->unk2B4[7] == i || other->unk2B4[8] == i
            || other->unk2B4[9] == i || other->unk2B4[10] == i || other->unk2B4[11] == i
            || other->unk2B4[12] == i || other->unk2B4[13] == i || other->unk2B4[14] == i
            || other->unk2B4[15] == i || other->unk2B4[16] == i || other->unk2B4[17] == i
            || other->unk2B4[18] == i || other->unk2B4[19] == i || other->unk2B4[20] == i)
            continue;
        IDS[COUNT++] = i;
    }
    sp->unk1CC = 2;
    sp->unk1D0 = 0;
    TOP = 0;
    if (DAT_80063330[player] != -1) {
        /* The count is never stepped: the search ends only by its match. */
        for (j = 0; COUNT != 0; j++) {
            if (DAT_80063330[player] == IDS[j])
                break;
        }
        if (COUNT < 12 || j < 6) {
            sp->unk1D0 = j;
        } else if (j > COUNT - 5) {
            TOP = COUNT - 11;
            sp->unk1D0 = j - TOP;
        } else {
            TOP = j - 5;
            sp->unk1D0 = 5;
        }
    }
    shown = -1;
    func_8001d39c();
    DrawSync(0);
    for (;;) {
        if (player) {
            DAT_801603c0.pads[0].unk8 = 0;
            DAT_801603c0.pads[0].unkC = 0;
            DAT_801603c0.pads[0].unk0 = 0;
        } else {
            DAT_801603c0.pads[1].unk8 = 0;
            DAT_801603c0.pads[1].unkC = 0;
            DAT_801603c0.pads[1].unk0 = 0;
        }
        if (PRESSED & 0x2000) {
            func_8001c8f8();
            if (sp->unk1CC & 0x8000) {
                sp->unk1CC &= 0x7fff;
            } else {
                while ((s16)func_8001ca18() != 0)
                    ;
                break;
            }
        }
        if (PRESSED & 0x4000) {
            func_8001c978();
            if (sp->unk1CC & 0x8000) {
                s16 chosen = IDS[TOP + sp->unk1D0];

                if (chosen == -2)
                    chosen = -1;
                DAT_80063330[player] = chosen;
                while ((s16)func_8001ca18() != 0)
                    ;
                break;
            }
            if (IDS[(s16)(TOP + sp->unk1D0)] != -1)
                sp->unk1CC |= 0x8000;
        }
        if (!(sp->unk1CC & 0x8000)) {
            if (REPEAT & 0x10) {
                func_8001c8b8();
                if (DAT_801603c0.pads[0].unk0 & 0x8000) {
                    if (COUNT < 11) {
                        sp->unk1D0 = 0;
                    } else {
                        TOP -= 11;
                        if (TOP < 0) {
                            if (TOP + 10 < 0)
                                sp->unk1D0 = 0;
                            TOP = 0;
                        }
                    }
                } else if (sp->unk1D0 > 0) {
                    sp->unk1D0--;
                } else if (TOP > 0) {
                    TOP--;
                } else {
                    TOP = 0;
                    sp->unk1D0 = 0;
                }
                DAT_801603c0.pads[0].unkC = 0;
            }
            if (REPEAT & 0x40) {
                func_8001c8b8();
                if ((DAT_801603c0.pads[0].unk0 | DAT_801603c0.pads[1].unk0) & 0x8000) {
                    if (COUNT >= 11) {
                        TOP += 11;
                        if (TOP > COUNT - 11) {
                            if (TOP >= COUNT)
                                sp->unk1D0 = 10;
                            TOP = COUNT - 11;
                        }
                    } else {
                        sp->unk1D0 = COUNT - 1;
                    }
                } else if (COUNT - 1 > sp->unk1D0 + TOP) {
                    if (sp->unk1D0 < 10) {
                        sp->unk1D0++;
                    } else if (TOP < COUNT - 11) {
                        TOP++;
                    } else {
                        TOP = COUNT - 11;
                        sp->unk1D0 = COUNT - TOP - 1;
                    }
                }
            }
            if (REPEAT & 0x800) {
                func_8001c8b8();
                if (COUNT >= 11) {
                    TOP -= 11;
                    if (TOP < 0) {
                        if (TOP + 10 < 0)
                            sp->unk1D0 = 0;
                        TOP = 0;
                    }
                } else {
                    sp->unk1D0 = 0;
                    if (REPEAT & 0x200)
                        func_8001c8b8();
                }
                if (COUNT >= 11) {
                    TOP += 11;
                    if (TOP > COUNT - 11) {
                        if (TOP >= COUNT)
                            sp->unk1D0 = 10;
                        TOP = COUNT - 11;
                    }
                } else {
                    sp->unk1D0 = COUNT - 1;
                }
            }
            if (sp->unk1CC == 2 && (s16)func_8001ca18() == 0) {
                id = IDS[TOP + sp->unk1D0];
                if (shown != id && id != -1 && id != -2) {
                    func_8001cac0(11, DAT_80191d00.unk0118[id].unk5[0] + 200, 640, 0);
                    shown = id;
                }
            }
        }
        DAT_801603c0.pads[0].unk8 = 0;
        DAT_801603c0.pads[1].unk8 = 0;
        DAT_801603c0.pads[0].unkC = 0;
        DAT_801603c0.pads[1].unkC = 0;
        func_800228d8(player);
        sp->unk1D4 = (sp->unk1D4 + 1) % 32;
    }
    free(IDS);
    pos.vx = 640;
    pos.vy = 124;
    if (DAT_80063330[0] != -1) {
        func_8002ee90(11, DAT_80191d00.unk0118[DAT_80063330[0]].unk5[0], DAT_801a4590.unk4A4);
        func_80012c2c(DAT_801a4590.unk4A4, &pos, NULL);
        DrawSync(0);
    }
    pos.vy += 28;
    if (DAT_80063334 != -1) {
        func_8002ee90(11, DAT_80191d00.unk0118[DAT_80063334].unk5[0], DAT_801a4590.unk4A4);
        func_80012c2c(DAT_801a4590.unk4A4, &pos, NULL);
        DrawSync(1);
    }
}
