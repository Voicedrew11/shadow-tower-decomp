/*
 * ST.EXE: clears the item counts (WIP module, 0x80055990..0x800559bc; split
 * from src/st/save.c because this function is not exact and 4 bytes short).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBETC.H>
#include <psyq/libc.h>
#include <st/memory.h>
#include <st/data.h>
#include <st/gfx.h>
#include <st/light.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/save.h>


/*
 * Not exact (4 bytes short): retail strength-reduces the byte address into
 * a pointer counted down from counts + 313; this source indexes it.
 */
ADDRESS(0x80055990, 0x2c)
void func_80055990(void)
{
    int i;

    for (i = 313; i >= 0; i--)
        *(u8 *)&DAT_801c7590.counts[i] = 0;
    viewer.unk27 = 0;
}

