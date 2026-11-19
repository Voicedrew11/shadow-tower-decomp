/*
 * VS.EXE: the round load (WIP module split from src/vs/main.c: func_800140d8 is
 * not exact, so the rest of the start-up module keeps its retail offsets).
 * $gp-relative accesses to the round state, so -G8.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <INLINE_C.H>
#include <st/anim.h>
#include <st/mesh.h>
#include <vs/data.h>
#include <vs/buffer.h>
#include <vs/menu.h>
#include <vs/sound.h>
#include <vs/funcs.h>

#define PACKET_WORDS (0x20000 / 4)

/* The fighter being loaded. */
#define FIGHTER (DAT_800633b0.fighter[i])

/* Loads archive member `member` into `buf`, nonzero on success
 * (src/vs/cd_8002e5e4.c). Declared without a prototype: the callers pass
 * the numbers as unmasked ints. */
int func_8002ee90();

/* Tentative definitions: ASPSX needs the size to address these $gp-relative.
 * An earlier object allocates them (docs/executable-linking.md). */
s32 DAT_80063338;

/* Loads both fighters of the current round: each one's model, sound bank,
 * animations and second model from archive 0 (five members per
 * character), registers its animations, and resets its state from its
 * character's data block.
 *
 * Not exact: the frame only. Retail's is 80 bytes (ours 64): sp+16..31 is
 * never referenced and the loop's spilled offset sits at sp+32. Sixteen
 * bytes ahead of the spill slot are either a 32-byte outgoing argument
 * area (a call with seven or eight arguments that left no code) or an
 * unused 16-byte local; nothing in the image names either, so none is
 * invented. The code itself is retail's once the fighter is indexed as
 * DAT_800633b0.fighter[i] at every use and the page and palette row are
 * i * 4 and i * 12 at the call.
 */
ADDRESS(0x800140d8, 0x3a8)
void func_800140d8(void)
{
    int member;
    int i;

    for (i = 0; i < 2; i++) {
        member = DAT_801624b8[i].slot[DAT_80063338] * 5;
        func_8002ee90(0, member + 1, (u_long *)&FIGHTER);
        func_80012b50((u_long *)&FIGHTER, i * 4, i * 12);
        DrawSync(0);
        func_8002ee90(0, member + 2, (u_long *)DAT_801604b0[i].head);
        func_8002ee90(0, member + 3, (u_long *)&FIGHTER);
        func_80012a70(i);
        if (func_8002ee90(0, member + 4, FIGHTER.unk4B000))
            func_8002345c((ModelFile *)FIGHTER.unk4B000);
        else
            FIGHTER.unk4B000[0] = 0;
        func_8002ee90(0, member, (u_long *)&FIGHTER);
        if (((StAnimFile *)FIGHTER.file)->unk6) {
            FIGHTER.unk4DF10 = 1;
            func_80023764(i, (StAnimFile *)FIGHTER.file);
        } else {
            FIGHTER.unk4DF10 = 2;
            func_80027fec(i, (StAnimFile *)FIGHTER.file);
        }
        func_80012d10(i);
        FIGHTER.unk4DF18 = DAT_8004a224[DAT_801624b8[i].slot[DAT_80063338]];
        FIGHTER.unk4DF0E = 0;
        FIGHTER.unk4DEEA = 0;
        FIGHTER.unk4DEF0.vz = 0;
        FIGHTER.unk4DF00 = 0;
        FIGHTER.unk4DEE8 = FIGHTER.unk4DF18.unk0[0].unk0;
        FIGHTER.unk4DEEC = FIGHTER.unk4DF18.unk0[0].unk2;
        FIGHTER.unk4DF08 = FIGHTER.unk4DF18.unk1B8;
        FIGHTER.unk4DF0A = FIGHTER.unk4DF18.unk1BA;
        FIGHTER.unk4DEF0.vy = -FIGHTER.unk4DF18.unk1B2;
        FIGHTER.unk4DF02 = i == 0 ? 3072 : 1024;
        FIGHTER.unk4DF12 = 128;
        FIGHTER.unk4DF04 = 0;
        FIGHTER.unk4DEE0 = 0;
        FIGHTER.unk4DEE4 = 0;
        FIGHTER.unk4DF14 = 0;
        FIGHTER.unk4DF0C = 0;
    }
    if (DAT_800633b0.fighter[0].unk4DF18.unk1B4 >= 1400
        || DAT_800633b0.fighter[1].unk4DF18.unk1B4 >= 1400) {
        DAT_800633b0.fighter[0].unk4DEF0.vx = -3072;
        DAT_800633b0.fighter[1].unk4DEF0.vx = 3072;
    } else {
        DAT_800633b0.fighter[0].unk4DEF0.vx = -2048;
        DAT_800633b0.fighter[1].unk4DEF0.vx = 2048;
    }
}
