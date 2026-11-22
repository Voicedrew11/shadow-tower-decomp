/*
 * VS.EXE: the versus setup screens (WIP module): the setup page's loop,
 * the team screen (pictures, slots and input), the fighter pick list,
 * the save item test and the memory card screen with the match banking.
 * One translation unit: retail lays out its constants (0x80011110, the
 * pick list's picture paths and the card screen's file name prefix)
 * before its one switch table (0x80011160, func_80016f10's), the order
 * cc1 writes one object's read-only data in.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>

/* Whether each player's memory card holds a usable save (the "record
 * loaded" bytes), read and set $gp-relative here; main reads them
 * absolutely. This module's small COMMON follows the first .sbss run. */
DATA(0x80063380, 0x1, ".sbss")
u8 DAT_80063380;
DATA(0x80063381, 0x1, ".sbss")
u8 DAT_80063381;

/* Unreferenced (layout evidence only): the eight bytes before the next
 * module's .sbss statics (0x80063390). */
DATA(0x80063388, 0x8, ".sbss")
u8 DAT_80063388[8];

/* The setup, team and pick screens index both picked fighters. */
#define VS_PICKED_PAIR

#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>
#include <vs/menu.h>
#include <vs/main.h>
#include <vs/card.h>
#include <vs/pad.h>
#include <vs/sound.h>
#include <vs/funcs.h>
#include <MALLOC.H>
#include <MEMORY.H>
#include <LIBAPI.H>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <vs/buffer.h>

/* The module's constants, then its switch table. */
RODATA(0x80011110, 0x6c)

/*
 * The versus setup screen's loop: loads the menu
 * tables, fonts and pictures, then runs the seven-line setup page (team,
 * memory card and save entries per player) and its confirm and cancel
 * prompts; returns 1 to start the match, 0 to leave.
 */

int cd_read_member(int archive, int member, u_long *buf);
u32 *copy_words(u32 *dst, u32 *src, int count);
void clear_vram(void);
void func_80016ca8(void);
void func_80016d94(void);
void func_8001789c(void);
void func_80018c84(int player);
void func_8001960c(int player);
int func_8001bb48(int mode, int port);
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

    clear_vram();
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

        cd_read_member(8, 4, message);
        copy_words((u32 *)&DAT_80196230[1], (u32 *)&message[1], 0x1500);
        message = (u_long *)((message[0] + sizeof(*message)) + (u8 *)message);
        func_80012c2c(message + 1, &sp->unk1E0, &sp->unk1E4);
        DrawSync(0);
        message = LOAD_BUFFER;
        cd_read_member(8, 8, message);
        copy_words((u32 *)&DAT_80196230[0], (u32 *)&message[1], 0x1500);
        message = (u_long *)((message[0] + sizeof(*message)) + (u8 *)message);
        func_80012c2c(message + 1, &sp->unk1D8, &sp->unk1DC);
        DrawSync(0);
    }
    buf = LOAD_BUFFER;
    cd_read_member(8, 5, buf);
    func_80012c2c(buf, &sp->unk1E8, NULL);
    DrawSync(0);
    cd_read_member(8, 7, buf);
    func_80012c2c(buf, &tex, &clut);
    DrawSync(0);
    if (DAT_80063330[0] != -1) {
        cd_read_member(11, DAT_80191d00.unk0118[DAT_80063330[0]].unk5[0], buf);
        func_80012c2c(buf, &pos, NULL);
        DrawSync(0);
    }
    pos.vy += 28;
    if (DAT_80063334 != -1) {
        cd_read_member(11, DAT_80191d00.unk0118[DAT_80063334].unk5[0], buf);
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
                    play_confirm_sound();
                    sp->unk1CC = 1;
                    break;
                case 1:
                    if (!(pad_state.pads[0].unk8 & 0x4000))
                        goto buzz;
                    play_confirm_sound();
                    pad_state.pads[0].unk8 = 0;
                    CARD_PAGE(0);
                    break;
                case 2:
                    if (pad_state.pads[1].unk8 & 0x4000) {
                        play_confirm_sound();
                        pad_state.pads[1].unk8 = 0;
                        CARD_PAGE(1);
                        break;
                    }
                    goto buzz;
                case 3:
                    if (!(pad_state.pads[0].unk8 & 0x4000) || !DAT_80063380)
                        goto buzz;
                    play_confirm_sound();
                    pad_state.pads[0].unk8 = 0;
                    TEAM_PAGE(0);
                    break;
                case 4:
                    if ((pad_state.pads[1].unk8 & 0x4000) && DAT_80063381) {
                        play_confirm_sound();
                        pad_state.pads[1].unk8 = 0;
                        TEAM_PAGE(1);
                        break;
                    }
                    goto buzz;
                case 5:
                    if (!(pad_state.pads[0].unk8 & 0x4000) || !DAT_80063380 || !DAT_80063381)
                        goto buzz;
                    play_confirm_sound();
                    pad_state.pads[0].unk8 = 0;
                    SAVE_PAGE(0);
                    break;
                case 6:
                    if (!(pad_state.pads[1].unk8 & 0x4000) || !DAT_80063380 || !DAT_80063381)
                        goto buzz;
                    play_confirm_sound();
                    pad_state.pads[1].unk8 = 0;
                    SAVE_PAGE(1);
                    break;
                buzz:
                    play_refuse_sound();
                    break;
                }
                pad_state.pads[0].unk8 = 0;
                pad_state.pads[1].unk8 = 0;
            }
            if (REPEAT & 0x10) {
                play_cursor_sound();
                pad_state.pads[0].unkC = 0;
                pad_state.pads[1].unkC = 0;
                sp->unk1CE = (sp->unk1CE + 6) % 7;
            }
            if (REPEAT & 0x40) {
                play_cursor_sound();
                pad_state.pads[0].unkC = 0;
                pad_state.pads[1].unkC = 0;
                sp->unk1CE = (sp->unk1CE + 1) % 7;
            }
            if (PRESSED & 0x2000) {
                play_cursor_sound();
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
                play_refuse_sound();
                goto ready_done;
            ready:
                play_confirm_sound();
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
                            play_refuse_sound();
                        }
                    }
                    if (sp->unk1CC == 1) {
                        clear_vram();
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

/*
 * The team screen's picture load: the title strips,
 * each team slot's picture and both players' fighter pictures go to VRAM
 * through the CD state's load buffer.
 */

/* The load's RECT initializer: 8 bytes, so .sdata under -G8. */
SDATA(0x8006323c, 0x8)

int cd_read_member(int archive, int member, u_long *buf);
void func_80021984(void);

ADDRESS(0x8001789c, 0x328)
void func_8001789c(void)
{
    RECT base = {0, 0, 640, 240};
    DVECTOR pos;
    RECT r;
    RECT r2;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    s32 *id;
    int n;
    int x;
    int slot;
    int i;
    u_long *buf;

    sp->unk0.team.pos[0].vx = 640;
    sp->unk0.team.pos[0].vy = 144;
    sp->unk0.team.pos[1].vx = 640;
    sp->unk0.team.pos[1].vy = 192;
    sp->unk0.team.pos[2].vx = 736;
    sp->unk0.team.pos[2].vy = 144;
    sp->unk0.team.pos[3].vx = 736;
    sp->unk0.team.pos[3].vy = 192;
    sp->unk0.team.pos[4].vx = 832;
    sp->unk0.team.pos[4].vy = 144;
    sp->unk0.team.pos[5].vx = 832;
    sp->unk0.team.pos[5].vy = 192;
    sp->unk0.team.pos[6].vx = 928;
    sp->unk0.team.pos[6].vy = 144;
    sp->unk0.team.pos[7].vx = 928;
    sp->unk0.team.pos[7].vy = 172;
    sp->unk0.team.pos[8].vx = 928;
    sp->unk0.team.pos[8].vy = 200;
    sp->unk0.team.pos[15].vx = 640;
    sp->unk0.team.pos[15].vy = 0;
    DAT_801a4590.unk4A0 = DAT_801a4590.unk4A4;
    buf = DAT_801a4590.unk4A0;

    for (n = 0, x = 320; n < 5; n++, x += 64) {
        cd_read_member(5, n + 31, buf);
        pos.vx = base.x + x;
        pos.vy = base.y;
        func_80012c2c(buf, &pos, NULL);
        DrawSync(0);
    }
    id = DAT_801624b8[0].slot;
    for (i = 0; i < 6; i++) {
        slot = *id++;
        if (slot == -1) {
            r2.x = sp->unk0.team.pos[i].vx;
            r2.y = sp->unk0.team.pos[i].vy;
            r2.w = 96;
            r2.h = 48;
            ClearImage(&r2, 0, 0, 0);
        } else {
            int size = cd_read_member(10, slot, DAT_801a4590.unk4A0) * 2048;

            func_80012c2c(DAT_801a4590.unk4A0, &sp->unk0.team.pos[i], NULL);
            DAT_801a4590.unk4A0 = (u_long *)((u8 *)DAT_801a4590.unk4A0 + size);
        }
    }
    for (i = 0; i < 2; i++) {
        if (DAT_80063330[i] == -1) {
            r.x = sp->unk0.team.pos[6 + i].vx;
            r.y = sp->unk0.team.pos[6 + i].vy;
            r.w = 56;
            r.h = 28;
            ClearImage(&r, 0, 0, 0);
        } else {
            int size = cd_read_member(11, DAT_80191d00.unk0118[DAT_80063330[i]].unk5[0],
                                     DAT_801a4590.unk4A0) * 2048;

            func_80012c2c(DAT_801a4590.unk4A0, &sp->unk0.team.pos[6 + i], NULL);
            DAT_801a4590.unk4A0 = (u_long *)((u8 *)DAT_801a4590.unk4A0 + size);
        }
    }
    r.x = sp->unk0.team.pos[8].vx;
    r.y = sp->unk0.team.pos[8].vy;
    r.w = 56;
    r.h = 28;
    ClearImage(&r, 10, 10, 10);
    DrawSync(0);
    func_80021984();
    func_80021984();
}

/*
 * Clears a VRAM block.
 */
/* Clears the 96 x 144 VRAM block at (832, 0) to black in three strips. */
ADDRESS(0x80017bc4, 0xc0)
void func_80017bc4(void)
{
    RECT r;

    setRECT(&r, 832, 0, 96, 48);
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
    setRECT(&r, 832, 48, 96, 48);
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
    setRECT(&r, 832, 96, 96, 48);
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
}

/*
 * The team screen's three slot pictures: each slot's
 * 96x48 picture lives at its scratchpad VRAM position and is parked in
 * the column at x 832 while the team is edited.
 */
/* Parks the three slot pictures at x 832 and takes a copy of `player`'s
 * team into the edit record. */
ADDRESS(0x80017c84, 0x120)
void func_80017c84(VsMenuScratch *sp, int player, VsTeamEdit *edit)
{
    RECT r;

    r.x = sp->unk0.team.pos[12].vx;
    r.y = sp->unk0.team.pos[12].vy;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, 832, 0);
    DrawSync(0);
    r.x = sp->unk0.team.pos[13].vx;
    r.y = sp->unk0.team.pos[13].vy;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, 832, 48);
    DrawSync(0);
    r.x = sp->unk0.team.pos[14].vx;
    r.y = sp->unk0.team.pos[14].vy;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, 832, 96);
    DrawSync(0);
    edit->team.slot[0] = DAT_801624b8[player].slot[0];
    edit->team.slot[1] = DAT_801624b8[player].slot[1];
    edit->team.slot[2] = DAT_801624b8[player].slot[2];
}

