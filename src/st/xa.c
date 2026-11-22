/*
 * ST.EXE: XA voice stream start (WIP module, 0x80054000..0x800541f4; the
 * stream wait, still images and talk sequence continue in src/st/xa_talk.c,
 * split off because xa_start_stream is not exact and 4 bytes short).
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
void xa_ready_callback(u8 intr, u8 *result)
{
    u8 loc[8];

    while (CdControl(CdlGetlocL, NULL, loc) == 0)
        ;
    viewer.xa_pos.minute = loc[0];
    viewer.xa_pos.second = loc[1];
    viewer.xa_pos.sector = loc[2];
    switch (intr) {
    case CdlDataReady:
        break;
    case CdlDiskError:
        while (CdControlB(CdlReadS, (u8 *)&viewer.xa_start, NULL) == 0)
            ;
        break;
    }
}

/* Starts channel `channel` of XA stream file `file`. */
ADDRESS(0x8005408c, 0x168)
void xa_start_stream(u8 file, int channel)
{
    u8 param[8];
    CdlFILTER filter;
    u8 byte_channel = channel;
    u8 *save;

    cd_wait_idle();
    CdReadyCallback(xa_ready_callback);
    param[0] = CdlModeSpeed | CdlModeRT | CdlModeSF | CdlModeRept;
    CdControlB(CdlSetmode, param, NULL);
    VSync(4);
    viewer.xa_length = xa_stream_lengths[file][byte_channel];
    /* the file's position, by byte offset from the start of the save data */
    save = (u8 *)&game_progress;
    memcpy((CdlLOC *)&viewer.xa_pos, save + file * 4 + offsetof(GameProgress, xa_positions), 4);
    memcpy(&viewer.xa_start, (CdlLOC *)&viewer.xa_pos, 4);
    while (CdControl(CdlSetloc, (u8 *)&game_progress.xa_positions[file], NULL) == 0)
        ;
    filter.file = 1;
    filter.chan = byte_channel;
    while (CdControl(CdlSetfilter, (u8 *)&filter, NULL) == 0)
        ;
    viewer.xa_start_tick = cd_state.unk4;
    while (CdControlB(CdlReadS, (u8 *)&game_progress.xa_positions[file], NULL) == 0)
        ;
}
