/*
 * VS.EXE: the team screen's three slot pictures (WIP module): each slot's
 * 96x48 picture lives at its scratchpad VRAM position and is parked in
 * the column at x 832 while the team is edited.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/menu.h>

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
/* Not exact: retail stores the fourth rectangle's size after loading the
 * call's destination (here before its position loads). */
ADDRESS(0x80017da4, 0x244)
void func_80017da4(VsMenuScratch *sp, int player, VsTeamEdit *edit)
{
    RECT r;
    int picked = edit->unk4;
    int cursor = ~sp->unk1D0;

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
    r.w = 96;
    r.h = 48;
    r.x = sp->unk0.team.pos[-sp->unk1D0 + 11].vx;
    r.y = sp->unk0.team.pos[-sp->unk1D0 + 11].vy;
    MoveImage2(&r, sp->unk0.team.pos[edit->unk4 + 12].vx, sp->unk0.team.pos[edit->unk4 + 12].vy);
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