/* Stores the edit record as `player`'s team with the picked slot and the
 * cursor's slot swapped, and puts the pictures back swapped too. */
ADDRESS(0x80017da4, 0x244)
void func_80017da4(VsMenuScratch *sp, int player, VsTeamEdit *edit)
{
    RECT r;
    int picked = edit->unk4;
    int cursor = ~sp->unk1D0;
    int dest_x, dest_y;

    DAT_801624b8[player].slot[0] = edit->team.slot[0];
    DAT_801624b8[player].slot[1] = edit->team.slot[1];
    DAT_801624b8[player].slot[2] = edit->team.slot[2];
    DAT_801624b8[player].slot[picked] = edit->team.slot[cursor];
    DAT_801624b8[player].slot[cursor] = edit->team.slot[picked];
    r.x = 832;
    r.y = 0;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, sp->unk0.team.pos[12].vx, sp->unk0.team.pos[12].vy);
    DrawSync(0);
    r.x = 832;
    r.y = 48;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, sp->unk0.team.pos[13].vx, sp->unk0.team.pos[13].vy);
    DrawSync(0);
    r.x = 832;
    r.y = 96;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, sp->unk0.team.pos[14].vx, sp->unk0.team.pos[14].vy);
    DrawSync(0);
    r.x = sp->unk0.team.pos[-sp->unk1D0 + 11].vx;
    r.y = sp->unk0.team.pos[-sp->unk1D0 + 11].vy;
    dest_x = sp->unk0.team.pos[edit->unk4 + 12].vx;
    dest_y = sp->unk0.team.pos[edit->unk4 + 12].vy;
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, dest_x, dest_y);
    DrawSync(0);
    switch (edit->unk4) {
    case 0:
        r.x = 832;
        r.y = 0;
        break;
    case 1:
        r.x = 832;
        r.y = 48;
        break;
    case 2:
        r.x = 832;
        r.y = 96;
        break;
    }
    r.w = 96;
    r.h = 48;
    MoveImage2(&r, sp->unk0.team.pos[-sp->unk1D0 + 11].vx, sp->unk0.team.pos[-sp->unk1D0 + 11].vy);
    DrawSync(0);
}

/* Stores the edit record as `player`'s team without the picked slot: the
 * later slots and their pictures move up and the last picture is
 * cleared. */
ADDRESS(0x80017fe8, 0x258)
void func_80017fe8(VsMenuScratch *sp, int player, VsTeamEdit *edit)
{
    RECT r;

    DAT_801624b8[player].slot[0] = edit->team.slot[0];
    DAT_801624b8[player].slot[1] = edit->team.slot[1];
    DAT_801624b8[player].slot[2] = edit->team.slot[2];
    DAT_801624b8[player].slot[edit->unk4] = -1;
    if (DAT_801624b8[player].slot[0] == -1) {
        r.x = 832;
        r.y = 48;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->unk0.team.pos[12].vx, sp->unk0.team.pos[12].vy);
        DrawSync(0);
        r.x = 832;
        r.y = 96;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->unk0.team.pos[13].vx, sp->unk0.team.pos[13].vy);
        DrawSync(0);
        DAT_801624b8[player].slot[0] = DAT_801624b8[player].slot[1];
        DAT_801624b8[player].slot[1] = DAT_801624b8[player].slot[2];
    } else if (DAT_801624b8[player].slot[1] == -1) {
        r.x = 832;
        r.y = 0;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->unk0.team.pos[12].vx, sp->unk0.team.pos[12].vy);
        DrawSync(0);
        r.x = 832;
        r.y = 96;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->unk0.team.pos[13].vx, sp->unk0.team.pos[13].vy);
        DrawSync(0);
        DAT_801624b8[player].slot[0] = DAT_801624b8[player].slot[0];
        DAT_801624b8[player].slot[1] = DAT_801624b8[player].slot[2];
    } else if (DAT_801624b8[player].slot[2] == -1) {
        r.x = 832;
        r.y = 0;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->unk0.team.pos[12].vx, sp->unk0.team.pos[12].vy);
        DrawSync(0);
        r.x = 832;
        r.y = 48;
        r.w = 96;
        r.h = 48;
        MoveImage2(&r, sp->unk0.team.pos[13].vx, sp->unk0.team.pos[13].vy);
        DrawSync(0);
        DAT_801624b8[player].slot[0] = DAT_801624b8[player].slot[0];
        DAT_801624b8[player].slot[1] = DAT_801624b8[player].slot[1];
    }
    r.x = sp->unk0.team.pos[14].vx;
    r.y = sp->unk0.team.pos[14].vy;
    r.w = 96;
    r.h = 48;
    ClearImage2(&r, 0, 0, 0);
    DrawSync(0);
    DAT_801624b8[player].slot[2] = -1;
}

/*
 * A single TIM upload, as one pass of func_80012c2c.
 */
/* Uploads one TIM image (and its CLUT) to the given positions. */
ADDRESS(0x80018240, 0xac)
void func_80018240(u_long *tim, DVECTOR *pos, DVECTOR *clut_pos)
{
    u_long flags;
    u_long *cursor;
    u_long *clut;
    u_long *clut_data;
    u_long *image;
    u_long *image_data;

    cursor = tim;
    if (cursor[0] != 0x10)
        return;
    cursor += 2;
    flags = tim[1];
    if (flags & 8) {
        clut_data = tim + 5;
        clut = cursor;
        cursor = (u_long *)((u_char *)cursor + tim[2]);
    }
    image = cursor;
    image_data = image + 3;
    cursor = (u_long *)((u_char *)cursor + image[0]);
    if (flags & 8) {
        ((RECT *)(clut + 1))->x = clut_pos->vx;
        ((RECT *)(clut + 1))->y = clut_pos->vy;
        LoadImage((RECT *)(clut + 1), clut_data);
    }
    ((RECT *)(image + 1))->x = pos->vx;
    ((RECT *)(image + 1))->y = pos->vy;
    LoadImage((RECT *)(image + 1), image_data);
}

/*
 * The team screen's input loop: moves the cursor over
 * the team slots and the item grid, picks items into the team, swaps
 * slots, scrolls the grid (reloading the pictures that scroll in) and
 * opens the detail page, until the cancel button leaves.
 */
int cd_read_member(int archive, int member, u_long *buf);
void func_80017bc4(void);
void func_80017c84(VsMenuScratch *sp, int player, VsTeamEdit *edit);
void func_80017da4(VsMenuScratch *sp, int player, VsTeamEdit *edit);
void func_80017fe8(VsMenuScratch *sp, int player, VsTeamEdit *edit);
void func_80018240(u_long *tim, DVECTOR *pos, DVECTOR *clut_pos);
void func_80021c14(int page);

/* The detail page toggle: the attribute figures when nonzero (bss, unowned). */

#define HELD (pad_state.pads[0].unk0 | pad_state.pads[1].unk0)

/* The team being edited: the three ids of `player`. */
#define TEAM (DAT_801624b8[player].slot)

