/*
 * ST.EXE: XA voice stream start (WIP module, 0x80054000..0x800541f4; the
 * stream wait, still images and talk sequence continue in src/st/xa_play.c,
 * split off because func_8005408c is not exact and 4 bytes short).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <LIBSND.H>
#include <psyq/libc.h>
#include <st/data.h>
#include <st/cd.h>
#include <st/gfx.h>
#include <st/vis.h>
#include <st/menu.h>
#include <st/hud.h>
#include <st/text.h>
#include <st/funcs.h>
#include <st/pad.h>
#include <st/xa.h>

/* LIBC memcpy as the game declared it: retail calls it even for 4 bytes, so
 * this prototype is not GCC's builtin one. */
void *memcpy(void *dst, void *src, int size);

/* CdReadyCallback while a stream plays: keeps the current position and
 * restarts the read from the start position after a disk error. */
ADDRESS(0x80054000, 0x8c)
void func_80054000(u8 intr, u8 *result)
{
    u8 loc[8];

    while (CdControl(CdlGetlocL, NULL, loc) == 0)
        ;
    viewer.unk1DC.minute = loc[0];
    viewer.unk1DC.second = loc[1];
    viewer.unk1DC.sector = loc[2];
    switch (intr) {
    case CdlDataReady:
        break;
    case CdlDiskError:
        while (CdControlB(CdlReadS, (u8 *)&viewer.unk1D8, NULL) == 0)
            ;
        break;
    }
}

/* Starts channel `channel` of XA stream file `file`. */
ADDRESS(0x8005408c, 0x168)
void func_8005408c(u8 file, int channel)
{
    u8 param[8];
    CdlFILTER filter;
    u8 byte_channel = channel;
    u8 *save;

    func_80015ad4();
    CdReadyCallback(func_80054000);
    param[0] = CdlModeSpeed | CdlModeRT | CdlModeSF | CdlModeRept;
    CdControlB(CdlSetmode, param, NULL);
    VSync(4);
    viewer.unk1E0 = DAT_800804ac[file][byte_channel];
    /* the file's position, by byte offset from the start of the save data */
    save = (u8 *)&DAT_801cba08;
    memcpy((CdlLOC *)&viewer.unk1DC, save + file * 4 + offsetof(Data801cba08, unk1618), 4);
    memcpy(&viewer.unk1D8, (CdlLOC *)&viewer.unk1DC, 4);
    while (CdControl(CdlSetloc, (u8 *)&DAT_801cba08.unk1618[file], NULL) == 0)
        ;
    filter.file = 1;
    filter.chan = byte_channel;
    while (CdControl(CdlSetfilter, (u8 *)&filter, NULL) == 0)
        ;
    viewer.unk1E8 = cd_state.unk4;
    while (CdControlB(CdlReadS, (u8 *)&DAT_801cba08.unk1618[file], NULL) == 0)
        ;
}
