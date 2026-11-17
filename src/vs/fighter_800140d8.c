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
 * Not exact: the page counter is spilled instead of the second-model
 * offset (64-byte frame, retail 80); the template-copy base temporary
 * and loop-update schedule differ.
 */
ADDRESS(0x800140d8, 0x3a8)
void func_800140d8(void)
{
    VsFighter *f;
    int member;
    int i;
    int page;
    int clut_y;

    for (i = 0, page = 0, clut_y = 0, f = DAT_800633b0.fighter;
         i < 2; i++, page += 4, clut_y += 12, f++) {
        member = DAT_801624b8[i].slot[DAT_80063338] * 5;
        func_8002ee90(0, member + 1, (u_long *)f);
        func_80012b50((u_long *)f, page, clut_y);
        DrawSync(0);
        func_8002ee90(0, member + 2, (u_long *)DAT_801604b0[i].head);
        func_8002ee90(0, member + 3, (u_long *)f);
        func_80012a70(i);
        if (func_8002ee90(0, member + 4, DAT_800633b0.fighter[i].unk4B000))
            func_8002345c((ModelFile *)DAT_800633b0.fighter[i].unk4B000);
        else
            f->unk4B000[0] = 0;
        func_8002ee90(0, member, (u_long *)f);
        if (((StAnimFile *)f->file)->unk6) {
            f->unk4DF10 = 1;
            func_80023764(i, (StAnimFile *)f->file);
        } else {
            f->unk4DF10 = 2;
            func_80027fec(i, (StAnimFile *)f->file);
        }
        func_80012d10(i);
        f->unk4DF18 = DAT_8004a224[DAT_801624b8[i].slot[DAT_80063338]];
        f->unk4DF0E = 0;
        f->unk4DEEA = 0;
        f->unk4DEF0.vz = 0;
        f->unk4DF00 = 0;
        f->unk4DEE8 = f->unk4DF18.unk0[0].unk0;
        f->unk4DEEC = f->unk4DF18.unk0[0].unk2;
        f->unk4DF08 = f->unk4DF18.unk1B8;
        f->unk4DF0A = f->unk4DF18.unk1BA;
        f->unk4DEF0.vy = -f->unk4DF18.unk1B2;
        f->unk4DF02 = i == 0 ? 3072 : 1024;
        f->unk4DF12 = 128;
        f->unk4DF04 = 0;
        f->unk4DEE0 = 0;
        f->unk4DEE4 = 0;
        f->unk4DF14 = 0;
        f->unk4DF0C = 0;
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