ADDRESS(0x800182ec, 0x998)
void func_800182ec(VsMenuScratch *sp, int player)
{
    VsTeamEdit edit;
    int shown = -1;
    u_long *buf;
    int top;
    int col;
    int row;
    int pick;
    int idx;
    int slot;
    int id;
    int n;
    int size;
    int i;
    int j;
    int k;

    edit.unk0 = 0;
    for (;;) {
        if (player) {
            pad_state.pads[0].unk8 = 0;
            pad_state.pads[0].unkC = 0;
            pad_state.pads[0].unk0 = 0;
        } else {
            pad_state.pads[1].unk8 = 0;
            pad_state.pads[1].unkC = 0;
            pad_state.pads[1].unk0 = 0;
        }
        top = sp->unk1D2;
        if (sp->unk1CC == 1) {
            int cursor = sp->unk1D0;

            if (cursor < 0) {
                col = -1;
                row = ~cursor;
            } else {
                col = (s16)(cursor - (cursor >> 2) * 4);
                row = cursor >> 2;
            }
            if (PRESSED & 0x2000) {
                if (!edit.unk0) {
                    play_cancel_sound();
                    return;
                }
                play_cancel_sound();
                func_80017fe8(sp, player, &edit);
                edit.unk0 = 0;
                sp->unk1D6 = 0;
                func_80017bc4();
            }
            if (!edit.unk0) {
                if (REPEAT & 0x20) {
                    play_cursor_sound();
                    if (col < 0 && !edit.unk0)
                        sp->unk1D0 = row << 2;
                    else if (col < 3)
                        sp->unk1D0++;
                }
                if (!edit.unk0 && (REPEAT & 0x80)) {
                    play_cursor_sound();
                    if (col > 0)
                        sp->unk1D0--;
                    else if (col == 0)
                        sp->unk1D0 = ~row;
                }
            }
            if (REPEAT & 0x10) {
                play_cursor_sound();
                if (edit.unk0) {
                    if (row > 0) {
                        sp->unk1D0++;
                        func_80017da4(sp, player, &edit);
                    }
                } else if (HELD & 0x8000) {
                    sp->unk1D2 -= 3;
                    if (sp->unk1D2 < 0)
                        sp->unk1D2 = 0;
                } else if (sp->unk1D0 < 0) {
                    if (row > 0)
                        sp->unk1D0++;
                } else if (row > 0) {
                    sp->unk1D0 -= 4;
                } else if (sp->unk1D2 > 0) {
                    sp->unk1D2--;
                }
            } else if (REPEAT & 0x40) {
                play_cursor_sound();
                if (edit.unk0) {
                    if (row < 2 && TEAM[row + 1] != -1) {
                        sp->unk1D0--;
                        func_80017da4(sp, player, &edit);
                    }
                } else if (HELD & 0x8000) {
                    if (sp->unk0.team.rows > 2) {
                        sp->unk1D2 += 3;
                        if (sp->unk1D2 > sp->unk0.team.rows - 2)
                            sp->unk1D2 = sp->unk0.team.rows - 2;
                    }
                } else if (sp->unk1D0 < 0) {
                    if (row < 2)
                        sp->unk1D0--;
                } else if (sp->unk1D2 + row < sp->unk0.team.rows) {
                    if (row < 2)
                        sp->unk1D0 += 4;
                    else if (sp->unk1D2 <= sp->unk0.team.rows - 3)
                        sp->unk1D2++;
                }
            } else if (REPEAT & 0x800) {
                play_cursor_sound();
                sp->unk1D2 -= 3;
                if (sp->unk1D2 < 0)
                    sp->unk1D2 = 0;
            } else if (REPEAT & 0x200) {
                play_cursor_sound();
                if (sp->unk0.team.rows > 2) {
                    sp->unk1D2 += 3;
                    if (sp->unk1D2 > sp->unk0.team.rows - 2)
                        sp->unk1D2 = sp->unk0.team.rows - 2;
                }
            } else if (PRESSED & 0x4000) {
                if (edit.unk0) {
                    play_confirm_sound();
                    edit.unk0 = 0;
                    sp->unk1D6 = 0;
                    func_80017bc4();
                } else {
                    pick = sp->unk1D2 * 4 + sp->unk1D0;
                    edit.unk0 = 1;
                    sp->unk1D6 = 1;
                    if (sp->unk1D0 < 0) {
                        edit.unk4 = ~sp->unk1D0;
                        if (TEAM[edit.unk4] != -1) {
                            play_confirm_sound();
                            func_80017c84(sp, player, &edit);
                            goto scrolled;
                        }
                    } else if (TEAM[2] == -1) {
                        if (sp->unk0.team.kinds[pick] == 0xff) {
                            play_refuse_sound();
                            edit.unk0 = 0;
                            sp->unk1D6 = 0;
                            goto scrolled;
                        }
                        for (i = 0; i < 3; i++) {
                            if (TEAM[i] == sp->unk0.team.kinds[pick])
                                break;
                        }
                        if (i == 3) {
                            for (i = 0; i < 3; i++) {
                                if (TEAM[i] == -1) {
                                    RECT r;

                                    TEAM[i] = sp->unk0.team.kinds[pick];
                                    r.x = sp->unk0.team.pos[pick % 12].vx;
                                    r.y = sp->unk0.team.pos[pick % 12].vy;
                                    r.w = 96;
                                    r.h = 48;
                                    MoveImage2(&r, sp->unk0.team.pos[12 + i].vx,
                                               sp->unk0.team.pos[12 + i].vy);
                                    DrawSync(0);
                                    break;
                                }
                            }
                            if (i != 3) {
                                play_confirm_sound();
                                sp->unk1D0 = ~i;
                                edit.unk4 = i;
                                func_80017c84(sp, player, &edit);
                                goto scrolled;
                            }
                        }
                    }
                    play_refuse_sound();
                    edit.unk0 = 0;
                    sp->unk1D6 = 0;
                }
            } else if ((PRESSED & 0x1000) && !edit.unk0) {
                DAT_8006335c = 0;
                play_confirm_sound();
                if (sp->unk1D0 < 0) {
                    id = TEAM[-sp->unk1D0 - 1];
                    if (id != -1) {
                        /* Retail only updates idx when a kind matches. */
                        for (k = 0; k < sp->unk0.team.count; k++) {
                            if (id == sp->unk0.team.kinds[k]) {
                                idx = k;
                                break;
                            }
                        }
                    } else {
                        idx = -1;
                    }
                } else {
                    idx = sp->unk1D2 * 4 + sp->unk1D0;
                    id = sp->unk0.team.kinds[idx];
                    for (k = 0; k < 3; k++) {
                        if (id == TEAM[k]) {
                            idx = -1;
                            break;
                        }
                    }
                }
                if (idx < sp->unk0.team.count && idx != -1) {
                    if (idx != shown) {
                        buf = DAT_801a4590.unk4A4;
                        cd_read_member(10, sp->unk0.team.kinds[idx] + 200, buf);
                        shown = idx;
                        func_80012c2c(&buf[1], &sp->unk0.team.pos[15], NULL);
                        {
                            DVECTOR tex = {960, 446};
                            DVECTOR clut = {560, 497};

                            buf = (u_long *)((buf[0] + sizeof(*buf)) + (u8 *)buf);
                            func_80018240(buf + 1, &tex, &clut);
                        }
                    }
                    sp->unk1CC = 2;
                }
            }
        scrolled:
            if (sp->unk1D2 != top) {
                if (top < sp->unk1D2) {
                    n = (sp->unk1D2 - top) * 4;
                    slot = (top + 3) * 4;
                } else {
                    n = (top - sp->unk1D2) * 4;
                    slot = sp->unk1D2 * 4;
                }
                DAT_801a4590.unk4A0 = DAT_801a4590.unk4A4;
                for (j = 0; j < n; j++, slot++) {
                    if (slot < sp->unk0.team.count) {
                        size = cd_read_member(10, sp->unk0.team.kinds[slot],
                                             DAT_801a4590.unk4A0) * 2048;
                        func_80012c2c(DAT_801a4590.unk4A0, &sp->unk0.team.pos[slot % 12], NULL);
                        DAT_801a4590.unk4A0 = (u_long *)((u8 *)DAT_801a4590.unk4A0 + size);
                    }
                }
                DrawSync(0);
            }
        } else if (PRESSED & 0x2000) {
            play_cancel_sound();
            sp->unk1CC = 1;
        } else if (PRESSED & 0x800) {
            DAT_8006335c = 1;
        } else if (PRESSED & 0x400) {
            DAT_8006335c = 0;
        }
        pad_state.pads[0].unk8 = 0;
        pad_state.pads[1].unk8 = 0;
        pad_state.pads[0].unkC = 0;
        pad_state.pads[1].unkC = 0;
        func_80021c14(player);
        sp->unk1D4 = (sp->unk1D4 + 1) % 32;
    }
}

/*
 * The team screen: lays out the picture positions,
 * lists the fighters `player` owns, loads the message tables, fonts and
 * pictures, runs the screen (func_800182ec) and then reloads the menu
 * tables and both players' fighter pictures.
 */

int cd_read_member(int archive, int member, u_long *buf);
u32 *copy_words(u32 *dst, u32 *src, int count);
void clear_vram(void);
void func_800182ec(VsMenuScratch *sp, int player);

