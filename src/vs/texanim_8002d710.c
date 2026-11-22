/*
 * VS.EXE: clearing the 2000 hit-spark particles (WIP module), shaped as
 * ST.EXE's texanim_clear_all (src/st/texanim.c), which frees the 16
 * animated-texture slots.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/data.h>

ADDRESS(0x8002d710, 0x28)
void sparks_clear_all(void)
{
    VsRecord20 *rec = DAT_801c4a58;
    int i = 2000;

    while (i--) {
        rec->unk10 = 0;
        rec++;
    }
}
