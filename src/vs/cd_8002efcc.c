/*
 * VS.EXE: stopping the drive, opening an archive, resetting the CD request
 * ring and a random particle offset (WIP module). The first two are VS's
 * copies of ST.EXE's func_8001624c and func_80016294 (src/st/cd.c).
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

/* func_8002f014's path parts. */
SDATA(0x800632e8, 0xb)

ADDRESS(0x8002efcc, 0x48)
void func_8002efcc(void)
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
 *
 * Not exact (size-exact): retail stores a->sectors after loading
 * func_8002e660's count and source arguments; this build stores it first
 * (the residue ST.EXE's func_80016294 shows too).
 */
ADDRESS(0x8002f014, 0xe0)
void func_8002f014(u16 archive, char *name)
{
    struct {
        u16 count;
        u16 sectors[1055];
    } header;
    CdlFILE file;
    char path[64];
    VsArchive *a = &DAT_801a4590.archives[archive];

    strcpy(path, "\\ST\\");
    strcat(path, name);
    strcat(path, ";1");
    while (!CdSearchFile(&file, path))
        ;
    a->file.pos = file.pos;
    a->file.size = file.size;
    func_8002ee54((u_long *)&header, &a->file, 2048);
    func_8002e660(a->sectors = func_8002e6d4((header.count + 1) * 2), header.sectors,
                  header.count + 1);
}

/* Resets the CD request ring and callbacks, then stops the drive. */
ADDRESS(0x8002f0f4, 0x80)
void func_8002f0f4(void)
{
    StCdRequest *r;
    int i;

    EnterCriticalSection();
    r = DAT_801a4590.unk8;
    i = 0;
    do {
        i++;
        r->unk0 = 0;
        r->unk1 = 0;
        r++;
    } while (i < VS_CD_REQUESTS);
    DAT_801a4590.unkD8 = DAT_801a4590.unk8;
    DAT_801a4590.unkDC = DAT_801a4590.unk8;
    CdSyncCallback(0);
    CdReadyCallback(0);
    CdReadCallback(0);
    ExitCriticalSection();
    func_8002efcc();
}

/* A random offset (x and z within +-8, y below `height`) and `pos` minus it
 * (y plus `dy`). */
ADDRESS(0x8002f174, 0xf0)
void func_8002f174(SVECTOR *offset, VECTOR *out, VECTOR *pos, short dy, short height)
{
    offset->vx = 8 - rand() / 2048;
    offset->vy = height - rand() / 1024;
    offset->vz = 8 - rand() / 2048;
    out->vx = pos->vx - offset->vx;
    out->vy = pos->vy + dy;
    out->vz = pos->vz - offset->vz;
}