ADDRESS(0x80018c84, 0x764)
void func_80018c84(int player)
{
    DVECTOR tex = {896, 448};
    DVECTOR clut = {544, 502};
    u8 unused[64];
    DVECTOR pos;
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u_long *buf;
    u_long *tim;
    u32 *owned;
    int size;
    int n;
    int i;
    u32 mask;
    u32 word;

    sp->unk1D6 = 0;
    sp->unk0.team.pos[0].vx = 640;
    sp->unk0.team.pos[0].vy = 144;
    sp->unk0.team.pos[1].vx = 640;
    sp->unk0.team.pos[1].vy = 192;
    sp->unk0.team.pos[2].vx = 736;
    sp->unk0.team.pos[2].vy = 144;
    sp->unk0.team.pos[3].vx = 736;
    sp->unk0.team.pos[3].vy = 192;
    sp->unk0.team.pos[4].vx = 832;
    sp->unk0.team.pos[4].vy = 144;
    sp->unk0.team.pos[5].vx = 928;
    sp->unk0.team.pos[5].vy = 0;
    sp->unk0.team.pos[6].vx = 928;
    sp->unk0.team.pos[6].vy = 48;
    sp->unk0.team.pos[7].vx = 928;
    sp->unk0.team.pos[7].vy = 96;
    sp->unk0.team.pos[8].vx = 928;
    sp->unk0.team.pos[8].vy = 144;
    sp->unk0.team.pos[9].vx = 928;
    sp->unk0.team.pos[9].vy = 192;
    sp->unk0.team.pos[10].vx = 640;
    sp->unk0.team.pos[10].vy = 256;
    sp->unk0.team.pos[11].vx = 640;
    sp->unk0.team.pos[11].vy = 304;
    sp->unk0.team.pos[12].vx = 640;
    sp->unk0.team.pos[12].vy = 352;
    sp->unk0.team.pos[13].vx = 640;
    sp->unk0.team.pos[13].vy = 400;
    sp->unk0.team.pos[14].vx = 640;
    sp->unk0.team.pos[14].vy = 448;
    sp->unk0.team.pos[15].vx = 640;
    sp->unk0.team.pos[15].vy = 0;
    sp->unk0.team.kinds = func_8002398c(151);
    sp->unk0.team.count = 0;
    for (i = 0; i < 151; i++)
        sp->unk0.team.kinds[i] = 0xff;
    owned = DAT_801a0a80[player].unk18BC.unk0;
    for (i = 0; i < 151; i++) {
        mask = 1;
        word = owned[i / 32];
        mask <<= i % 32;
        if ((word & mask) && i != 3 &&
            i != 5 && i != 6 && i != 7 && i != 9 && i != 10 && i != 11 && i != 19 &&
            i != 25 && i != 26 && i != 33 && i != 38 && i != 42 && i != 54 && i != 55 &&
            i != 67 && i != 70 && i != 73 && i != 83 && i != 84 && i != 85 && i != 86 &&
            i != 89 && i != 90 && i != 91 && i != 92 && i != 106 && i != 108 && i != 109 &&
            i != 116 && i != 118 && i != 131 && i != 132 && i != 133 && i != 135 &&
            i != 140 && i != 142 && i != 150)
            sp->unk0.team.kinds[sp->unk0.team.count++] = i;
    }
    sp->unk0.team.rows = sp->unk0.team.count / 4;
    sp->unk0.team.last = sp->unk0.team.count % 4;
    if (sp->unk0.team.last == 0) {
        sp->unk0.team.last = 4;
        sp->unk0.team.rows--;
    }
    sp->unk0.team.items = func_8002398c(0x2000);
    cd_read_member(10, 400, (u_long *)sp->unk0.team.items);
    buf = DAT_801a4590.unk4A4;
    cd_read_member(8, 2, buf);
    copy_words((u32 *)&DAT_80196230[1], (u32 *)&buf[1], 0x1500);
    buf = (u_long *)((buf[0] + sizeof(*buf)) + (u8 *)buf);
    func_80012c2c(buf + 1, &sp->unk1E0, &sp->unk1E4);
    DrawSync(0);
    buf = DAT_801a4590.unk4A4;
    cd_read_member(8, 1, buf);
    copy_words((u32 *)&DAT_80196230[0], (u32 *)&buf[1], 0x1500);
    buf = (u_long *)((buf[0] + sizeof(*buf)) + (u8 *)buf);
    func_80012c2c(buf + 1, &sp->unk1D8, &sp->unk1DC);
    DrawSync(0);
    cd_read_member(8, 7, DAT_801a4590.unk4A4);
    func_80012c2c(DAT_801a4590.unk4A4, &tex, &clut);
    DrawSync(0);
    DAT_801a4590.unk4A0 = DAT_801a4590.unk4A4;
    n = 12;
    if (sp->unk0.team.count < 13)
        n = sp->unk0.team.count;
    for (i = 0; i < n; i++) {
        size = cd_read_member(10, sp->unk0.team.kinds[i], DAT_801a4590.unk4A0) * 2048;
        func_80012c2c(DAT_801a4590.unk4A0, &sp->unk0.team.pos[i], NULL);
        DAT_801a4590.unk4A0 = (u_long *)((u8 *)DAT_801a4590.unk4A0 + size);
    }
    if (DrawSync(1) > 8)
        DrawSync(0);
    DAT_801a4590.unk4A0 = DAT_801a4590.unk4A4;
    for (i = 0; i < 3; i++) {
        if (DAT_801624b8[player].slot[i] != -1) {
            size = cd_read_member(10, DAT_801624b8[player].slot[i], DAT_801a4590.unk4A0) * 2048;
            func_80012c2c(DAT_801a4590.unk4A0, &sp->unk0.team.pos[12 + i], NULL);
            DAT_801a4590.unk4A0 = (u_long *)((u8 *)DAT_801a4590.unk4A0 + size);
        }
    }
    clear_vram();
    DrawSync(0);
    sp->unk1CC = 1;
    sp->unk1D0 = 0;
    sp->unk1D2 = 0;
    func_800182ec(sp, player);
    free(sp->unk0.team.kinds);
    free(sp->unk0.team.items);
    pos.vx = 640;
    pos.vy = 124;
    DAT_801a4590.unk4A0 = buf = DAT_801a4590.unk4A4;
    cd_read_member(8, 4, buf);
    copy_words((u32 *)&DAT_80196230[1], (u32 *)&buf[1], 0x1500);
    tim = (u_long *)((u8 *)buf + DAT_801a4590.unk4A4[0]);
    func_80012c2c(tim + 2, &sp->unk1E0, &sp->unk1E4);
    DrawSync(0);
    cd_read_member(8, 8, buf);
    copy_words((u32 *)&DAT_80196230[0], (u32 *)&buf[1], 0x1500);
    tim = (u_long *)((u8 *)buf + DAT_801a4590.unk4A4[0]);
    func_80012c2c(tim + 2, &sp->unk1D8, &sp->unk1DC);
    DrawSync(0);
    cd_read_member(8, 7, buf);
    func_80012c2c(buf, &tex, &clut);
    DrawSync(0);
    if (DAT_80063330[0] != -1) {
        cd_read_member(11, DAT_80191d00.unk0118[DAT_80063330[0]].unk5[0], buf);
        func_80012c2c(buf, &pos, NULL);
        DrawSync(0);
    }
    pos.vy += 28;
    if (DAT_80063334 != -1) {
        cd_read_member(11, DAT_80191d00.unk0118[DAT_80063334].unk5[0], buf);
        func_80012c2c(buf, &pos, NULL);
        DrawSync(1);
    }
    sp->unk1D6 = 0;
}

/*
 * An item availability test on the two save records.
 */
/*
 * Whether `player`'s save lacks an item the other save has: an item id
 * (0..265) the other record marks and this one does not, that is not
 * flagged here without its second mark, and that is none of the other
 * record's 21 listed ids.
 */
ADDRESS(0x800193e8, 0x224)
int func_800193e8(int player)
{
    int found = 0;
    VsSaveSub *mine = &DAT_801a0a80[player].unk18BC;
    VsSaveSub *other = &DAT_801a0a80[~player & 1].unk18BC;
    int i;

    for (i = 0; i < 266; i++) {
        u32 mask;
        u32 word;

        if (mine->unk150[i] != 0 || other->unk150[i] == 0)
            continue;
        mask = 1;
        word = DAT_801a0a80[player].unk14CC[i / 32];
        mask <<= i % 32;
        if (word & mask) {
            if (mine->unk14[i] == 0)
                continue;
        }
        if (other->unk2B4[0] == i || other->unk2B4[1] == i || other->unk2B4[2] == i
            || other->unk2B4[3] == i || other->unk2B4[4] == i || other->unk2B4[5] == i
            || other->unk2B4[6] == i || other->unk2B4[7] == i || other->unk2B4[8] == i
            || other->unk2B4[9] == i || other->unk2B4[10] == i || other->unk2B4[11] == i
            || other->unk2B4[12] == i || other->unk2B4[13] == i || other->unk2B4[14] == i
            || other->unk2B4[15] == i || other->unk2B4[16] == i || other->unk2B4[17] == i
            || other->unk2B4[18] == i || other->unk2B4[19] == i || other->unk2B4[20] == i)
            continue;
        found = 1;
    }
    return found;
}

/*
 * The fighter pick list: lists the ids `player` may
 * pick from both memory card saves, lets the player scroll it (showing the
 * picture of the id under the cursor) and stores the pick in
 * DAT_80063330[player].
 */

