/*
 * ST.EXE: CD position offset and the request-ring waits (WIP module). Split
 * from src/st/cdreq.c while func_800159d4 there was not exact.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <st/data.h>
#include <st/cd.h>
#include <st/funcs.h>

ADDRESS(0x80015a94, 0x40)
void func_80015a94(CdlLOC *pos, int offset, CdlLOC *out)
{
    func_800159d4(out, func_80015950(pos) + offset);
}

/* Waits until the request ring is empty. */
ADDRESS(0x80015ad4, 0x58)
void func_80015ad4(void)
{
    while (cd_state.unk9D8 != cd_state.unk9D4 || cd_state.unk9D8->unk0 != 0)
        func_8001531c();
}

ADDRESS(0x80015b2c, 0x4c)
void func_80015b2c(StCdRequest *request)
{
    while (request->unk0)
        func_8001531c();
}
