/*
 * VS.EXE: stopping the drive, opening an archive, resetting the CD request
 * ring and a random particle offset (WIP module). The first two are VS's
 * copies of ST.EXE's cd_stop_drive and cd_archive_open (src/st/cd.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <LIBETC.H>
#include <LIBGTE.H>
#include <LIBCD.H>
#include <psyq/libc.h>
#include <st/cd.h>
#include <vs/data.h>
#include <vs/funcs.h>

/* cd_archive_open's path parts. */
SDATA(0x800632e8, 0xb)

ADDRESS(0x8002efcc, 0x48)
void cd_stop_drive(void)
{
    u_char param[16];

    CdControl(CdlStop, 0, 0);
    param[0] = CdlModeSpeed;
    CdControlB(CdlSetmode, param, 0);
    VSync(4);
}

/*
 * Opens archive `archive` from the \ST\ directory: records its extent and
 * loads its start-sector table.
 */
ADDRESS(0x8002f014, 0xe0)
void cd_archive_open(u16 archive, char *name)
{
    struct {
        u16 count;
        u16 sectors[1055];
    } header;
    CdlFILE file;
    char path[64];
    VsArchive *a = &vs_cd_state.archives[archive];
    u16 *sectors;
    int count;
    u16 *source;

    strcpy(path, "\\ST\\");
    strcat(path, name);
    strcat(path, ";1");
    while (!CdSearchFile(&file, path))
        ;
    a->file.pos = file.pos;
    a->file.size = file.size;
    cd_read_file_extent((u_long *)&header, &a->file, 2048);
    sectors = mem_alloc((header.count + 1) * 2);
    count = header.count + 1;
    source = header.sectors;
    a->sectors = sectors;
    copy_halfwords(sectors, source, count);
}

/* Resets the CD request ring and callbacks, then stops the drive. */
ADDRESS(0x8002f0f4, 0x80)
void cd_init(void)
{
    StCdRequest *r;
    int i;

    EnterCriticalSection();
    r = vs_cd_state.requests;
    i = 0;
    do {
        i++;
        r->kind = CD_REQUEST_NONE;
        r->step = 0;
        r++;
    } while (i < VS_CD_REQUESTS);
    vs_cd_state.next_free = vs_cd_state.requests;
    vs_cd_state.current = vs_cd_state.requests;
    CdSyncCallback(0);
    CdReadyCallback(0);
    CdReadCallback(0);
    ExitCriticalSection();
    cd_stop_drive();
}

/* A random offset (x and z within +-8, y below `height`) and `pos` minus it
 * (y plus `dy`). */
ADDRESS(0x8002f174, 0xf0)
void effect_random_offset(SVECTOR *offset, VECTOR *out, VECTOR *pos, short dy, short height)
{
    offset->vx = 8 - rand() / 2048;
    offset->vy = height - rand() / 1024;
    offset->vz = 8 - rand() / 2048;
    out->vx = pos->vx - offset->vx;
    out->vy = pos->vy + dy;
    out->vz = pos->vz - offset->vz;
}