int cd_read_member(int archive, int member, u_long *buf);
void clear_vram(void);
void tim_load_member_async(int archive, int member, int x, int y);
void func_800228d8(int page);
void exit(int code);

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
    cd_read_member(5, 6, buf);
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
    clear_vram();
    DrawSync(0);
    for (;;) {
        if (player) {
            pad_state.pads[0].unk8 = 0;
            pad_state.pads[0].unkC = 0;
            pad_state.pads[0].unk0 = 0;
        } else {
            pad_state.pads[1].unk8 = 0;
            pad_state.pads[1].unkC = 0;
            pad_state.pads[1].unk0 = 0;
        }
        if (PRESSED & 0x2000) {
            play_cancel_sound();
            if (sp->unk1CC & 0x8000) {
                sp->unk1CC &= 0x7fff;
            } else {
                while ((s16)cd_queue_busy() != 0)
                    ;
                break;
            }
        }
        if (PRESSED & 0x4000) {
            play_confirm_sound();
            if (sp->unk1CC & 0x8000) {
                s16 chosen = IDS[TOP + sp->unk1D0];

                if (chosen == -2)
                    chosen = -1;
                DAT_80063330[player] = chosen;
                while ((s16)cd_queue_busy() != 0)
                    ;
                break;
            }
            if (IDS[(s16)(TOP + sp->unk1D0)] != -1)
                sp->unk1CC |= 0x8000;
        }
        if (!(sp->unk1CC & 0x8000)) {
            if (REPEAT & 0x10) {
                play_cursor_sound();
                if (pad_state.pads[0].unk0 & 0x8000) {
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
                pad_state.pads[0].unkC = 0;
            }
            if (REPEAT & 0x40) {
                play_cursor_sound();
                if ((pad_state.pads[0].unk0 | pad_state.pads[1].unk0) & 0x8000) {
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
                play_cursor_sound();
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
                        play_cursor_sound();
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
            if (sp->unk1CC == 2 && (s16)cd_queue_busy() == 0) {
                id = IDS[TOP + sp->unk1D0];
                if (shown != id && id != -1 && id != -2) {
                    tim_load_member_async(11, DAT_80191d00.unk0118[id].unk5[0] + 200, 640, 0);
                    shown = id;
                }
            }
        }
        pad_state.pads[0].unk8 = 0;
        pad_state.pads[1].unk8 = 0;
        pad_state.pads[0].unkC = 0;
        pad_state.pads[1].unkC = 0;
        func_800228d8(player);
        sp->unk1D4 = (sp->unk1D4 + 1) % 32;
    }
    free(IDS);
    pos.vx = 640;
    pos.vy = 124;
    if (DAT_80063330[0] != -1) {
        cd_read_member(11, DAT_80191d00.unk0118[DAT_80063330[0]].unk5[0], DAT_801a4590.unk4A4);
        func_80012c2c(DAT_801a4590.unk4A4, &pos, NULL);
        DrawSync(0);
    }
    pos.vy += 28;
    if (DAT_80063334 != -1) {
        cd_read_member(11, DAT_80191d00.unk0118[DAT_80063334].unk5[0], DAT_801a4590.unk4A4);
        func_80012c2c(DAT_801a4590.unk4A4, &pos, NULL);
        DrawSync(1);
    }
}

/*
 * Memory card event handling, the card screen and the match
 * banking. ST.EXE links the same routines (unreconstructed
 * there). func_8001c5e0 uses this module's merged "O" literal.
 */

int cd_read_member();

/* The card path pieces ("bu", "0:", "0:*", "*") and a two-byte SJIS digit:
 * short literals, so .sdata under -G8. */
SDATA(0x80063244, 0x13)

/* Clears the four software card events. */
ADDRESS(0x8001a124, 0x54)
void func_8001a124(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    TestEvent(sp->unk0.card.sw_events[0]);
    TestEvent(sp->unk0.card.sw_events[1]);
    TestEvent(sp->unk0.card.sw_events[2]);
    TestEvent(sp->unk0.card.sw_events[3]);
}

/* Clears the four hardware card events. */
ADDRESS(0x8001a178, 0x54)
void func_8001a178(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    TestEvent(sp->unk0.card.hw_events[0]);
    TestEvent(sp->unk0.card.hw_events[1]);
    TestEvent(sp->unk0.card.hw_events[2]);
    TestEvent(sp->unk0.card.hw_events[3]);
}

/* Opens and enables the eight card events. */
ADDRESS(0x8001a1cc, 0x190)
void func_8001a1cc(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    EnterCriticalSection();
    sp->unk0.card.sw_events[0] = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
    sp->unk0.card.sw_events[1] = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
    sp->unk0.card.sw_events[2] = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    sp->unk0.card.sw_events[3] = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);
    sp->unk0.card.hw_events[0] = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
    sp->unk0.card.hw_events[1] = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
    sp->unk0.card.hw_events[2] = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    sp->unk0.card.hw_events[3] = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
    EnableEvent(sp->unk0.card.sw_events[0]);
    EnableEvent(sp->unk0.card.sw_events[1]);
    EnableEvent(sp->unk0.card.sw_events[2]);
    EnableEvent(sp->unk0.card.sw_events[3]);
    EnableEvent(sp->unk0.card.hw_events[0]);
    EnableEvent(sp->unk0.card.hw_events[1]);
    EnableEvent(sp->unk0.card.hw_events[2]);
    EnableEvent(sp->unk0.card.hw_events[3]);
    ExitCriticalSection();
}

/* Disables and closes the eight card events. */
ADDRESS(0x8001a35c, 0xf0)
void func_8001a35c(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    EnterCriticalSection();
    DisableEvent(sp->unk0.card.sw_events[0]);
    DisableEvent(sp->unk0.card.sw_events[1]);
    DisableEvent(sp->unk0.card.sw_events[2]);
    DisableEvent(sp->unk0.card.sw_events[3]);
    DisableEvent(sp->unk0.card.hw_events[0]);
    DisableEvent(sp->unk0.card.hw_events[1]);
    DisableEvent(sp->unk0.card.hw_events[2]);
    DisableEvent(sp->unk0.card.hw_events[3]);
    CloseEvent(sp->unk0.card.sw_events[0]);
    CloseEvent(sp->unk0.card.sw_events[1]);
    CloseEvent(sp->unk0.card.sw_events[2]);
    CloseEvent(sp->unk0.card.sw_events[3]);
    CloseEvent(sp->unk0.card.hw_events[0]);
    CloseEvent(sp->unk0.card.hw_events[1]);
    CloseEvent(sp->unk0.card.hw_events[2]);
    CloseEvent(sp->unk0.card.hw_events[3]);
    ExitCriticalSection();
}

/* Waits for a software card event: 0 done, 1 error, 2 timeout, 3 new card. */
ADDRESS(0x8001a44c, 0xb4)
int func_8001a44c(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    long ioe, error, timeout, new;

    for (;;) {
        ioe = TestEvent(sp->unk0.card.sw_events[0]);
        error = TestEvent(sp->unk0.card.sw_events[1]);
        timeout = TestEvent(sp->unk0.card.sw_events[2]);
        new = TestEvent(sp->unk0.card.sw_events[3]);
        if (ioe == 1)
            return 0;
        if (error == 1)
            return 1;
        if (timeout == 1)
            return 2;
        if (new == 1)
            return 3;
    }
}

/* The same wait on the hardware card events. */
ADDRESS(0x8001a500, 0xb4)
int func_8001a500(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    long ioe, error, timeout, new;

    for (;;) {
        ioe = TestEvent(sp->unk0.card.hw_events[0]);
        error = TestEvent(sp->unk0.card.hw_events[1]);
        timeout = TestEvent(sp->unk0.card.hw_events[2]);
        new = TestEvent(sp->unk0.card.hw_events[3]);
        if (ioe == 1)
            return 0;
        if (error == 1)
            return 1;
        if (timeout == 1)
            return 2;
        if (new == 1)
            return 3;
    }
}

/* Checks that the card file of player `port` holds that player's save. */
ADDRESS(0x8001a5b4, 0x128)
void func_8001a5b4(int port)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char path[32];
    long buf[32];
    int fd;

    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    strcat(path, DAT_801a0a30[port]);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        sp->unk1B8->status = 4;
        return;
    }
    lseek(fd, 512, SEEK_SET);
    read(fd, buf, 128);
    close(fd);
    if (DAT_801a0a80[port].unk0 != buf[0])
        sp->unk1B8->status = 4;
    else
        sp->unk1B8->status = 0;
}

/* Scans card `port`: free space when unformatted space is asked for, then
 * the game's save files and the first unused save number. */
ADDRESS(0x8001a6dc, 0x3a8)
void func_8001a6dc(int port)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    struct DIRENTRY dir[15];
    char path[32];
    long used[8];
    int i, n, total;

    sp->unk1B8 = &sp->unk0.card.ports[port];
    if (sp->unk1CC & 0xff00) {
        strcpy(path, "bu");
        strcat(path, itoa(port));
        strcat(path, "0:*");
        sp->unk1B8->count = 0;
        if (firstfile(path, &dir[sp->unk1B8->count])) {
            do {
                sp->unk1B8->count++;
            } while (nextfile(&dir[sp->unk1B8->count]));
        }
        total = 0;
        for (i = 0; i < sp->unk1B8->count; i++)
            total += dir[i].size;
        if (total > 0x1a000)
            sp->unk1B8->unkC8 = 0;
        else
            sp->unk1B8->unkC8 = 1;
    }
    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    strcat(path, "BASLUS-00863FS_ST");
    strcat(path, "*");
    sp->unk1B8->count = 0;
    if (firstfile(path, &dir[sp->unk1B8->count])) {
        do {
            sp->unk1B8->count++;
        } while (nextfile(&dir[sp->unk1B8->count]));
    }
    if (sp->unk1B8->count == 0) {
        sp->unk1C8 = 1;
        if (sp->unk1CC & 0xff00) {
            if (sp->unk1B8->unkC8 == 0)
                sp->unk1B8->status = 5;
            else
                sp->unk1B8->status = 0;
        } else {
            sp->unk1B8->status = 4;
        }
        return;
    }
    for (i = 0; i < 8; i++)
        used[i] = 0;
    for (i = 0; i < sp->unk1B8->count; i++) {
        strcpy(sp->unk1B8->files[i].name, dir[i].name);
        n = (((u_char *)dir[i].name)[19] - '0') * 10 + (((u_char *)dir[i].name)[20] - '0');
        used[n] = -1;
    }
    for (i = 1; i < 8; i++) {
        if (used[i] != -1)
            break;
    }
    sp->unk1C8 = i;
    sp->unk1B8->status = 0;
}

