/*
 * ST.EXE: reading an area file into the small-file buffer, and the CD
 * completion callback that streams a read buffer of image blocks into VRAM
 * (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBCD.H>
#include <st/cd.h>
#include <st/data.h>
#include <st/area.h>

ADDRESS(0x80045f90, 0x28)
void func_80045f90(int archive, int member, void (*done)())
{
    func_80015dfc(archive, member, DAT_80088c88, done);
}

/* Takes the next run of at most 16 sectors from the request's count. */
ADDRESS(0x80045fb8, 0x44)
void func_80045fb8(StCdRequest *request)
{
    int n = request->unkE;

    if (n > 16) {
        request->unk24 = 16;
        request->unk28 = 16;
        request->unkE -= 16;
    } else {
        request->unk24 = n;
        request->unk28 = n;
        request->unkE = 0;
    }
}

/*
 * The current request's 32 KiB buffer holds image blocks: an 8-halfword
 * header (the RECT twice) and its pixels; a header of -1 ends the file.
 * Loads what fits, keeps a partial image for the next buffer, then seeks
 * the next run.
 */
ADDRESS(0x80045ffc, 0x274)
void func_80045ffc(void)
{
    StCdRequest *request = cd_state.unk9D8;
    u16 *p;
    int used;
    int left;
    int n;
    u16 h;
    s16 rows;

    if (request->unk0 != 3 || request->unk10 != 1)
        return;
    used = 0;
    p = (u16 *)request->unk20;
    request->unk10 = 0;
    for (;;) {
        left = 0x8000 - used;
        h = request->unk4.h;
        if (request->unk4.h != 0) {
            n = request->unk4.h * request->unk4.w;
            if (left < n * 2) {
                rows = left / request->unk4.w >> 1;
                if (rows != 0) {
                    request->unk4.h = rows;
                    LoadImage(&request->unk4, (u_long *)p);
                    request->unk4.h = h - rows;
                    request->unk4.y += rows;
                }
                break;
            }
            request->unk4.h = h;
            LoadImage(&request->unk4, (u_long *)p);
            used += n * 2;
            p += n;
            request->unk4.h = 0;
            continue;
        }
        if (left < 16)
            break;
        if (p[0] != p[4] || p[1] != p[5] || p[2] != p[6] || p[3] != p[7] || p[2] == 0 ||
            p[3] == 0) {
            request->unk4.h = 0;
            request->unk10 = 0;
            request->unkE = request->unkC;
            func_80045fb8(request);
            request->unk1 = 0;
            while (!CdControl(CdlSeekL, (u_char *)&request->unk18, 0))
                ;
            return;
        }
        used += 16;
        if (p[0] == 0xffff) {
            DrawSync(0);
            request->unk10 = 2;
            request->unk24 = 0;
            request->unk2C(request);
            func_80015848(request);
            return;
        }
        request->unk4.x = p[0];
        request->unk4.y = p[1];
        request->unk4.w = p[2];
        request->unk4.h = p[3];
        p += 8;
    }
    func_80015a94(&request->unk18, request->unk28, &request->unk18);
    func_80045fb8(request);
    request->unk1 = 0;
    DrawSync(0);
    while (!CdControl(CdlSeekL, (u_char *)&request->unk18, 0))
        ;
}
