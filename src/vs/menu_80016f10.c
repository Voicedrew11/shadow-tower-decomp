/*
 * VS.EXE: the versus setup screen's loop (WIP module): loads the menu
 * tables, fonts and pictures, then runs the seven-line setup page (team,
 * memory card and save entries per player) and its confirm and cancel
 * prompts; returns 1 to start the match, 0 to leave.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>

/* Whether each player's memory card holds a usable save: tentative
 * definitions, read $gp-relative here as in the card screen's module
 * (allocated by an earlier object). */
u8 DAT_80063380;
u8 DAT_80063381;

#include <LIBGTE.H>
#include <LIBGPU.H>
#define VS_PICKED_PAIR
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/pad.h>
#include <vs/sound.h>
#include <vs/funcs.h>

/* func_80016f10's switch table. */
JUMPTABLES(0x80011160, 0x1c)

int func_8002ee90(int archive, int member, u_long *buf);
u32 *func_8002e630(u32 *dst, u32 *src, int count);
void func_8001d39c(void);
void func_80016ca8(void);
void func_80016d94(void);
void func_8001789c(void);
void func_80018c84(int player);
void func_8001960c(int player);
void func_8001bb48(int arg0, int player);
int func_8001c530(void);
int func_800193e8(int player);
void func_8001381c(void);
void func_80023308(int page);

/* The CD state's reusable menu-file buffer. */
#define LOAD_BUFFER (DAT_801a4590.unk4A4)

#define PAD(n) (pad_state.pads[(n)])
#define PRESSED (PAD(0).unk8 | PAD(1).unk8)
#define REPEAT (PAD(0).unkC | PAD(1).unkC)

/* Both card entries run the same page transition with a literal port. */
#define CARD_PAGE(port)                     \
    sp->unk1CC = 2;                        \
    sp->unk1D5 = 0;                        \
    sp->unk1CA = 0;                        \
    func_8001bb48(0, port);                 \
    sp->unk1D5 = 1;                        \
    sp->unk1CC = 0

/* Likewise the team page and the save page of each player. */
#define TEAM_PAGE(player)                       sp->unk1D5 = 1;                            func_80018c84(player);                      sp->unk1CC = 0

#define SAVE_PAGE(player)                                          if (DAT_801a0a80[0].unk0 == DAT_801a0a80[1].unk0) {                sp->unk1CC = 9;                                           } else if (func_800193e8(player) == 0) {                           sp->unk1CC = 99;                                          } else {                                                           sp->unk1D5 = 1;                                               func_8001960c(player);                                         sp->unk1CC = 0;                                           }