/* Reads card `port`'s state through the BIOS card events, three tries. */
ADDRESS(0x8001aa84, 0x178)
void func_8001aa84(int port, int new_ok)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int tries;
    int result;

    func_8001a1cc();
    tries = 2;
    do {
        func_8001a124();
        _card_info(port << 4);
        result = func_8001a44c();
        if (result == 0) {
            sp->unk0.card.ports[port].status = 0;
        } else if (result == 1) {
            if (tries)
                continue;
            sp->unk0.card.ports[port].status = 2;
            break;
        } else if (result == 2) {
            if (tries)
                continue;
            sp->unk0.card.ports[port].status = result;
            break;
        } else if (result == 3) {
            sp->unk0.card.ports[port].status = 1;
            if (!new_ok)
                break;
        }
        func_8001a178();
        _card_clear(port << 4);
        if (func_8001a500()) {
            sp->unk0.card.ports[port].status = 2;
            continue;
        }
        func_8001a124();
        _card_load(port << 4);
        result = func_8001a44c();
        if (result == 1 || result == 2) {
            if (tries)
                continue;
            sp->unk0.card.ports[port].status = 2;
            break;
        }
        if (result == 0)
            sp->unk0.card.ports[port].status = 0;
        else if (result == 3)
            sp->unk0.card.ports[port].status = result;
        break;
    } while (tries--);
    func_8001a35c();
}

/* A yes/no prompt: returns the choice (0 or 1), 0 on cancel. */
ADDRESS(0x8001abfc, 0x11c)
s16 func_8001abfc(int first)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u32 buttons;

    sp->unk1D4 = 0;
    sp->unk1C6 = 0;
    for (;;) {
        if (first) {
            pad_state.pads[0].unk8 = 0;
            pad_state.pads[0].unkC = 0;
            pad_state.pads[0].unk0 = 0;
        } else {
            pad_state.pads[1].unk8 = 0;
            pad_state.pads[1].unkC = 0;
            pad_state.pads[1].unk0 = 0;
        }
        buttons = pad_state.pads[0].unk8 | pad_state.pads[1].unk8;
        if (buttons & 0x4000) {
            play_confirm_sound();
            break;
        }
        if (buttons & 0x2000) {
            play_cancel_sound();
            sp->unk1C6 = 0;
            break;
        }
        if (buttons & 0x80) {
            play_cursor_sound();
            sp->unk1C6 = 1;
        }
        if ((pad_state.pads[0].unk8 | pad_state.pads[1].unk8) & 0x20) {
            play_cursor_sound();
            sp->unk1C6 = 0;
        }
        pad_state.pads[0].unk8 = 0;
        pad_state.pads[1].unk8 = 0;
        func_800204b0();
    }
    return sp->unk1C6;
}

/* Picks a card slot: returns nonzero when one was chosen. */
ADDRESS(0x8001ad18, 0x160)
int func_8001ad18(int first)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    u32 buttons;

    sp->unk1D4 = 0;
    for (;;) {
        if (first) {
            pad_state.pads[0].unk8 = 0;
            pad_state.pads[0].unkC = 0;
            pad_state.pads[0].unk0 = 0;
        } else {
            pad_state.pads[1].unk8 = 0;
            pad_state.pads[1].unkC = 0;
            pad_state.pads[1].unk0 = 0;
        }
        buttons = pad_state.pads[0].unk8 | pad_state.pads[1].unk8;
        if (buttons & 0x4000) {
            play_confirm_sound();
            sp->unk1CC <<= 1;
            break;
        }
        if (buttons & 0x2000) {
            play_cancel_sound();
            sp->unk1CC = 0;
            break;
        }
        if ((pad_state.pads[0].unkC | pad_state.pads[1].unkC) & 0x10) {
            play_cursor_sound();
            sp->unk1CC = 2;
        }
        if ((pad_state.pads[0].unkC | pad_state.pads[1].unkC) & 0x40) {
            play_cursor_sound();
            sp->unk1CC = 512;
        }
        pad_state.pads[0].unk8 = 0;
        pad_state.pads[1].unk8 = 0;
        pad_state.pads[0].unkC = 0;
        pad_state.pads[1].unkC = 0;
        func_800204b0();
        sp->unk1D4 = (sp->unk1D4 + 1) % 32;
    }
    return sp->unk1CC != 0;
}

/* Reads the three header values of each listed save file; -1 on a read
 * error. */
ADDRESS(0x8001ae78, 0x154)
int func_8001ae78(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char path[32];
    VsCardHeader header;
    int i, fd;

    i = sp->unk1B8->count;
    while (--i != -1) {
        strcpy(path, "bu");
        strcat(path, itoa(sp->unk1BC));
        strcat(path, "0:");
        strcat(path, sp->unk1B8->files[i].name);
        fd = open(path, O_RDONLY);
        lseek(fd, 0, SEEK_SET);
        if (read(fd, &header, 512) == -1)
            return -1;
        (i + sp->unk1B8->files)->unk15 = header.unk3D;
        (i + sp->unk1B8->files)->unk16 = header.unk3E;
        (i + sp->unk1B8->files)->unk18 = header.unk40;
        close(fd);
    }
    return 1;
}

/* The save file list: returns 1 with the chosen file in unk1C2, 0 on
 * cancel. */
ADDRESS(0x8001afcc, 0x348)
int func_8001afcc(int first)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;

    sp->unk1BE = sp->unk1B8->count;
    sp->unk1C0 = 0;
    sp->unk0.menu.half[1] = 0;
    sp->unk1C2 = 0;
    sp->unk1C4 = 0;
    sp->unk1D4 = 0;
    for (;;) {
        if (first) {
            pad_state.pads[0].unk8 = 0;
            pad_state.pads[0].unkC = 0;
            pad_state.pads[0].unk0 = 0;
        } else {
            pad_state.pads[1].unk8 = 0;
            pad_state.pads[1].unkC = 0;
            pad_state.pads[1].unk0 = 0;
        }
        if ((sp->unk1CC & 0xff00) || ((pad_state.pads[0].unk8 | pad_state.pads[1].unk8) & 0x4000)) {
            play_confirm_sound();
            pad_state.pads[0].unk8 = 0;
            pad_state.pads[1].unk8 = 0;
            sp->unk0.menu.half[1] = 1;
            sp->unk1D4 = 32;
            sp->unk1C6 = 1;
            for (;;) {
                u32 buttons;

                if (first) {
                    pad_state.pads[0].unk8 = 0;
                    pad_state.pads[0].unkC = 0;
                    pad_state.pads[0].unk0 = 0;
                } else {
                    pad_state.pads[1].unk8 = 0;
                    pad_state.pads[1].unkC = 0;
                    pad_state.pads[1].unk0 = 0;
                }
                buttons = pad_state.pads[0].unk8 | pad_state.pads[1].unk8;
                if (buttons & 0x4000) {
                    play_confirm_sound();
                    break;
                }
                if (buttons & 0x2000) {
                    play_cancel_sound();
                    sp->unk1C6 = 0;
                    break;
                }
                if (buttons & 0x80) {
                    play_cursor_sound();
                    sp->unk1C6 = 1;
                }
                if ((pad_state.pads[0].unk8 | pad_state.pads[1].unk8) & 0x20) {
                    play_cursor_sound();
                    sp->unk1C6 = 0;
                }
                pad_state.pads[0].unk8 = 0;
                pad_state.pads[1].unk8 = 0;
                func_800204b0();
            }
            pad_state.pads[0].unk8 = 0;
            pad_state.pads[1].unk8 = 0;
            if (sp->unk1C6)
                goto done;
            if (sp->unk1CC & 0xff00) {
                sp->unk1C2 = -1;
                goto done;
            }
            sp->unk0.menu.half[1] = 0;
            continue;
        }
        if ((pad_state.pads[0].unk8 | pad_state.pads[1].unk8) & 0x2000) {
            play_cancel_sound();
            sp->unk1C2 = -1;
            goto done;
        }
        if ((pad_state.pads[0].unkC | pad_state.pads[1].unkC) & 0x10) {
            play_cursor_sound();
            if (sp->unk1C2 == 0) {
                if (sp->unk1C4 > 0)
                    sp->unk1C4--;
            } else {
                sp->unk1C2--;
            }
        }
        if ((pad_state.pads[0].unkC | pad_state.pads[1].unkC) & 0x40) {
            play_cursor_sound();
            if (sp->unk1C2 == 2) {
                if (sp->unk1C4 < sp->unk1BE - 3)
                    sp->unk1C4++;
            } else if (sp->unk1C2 + sp->unk1C4 < sp->unk1BE - 1) {
                sp->unk1C2++;
            }
        }
        pad_state.pads[0].unk8 = 0;
        pad_state.pads[1].unk8 = 0;
        pad_state.pads[0].unkC = 0;
        pad_state.pads[1].unkC = 0;
        func_800204b0();
        sp->unk1D4 = (sp->unk1D4 + 1) % 32;
        }
done:
    if (sp->unk1C2 == -1)
        return 0;
    sp->unk1C2 += sp->unk1C4;
    return 1;
}

