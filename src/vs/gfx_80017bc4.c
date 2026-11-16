/*
 * VS.EXE: clears a VRAM block (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

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
