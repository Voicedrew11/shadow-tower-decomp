/*
 * VS.EXE: clearing a 2000-record table (WIP module), shaped as ST.EXE's
 * func_80016878 (src/st/texanim.c), which frees the 16 animated-texture
 * slots.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>

ADDRESS(0x8002d710, 0x28)
void func_8002d710(void)
{
    VsRecord20 *rec = DAT_801c4a58;
    int i = 2000;

    while (i--) {
        rec->unk10 = 0;
        rec++;
    }
}