/* Writes `size` bytes of `data` at `offset` in 128-byte blocks of 127 data
 * bytes and an inverted sum byte; returns the sum of those bytes, or -1. */
ADDRESS(0x8001b314, 0xe4)
int func_8001b314(int fd, int offset, u8 *buf, int size)
{
    u8 *data = buf;
    u8 block[128];
    int sum = 0;
    int n, i;

    for (n = 0; size > 0; n++) {
        for (block[127] = i = 0; i < 127; i++) {
            block[i] = *data;
            block[127] += *data;
            data++;
        }
        block[127] = ~block[127];
        lseek(fd, offset + n * 128, SEEK_SET);
        if (write(fd, block, 128) != 128)
            return -1;
        size -= 127;
        sum += block[127];
    }
    return sum & 0xff;
}

/* Reads blocks written by func_8001b314, rereading a bad block up to three
 * times; returns the sum of the check bytes, or -1. */
ADDRESS(0x8001b3f8, 0x10c)
int func_8001b3f8(int fd, int offset, u8 *buf, int size)
{
    u8 *data = buf;
    u8 block[128];
    int retries = 0;
    u8 sum = 0;
    int n = 0;
    u8 check;
    int i;

    while (size > 0) {
        lseek(fd, offset + n * 128, SEEK_SET);
        if (read(fd, block, 128) != 128)
            return -1;
        for (check = i = 0; i < 127; i++) {
            check += block[i];
            *data++ = block[i];
        }
        check = ~check;
        if (check != block[127]) {
            data -= 127;
            if (retries >= 3)
                return -1;
            retries++;
        } else {
            size -= 127;
            n++;
            sum += check;
        }
    }
    return sum;
}

/* Loads the chosen save file into `save`: 1 on success, 0 on failure. */
ADDRESS(0x8001b504, 0x34c)
int func_8001b504(VsSave *save)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    VsSave *loaded;
    char path[32];
    VsCardHeader *dst;
    VsCardHeader *header;
    int tries = 3;
    int fd, sum, i;
    u16 count;

    if (save == &DAT_801a0a80[0])
        dst = &DAT_8015ffc0[0];
    else
        dst = &DAT_8015ffc0[1];
open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->unk1BC));
    strcat(path, "0:");
    strcat(path, sp->unk1B8->files[sp->unk1C2].name);
    fd = open(path, O_WRONLY);
    if (fd == -1) {
        if (!tries)
            return 0;
        tries--;
        goto open;
    }
    header = func_8002398c(512);
read:
    lseek(fd, 0, SEEK_SET);
    if (read(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            free(header);
            return 0;
        }
        tries--;
        goto read;
    }
    loaded = &DAT_800633b0.save;
    sum = func_8001b3f8(fd, 512, (u8 *)loaded, 7560);
    if (sum == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        goto read;
    }
    if (sum != header->unk42) {
        if (!tries) {
            close(fd);
            sp->unk0.menu.half[0] = 349;
            return 0;
        }
        tries--;
        goto read;
    }
    close(fd);
    *dst = *header;
    *save = *loaded;
    count = 0;
    {
        char zero[2] = "\x82\x4f";

        for (i = 0; i < 266; i++) {
            if (save->unk14CC[i / 32] & (1 << (i % 32)))
                count++;
        }
    }
    dst->unk40 = count;
    free(header);
    return 1;
}

/* Writes `save` to its card file, creating it first when asked: 1 on
 * success, 0 on failure. */
ADDRESS(0x8001b850, 0x290)
int func_8001b850(VsSave *save)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char number[3];
    char path[32];
    VsCardHeader *header;
    int tries = 3;
    int blocks = 2;
    int create;
    int fd, sum;

    if (save == &DAT_801a0a80[0])
        header = &DAT_8015ffc0[0];
    else
        header = &DAT_8015ffc0[1];
    create = 0;
open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->unk1BC));
    strcat(path, "0:");
    if (create) {
        strcat(path, "BASLUS-00863FS_ST");
        number[0] = sp->unk1C8 / 10 + '0';
        number[1] = sp->unk1C8 % 10 + '0';
        number[2] = 0;
        strcat(path, number);
        fd = open(path, (blocks << 16) | O_CREAT);
        if (fd == -1)
            goto retry;
        close(fd);
    } else {
        strcat(path, sp->unk1B8->files[sp->unk1C2].name);
    }
    fd = open(path, O_WRONLY);
    if (fd == -1) {
retry:
        if (tries) {
            tries--;
            goto open;
        }
        return 0;
    }
write:
    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    sum = func_8001b314(fd, 512, (u8 *)save, 7560);
    if (sum == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    header->unk42 = sum;
    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    close(fd);
    return 1;
}

/* Formats the card in `port`. */
ADDRESS(0x8001bae0, 0x68)
void func_8001bae0(int port)
{
    char path[32];

    strcpy(path, "bu");
    strcat(path, itoa(port));
    strcat(path, "0:");
    format(path);
}

/* Waits for a new press on `pad`: the controller interrupt sets its
 * pressed bits behind the compiler's back, so the poll reads through a
 * volatile view (retail rereads the halfword after its own store). */
#define WAIT_PRESS(pad)                                         \
    do {                                                        \
        (pad).unk8 = 0;                                         \
        while (!*(volatile u16 *)&(pad).unk8)                   \
            ;                                                   \
        (pad).unk8 = 0;                                         \
    } while (0)

/* Messages func_8001bb48 shows. */
#define MSG_CARD_ERROR 350
#define MSG_NEW_CARD 351
#define MSG_NO_FILE 352
#define MSG_CARD_FULL 353
#define MSG_LOAD_FAILED 354
#define MSG_SAVE_FAILED 355
#define MSG_NO_SAVE 357
#define MSG_NOT_LOADED 358

int func_8001c378(VsSave *save);

/*
 * The card screen of player `port`: loads its pictures, then steps
 * through the card checks (sp->unk1CC holds the step as a moving bit),
 * saving (step bits 0xff00) or loading that player's record, showing a
 * message and waiting for a press when a step fails; 0x80 and 0x8000
 * restart. Reloads the menu pictures and both players' fighter faces
 * before returning 1 when the card was used, else 0.
 */
ADDRESS(0x8001bb48, 0x830)
int func_8001bb48(int mode, int port)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    DVECTOR pos = {896, 448};
    DVECTOR clut = {544, 502};
    DVECTOR unused_pos = {576, 384};
    DVECTOR unused_clut = {544, 502};
    DVECTOR face;
    u_long *buf;
    int started = 0;
    int result;
    int status;

    clear_vram();
    DrawSync(0);
    func_80016d94();
    buf = DAT_801a4590.unk4A4;
    cd_read_member(8, 0, buf);
    copy_words((u32 *)DAT_80196230[0].offsets, (u32 *)(buf + 1), 0x1500);
    buf = (u_long *)((u8 *)buf + (buf[0] + 4));
    func_80012c2c(buf + 1, &sp->unk1D8, &sp->unk1DC);
    DrawSync(0);
    cd_read_member(8, 6, DAT_801a4590.unk4A4);
    func_80012c2c(DAT_801a4590.unk4A4, &pos, &clut);
    DrawSync(0);
    sp->unk0.menu.half[0] = 0;
    sp->unk0.menu.half[1] = 0;
    sp->unk1CA = mode;
    pad_state.pads[0].unk8 = 0;
    pad_state.pads[1].unk8 = 0;
    pad_state.pads[0].unkC = 0;
    pad_state.pads[1].unkC = 0;
