/*
 * VS.EXE: the startup card check (WIP module). ST.EXE links the same
 * routine (unreconstructed there).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <vs/menu.h>
#include <vs/card.h>

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
