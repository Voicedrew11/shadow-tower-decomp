/*
 * VS.EXE: an item availability test on the two save records (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/card.h>

/*
 * Whether `player`'s save lacks an item the other save has: an item id
 * (0..265) the other record marks and this one does not, that is not
 * flagged here without its second mark, and that is none of the other
 * record's 21 listed ids.
 */
/* Not exact: register choice (retail keeps the other record in $a3, the
 * item id in $a0 and the bit's 1 in $a1), one word longer here. */
ADDRESS(0x800193e8, 0x224)
int func_800193e8(int player)
{
    int found = 0;
    VsSaveSub *mine = &DAT_801a0a80[player].unk18BC;
    VsSaveSub *other = &DAT_801a0a80[~player & 1].unk18BC;
    int i;

    for (i = 0; i < 266; i++) {
        if (mine->unk150[i] != 0 || other->unk150[i] == 0)
            continue;
        if (DAT_801a0a80[player].unk14CC[i / 32] & (1 << (i % 32))) {
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