again:
    sp->unk1BC = port;
    sp->unk1B8 = &sp->unk0.card.ports[sp->unk1BC];
    if (!func_8001ad18(port)) {
        result = 0;
        goto done;
    }
    result = 0;
    if (sp->unk1CC & 0xff00) {
        if (port == 0) {
            if (!DAT_80063380) {
                sp->unk0.menu.half[0] = MSG_NOT_LOADED;
                sp->unk1CC <<= 5;
                goto wait;
            }
        } else {
            if (!DAT_80063381) {
                sp->unk0.menu.half[0] = MSG_NOT_LOADED;
                sp->unk1CC <<= 5;
                goto wait;
            }
        }
    }
    func_800204b0();
    func_800204b0();
    if (!started) {
        StartCARD();
        started = 1;
        _bu_init();
    }
    func_8001aa84(sp->unk1BC, 1);
    if (sp->unk1B8->status == 2) {
        sp->unk0.menu.half[0] = MSG_CARD_ERROR;
        sp->unk1CC <<= 5;
        goto wait;
    }
    sp->unk1CC <<= 2;
    pad_state.pads[0].unk8 = 0;
    pad_state.pads[1].unk8 = 0;
    pad_state.pads[0].unkC = 0;
    pad_state.pads[1].unkC = 0;
    if (sp->unk1CC & 0xff00) {
        func_8001a5b4(sp->unk1BC);
        if (sp->unk1B8->status == 0) {
            strcpy(sp->unk1B8->files[0].name, DAT_801a0a30[port]);
            sp->unk1B8->count = 1;
            sp->unk1B8->unkC8 = 0;
        } else {
            sp->unk0.menu.half[0] = MSG_NO_SAVE;
            sp->unk1CC <<= 3;
            goto wait;
        }
    } else {
        func_8001a6dc(sp->unk1BC);
        status = sp->unk1B8->status;
        if (status != 0) {
            if (status == 4)
                sp->unk0.menu.half[0] = MSG_NO_FILE;
            else if (status == 5)
                sp->unk0.menu.half[0] = MSG_CARD_FULL;
            sp->unk1CC <<= 3;
            goto wait;
        }
    }
    if (func_8001ae78() == -1) {
        sp->unk0.menu.half[0] = MSG_CARD_ERROR;
        sp->unk1CC <<= 3;
        goto wait;
    }
    if (!func_8001afcc(port)) {
        sp->unk1CC >>= 3;
        goto again;
    }
    sp->unk1CC <<= 1;
    func_800204b0();
    func_800204b0();
    func_8001aa84(sp->unk1BC, 0);
    status = sp->unk1B8->status;
    if (status == 1) {
        sp->unk0.menu.half[0] = MSG_NEW_CARD;
        sp->unk1CC <<= 2;
        goto wait;
    }
    if (status == 2) {
        sp->unk0.menu.half[0] = MSG_CARD_ERROR;
        sp->unk1CC <<= 2;
        goto wait;
    }
    if (sp->unk1CC & 0xff00) {
        DAT_801a0a80[port].unk18BC.unk2E0[0] = DAT_801624b8[port].slot[0];
        DAT_801a0a80[port].unk18BC.unk2E0[1] = DAT_801624b8[port].slot[1];
        DAT_801a0a80[port].unk18BC.unk2E0[2] = DAT_801624b8[port].slot[2];
        result = func_8001c378(&DAT_801a0a80[port]);
    } else {
        result = func_8001b504(&DAT_801a0a80[port]);
    }
    if (!result) {
        if (sp->unk0.menu.half[0] == 0) {
            if (sp->unk1CC & 0xff00)
                sp->unk0.menu.half[0] = MSG_SAVE_FAILED;
            else
                sp->unk0.menu.half[0] = MSG_LOAD_FAILED;
        }
        sp->unk1CC <<= 2;
        goto wait;
    }
    if (!(sp->unk1CC & 0xff00)) {
        DAT_801624b8[port].slot[0] = DAT_801a0a80[port].unk18BC.unk2E0[0];
        DAT_801624b8[port].slot[1] = DAT_801a0a80[port].unk18BC.unk2E0[1];
        DAT_801624b8[port].slot[2] = DAT_801a0a80[port].unk18BC.unk2E0[2];
        DAT_80063334 = -1;
        DAT_80063330[0] = -1;
        strcpy(DAT_801a0a30[port], sp->unk1B8->files[sp->unk1C2].name);
        DAT_80063328[port] = sp->unk1C2;
        if (port == 0)
            DAT_80063380 = 1;
        else
            DAT_80063381 = 1;
    }
    sp->unk1CC <<= 1;
wait:
    func_800204b0();
    func_800204b0();
    if (port)
        WAIT_PRESS(pad_state.pads[1]);
    else
        WAIT_PRESS(pad_state.pads[0]);
    play_cancel_sound();
    if (sp->unk1CC & 0x80) {
        sp->unk1CC = 2;
        goto again;
    }
    if (sp->unk1CC & 0x8000) {
        sp->unk1CC = 512;
        goto again;
    }
    result = 1;
done:
    {
        u_long *menu_buf;
        u_long *tim;

        clear_vram();
        DrawSync(0);
        func_80016ca8();
        face.vx = 640;
        face.vy = 124;
        DAT_801a4590.unk4A0 = menu_buf = DAT_801a4590.unk4A4;
        cd_read_member(8, 4, menu_buf);
        copy_words((u32 *)DAT_80196230[1].offsets, (u32 *)&menu_buf[1], 0x1500);
        tim = (u_long *)((u8 *)menu_buf + DAT_801a4590.unk4A4[0]);
        func_80012c2c(tim + 2, &sp->unk1E0, &sp->unk1E4);
        DrawSync(0);
        cd_read_member(8, 8, menu_buf);
        copy_words((u32 *)DAT_80196230[0].offsets, (u32 *)&menu_buf[1], 0x1500);
        tim = (u_long *)((u8 *)menu_buf + DAT_801a4590.unk4A4[0]);
        func_80012c2c(tim + 2, &sp->unk1D8, &sp->unk1DC);
        DrawSync(0);
        cd_read_member(8, 7, menu_buf);
        func_80012c2c(menu_buf, &pos, &clut);
        DrawSync(0);
        if (DAT_80063330[0] != -1) {
            cd_read_member(11, DAT_80191d00.unk0118[DAT_80063330[0]].unk5[0], menu_buf);
            func_80012c2c(menu_buf, &face, NULL);
            DrawSync(0);
        }
        face.vy += 28;
        if (DAT_80063334 != -1) {
            cd_read_member(11, DAT_80191d00.unk0118[DAT_80063334].unk5[0], menu_buf);
            func_80012c2c(menu_buf, &face, NULL);
            DrawSync(1);
        }
        return result;
    }
}

/* Writes `save` over its player's card file (named in DAT_801a0a30): 1 on
 * success, 0 on failure.
 *
 * func_8001b850 without its create branch: `number` (the new file's digits
 * there) is still declared ahead of `path` and never used.
 */
ADDRESS(0x8001c378, 0x1b8)
int func_8001c378(VsSave *save)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    char number[3];
    char path[32];
    VsCardHeader *header;
    char *name;
    int tries = 3;
    int fd, sum;

    if (save == &DAT_801a0a80[0]) {
        header = &DAT_8015ffc0[0];
        name = DAT_801a0a30[0];
    } else {
        header = &DAT_8015ffc0[1];
        name = DAT_801a0a30[1];
    }
open:
    strcpy(path, "bu");
    strcat(path, itoa(sp->unk1BC));
    strcat(path, "0:");
    strcat(path, name);
    fd = open(path, O_WRONLY);
    if (fd == -1) {
        if (!tries)
            return 0;
        tries--;
        goto open;
    }
write:
    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    sum = func_8001b314(fd, 512, (u8 *)save, 7560);
    if (sum == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    header->unk42 = sum;
    lseek(fd, 0, SEEK_SET);
    if (write(fd, header, 512) == -1) {
        if (!tries) {
            close(fd);
            return 0;
        }
        tries--;
        lseek(fd, 0, SEEK_SET);
        goto write;
    }
    close(fd);
    return 1;
}

/* Starts the card system and checks both players' card files: 1 when
 * either fails the check. */
ADDRESS(0x8001c530, 0xb0)
int func_8001c530(void)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int i;

    sp->unk0.menu.half[0] = 0;
    StartCARD();
    _bu_init();
    for (i = 0; i < 2; i++) {
        sp->unk1BC = i;
        sp->unk1B8 = &sp->unk0.card.ports[sp->unk1BC];
        func_8001a5b4(sp->unk1BC);
        if (sp->unk1B8->status)
            return 1;
    }
    return 0;
}

/*
 * Banks a won match: unless `again`, marks the winner's fighter (side
 * `side`, picked DAT_80063330[side]) as won in its save record (bumping
 * the header's three-digit count the first time) and hands the loser's
 * count of it over; then writes both records back. Returns a bit per
 * player whose write failed.
 *
 * Not exact (frame size only, every instruction agrees): retail's frame is
 * 56 bytes with eight untouched bytes at sp+24, right after `zero`; this
 * build's is 48. The allocation matches, HI is not spilled and nothing is
 * caller-saved, so the slot is a declared local of up to 8 bytes that the
 * code never uses; no twin names it (an unused `char number[3];` as in
 * func_8001c378 would make the function exact).
 */
ADDRESS(0x8001c5e0, 0x2d8)
int func_8001c5e0(int side, int again)
{
    VsMenuScratch *sp = VS_MENU_SCRATCH;
    int failed = 0;
    u16 id = DAT_80063330[side];
    char zero[2] = "\x82\x4f";
    VsSaveSub *win = &DAT_801a0a80[side].unk18BC;
    VsSaveSub *lose = &DAT_801a0a80[1 - side].unk18BC;
    int started = failed;
    u16 count;
    int i;

    if (!again) {
        if (!(DAT_801a0a80[side].unk14CC[id / 32] & (1 << (id % 32)))) {
            /* The count is reached through its own address (header + 0x40),
             * not as a member of the header element. */
            count = *(u16 *)&DAT_8015ffc0[side].unk40 + 1;
            DAT_801a0a80[side].unk1802 = count;
            if (count >= 100) {
                DAT_8015ffc0[side].unk34[0] = zero[0];
                DAT_8015ffc0[side].unk34[1] = zero[1] + count / 100 % 10;
            }
            if (count >= 10) {
                DAT_8015ffc0[side].unk34[2] = zero[0];
                DAT_8015ffc0[side].unk34[3] = zero[1] + count / 10 % 10;
            }
            DAT_8015ffc0[side].unk34[4] = zero[0];
            DAT_8015ffc0[side].unk34[5] = zero[1] + count % 10;
            *(u16 *)&DAT_8015ffc0[side].unk40 = count;
            DAT_801a0a80[side].unk14CC[id / 32] |= 1 << (id % 32);
        }
        win->unk150[id] = 1;
        win->unk2EC[id] = lose->unk2EC[id];
        win->unk14[id] = 0;
        {
            /* Retail loads this 1 on its own: an int-typed value, not the
             * byte constant stored above. */
            int on = 1;

            lose->unk14[id] = on;
        }
        lose->unk150[id] = 0;
        lose->unk2EC[id] = 0;
    }
    if (!started) {
        StartCARD();
        _bu_init();
    }
    for (i = 0; i < 2; i++) {
        sp->unk1BC = i;
        sp->unk1B8 = &sp->unk0.card.ports[sp->unk1BC];
        if (!func_8001c378(&DAT_801a0a80[i]))
            failed |= 1 << i;
    }
    return failed;
}