ADDRESS(0x80016f10, 0x98c)
int func_80016f10(void)
{
    DVECTOR tex = {896, 448};
    DVECTOR clut = {544, 502};
    DVECTOR pos;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u_long *buf;
    u32 b;

    func_8001d39c();
    DrawSync(0);
    func_80016ca8();
    sp->unk1D8.vx = 960;
    sp->unk1D8.vy = 256;
    sp->unk1DC.vx = 560;
    sp->unk1DC.vy = 495;
    sp->unk1E0.vx = 896;
    sp->unk1E0.vy = 256;
    sp->unk1E4.vx = 560;
    sp->unk1E4.vy = 496;
    sp->unk1E8.vx = 768;
    sp->unk1E8.vy = 256;
    pos.vx = 640;
    pos.vy = 124;
    {
        u_long *message = LOAD_BUFFER;

        func_8002ee90(8, 4, message);
        func_8002e630((u32 *)&DAT_80196230[1], (u32 *)&message[1], 0x1500);
        message = (u_long *)((message[0] + sizeof(*message)) + (u8 *)message);
        func_80012c2c(message + 1, &sp->unk1E0, &sp->unk1E4);
        DrawSync(0);
        message = LOAD_BUFFER;
        func_8002ee90(8, 8, message);
        func_8002e630((u32 *)&DAT_80196230[0], (u32 *)&message[1], 0x1500);
        message = (u_long *)((message[0] + sizeof(*message)) + (u8 *)message);
        func_80012c2c(message + 1, &sp->unk1D8, &sp->unk1DC);
        DrawSync(0);
    }
    buf = LOAD_BUFFER;
    func_8002ee90(8, 5, buf);
    func_80012c2c(buf, &sp->unk1E8, NULL);
    DrawSync(0);
    func_8002ee90(8, 7, buf);
    func_80012c2c(buf, &tex, &clut);
    DrawSync(0);
    if (DAT_80063330[0] != -1) {
        func_8002ee90(11, DAT_80191d00.unk0118[DAT_80063330[0]].unk5[0], buf);
        func_80012c2c(buf, &pos, NULL);
        DrawSync(0);
    }
    pos.vy += 28;
    if (DAT_80063334 != -1) {
        func_8002ee90(11, DAT_80191d00.unk0118[DAT_80063334].unk5[0], buf);
        func_80012c2c(buf, &pos, NULL);
        DrawSync(1);
    }
    sp->unk1CE = 0;
    sp->unk1CC = 0;
    func_8001381c();
    sp->unk1D5 = 1;
    for (;;) {
        if (sp->unk1CC == 0) {
            if (PRESSED & 0x4000) {
                switch (sp->unk1CE) {
                case 0:
                    if (DAT_80063330[0] != -1) {
                        if (DAT_80063334 != -1)
                            goto start;
                        sp->unk1CC = 4;
                    } else {
                        if (DAT_80063334 == -1)
                            goto start;
                        sp->unk1CC = 3;
                    }
                    goto buzz;
                start:
                    func_8001c978();
                    sp->unk1CC = 1;
                    break;
                case 1:
                    if (!(pad_state.pads[0].unk8 & 0x4000))
                        goto buzz;
                    func_8001c978();
                    pad_state.pads[0].unk8 = 0;
                    CARD_PAGE(0);
                    break;
                case 2:
                    if (pad_state.pads[1].unk8 & 0x4000) {
                        func_8001c978();
                        pad_state.pads[1].unk8 = 0;
                        CARD_PAGE(1);
                        break;
                    }
                    goto buzz;
                case 3:
                    if (!(pad_state.pads[0].unk8 & 0x4000) || !DAT_80063380)
                        goto buzz;
                    func_8001c978();
                    pad_state.pads[0].unk8 = 0;
                    TEAM_PAGE(0);
                    break;
                case 4:
                    if ((pad_state.pads[1].unk8 & 0x4000) && DAT_80063381) {
                        func_8001c978();
                        pad_state.pads[1].unk8 = 0;
                        TEAM_PAGE(1);
                        break;
                    }
                    goto buzz;
                case 5:
                    if (!(pad_state.pads[0].unk8 & 0x4000) || !DAT_80063380 || !DAT_80063381)
                        goto buzz;
                    func_8001c978();
                    pad_state.pads[0].unk8 = 0;
                    SAVE_PAGE(0);
                    break;
                case 6:
                    if (!(pad_state.pads[1].unk8 & 0x4000) || !DAT_80063380 || !DAT_80063381)
                        goto buzz;
                    func_8001c978();
                    pad_state.pads[1].unk8 = 0;
                    SAVE_PAGE(1);
                    break;
                buzz:
                    func_8001c938();
                    break;
                }
                pad_state.pads[0].unk8 = 0;
                pad_state.pads[1].unk8 = 0;
            }
            if (REPEAT & 0x10) {
                func_8001c8b8();
                pad_state.pads[0].unkC = 0;
                pad_state.pads[1].unkC = 0;
                sp->unk1CE = (sp->unk1CE + 6) % 7;
            }
            if (REPEAT & 0x40) {
                func_8001c8b8();
                pad_state.pads[0].unkC = 0;
                pad_state.pads[1].unkC = 0;
                sp->unk1CE = (sp->unk1CE + 1) % 7;
            }
            if (PRESSED & 0x2000) {
                func_8001c8b8();
                sp->unk1CC = 6;
            }
            if (PRESSED & 8) {
                sp->unk1CE = 0;
                if (DAT_80063330[0] != -1) {
                    if (DAT_80063334 != -1)
                        goto ready;
                    sp->unk1CC = 4;
                } else {
                    if (DAT_80063334 == -1)
                        goto ready;
                    sp->unk1CC = 3;
                }
                func_8001c938();
                goto ready_done;
            ready:
                func_8001c978();
                sp->unk1CC = 1;
            ready_done:
                PAD(0).unk8 = 0;
                PAD(1).unk8 = 0;
                goto next;
            }
        } else if (sp->unk1CC == 1) {
            if (PRESSED & 0x4000) {
                if (DAT_80063228 == 1) {
                    if (DAT_801624b8[0].slot[0] == -1 || DAT_801624b8[1].slot[0] == -1)
                        sp->unk1CC = 2;
                } else if (DAT_801624b8[0].slot[2] == -1 || DAT_801624b8[1].slot[2] == -1) {
                    sp->unk1CC = 2;
                }
                if (sp->unk1CC == 1) {
                    if (DAT_80063330[0] != -1 && DAT_80063334 != -1) {
                        func_80023308(11);
                        func_80023308(11);
                        if (func_8001c530()) {
                            sp->unk1CC = 10;
                            func_8001c938();
                        }
                    }
                    if (sp->unk1CC == 1) {
                        func_8001d39c();
                        DrawSync(0);
                        func_80016d94();
                        func_8001789c();
                        return 1;
                    }
                }
            }
            if (PRESSED & 0x2000)
                sp->unk1CC = 0;
            b = PRESSED;
            if (b & 0x20)
                DAT_80063228 = 3;
            if (b & 0x80)
                DAT_80063228 = 1;
        } else if (sp->unk1CC == 5 || sp->unk1CC == 6) {
            if (PRESSED & 0x4000) {
                if (sp->unk1CC == 5) {
                    pad_state.pads[0].unk8 = 0;
                    pad_state.pads[1].unk8 = 0;
                    return 0;
                }
                sp->unk1CC = 0;
            }
            if (PRESSED & 0x2000)
                sp->unk1CC = 0;
            if (PRESSED & 0x20)
                sp->unk1CC = 6;
            if (PRESSED & 0x80)
                sp->unk1CC = 5;
        } else if (PRESSED & 0x6000) {
            sp->unk1CC = 0;
        }
    next:
        pad_state.pads[0].unk8 = 0;
        pad_state.pads[1].unk8 = 0;
        pad_state.pads[0].unkC = 0;
        pad_state.pads[1].unkC = 0;
        func_80023308(sp->unk1CC);
        sp->unk1D4 = (sp->unk1D4 + 1) % 32;
    }
}
