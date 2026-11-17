/*
 * FDAT_272.BIN: its first handler and the scene words after its code (WIP
 * module; a unit of its own because func_801ed4d0 is not yet exact). The
 * story flags' bits 8-9 and 10-12 are two small fields here (the area's
 * scene stage and the gate's stage).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/vis.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/fdat.h>

DATA(0x801ef504, 0x18, ".bss")
int fdat_801ef504[6];

RODATA(0x801eb72c, 0x8)

void func_801ef1c0(void);

/* Not exact (one attempt, retries off; 93.8%): first divergence retail `addiu a2,v0,8408`, this source `addiu a3,v0,8408`. */
/*
 * Not exact (registers, one instruction long): retail keeps table B entry
 * 5's pointer in a2 and the flags' base in a1 (this source: a3, a2) and
 * reloads the flags word once less.
 *
 * Runs the gate stages (story flag bits 10-12) by table B entry 5's state;
 * sets flag 13 once the six entries of fdat_801ef504 are all beaten, then
 * counts down to func_801ef1c0's scene; moves the palette row 487 to 489
 * when fdat_801ef934 asks.
 */
ADDRESS(0x801ed4d0, 0x290)
void func_801ed4d0(void)
{
    Vis88 *d5 = &vis_table_b.entries[5];
    Vis212 *e32 = &vis_table_a.entries[32];
    int i;

    if ((DAT_801cba08.unk1440.word >> 10 & 7) == 0 && d5->unk10 != 0) {
        DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x400;
        fdat_801ed410(e32);
        e32->unk36 = -1;
        e32->unk03 = 1;
        e32->unk1C.bytes[2] &= 0xf2;
    } else if ((DAT_801cba08.unk1440.word >> 10 & 7) == 1 && d5->unk10 > 0x800) {
        DAT_801cba08.unk1440.word = (DAT_801cba08.unk1440.word & ~0x1c00) | 0x800;
    }
    if (fdat_801ef938 == 0 && area_state.unk0 == 0)
        fdat_801ef938 = 1;
    if (!(DAT_801cba08.unk1440.word >> 13 & 1) && fdat_801ef938 == 1) {
        DAT_801cba08.unk1440.word |= 0x2000;
        for (i = 0; i < 6; i++) {
            u8 state = vis_table_a.entries[fdat_801ef504[i]].unk05;

            if (state != 3 && state != 5) {
                DAT_801cba08.unk1440.word &= ~0x2000;
                break;
            }
        }
    }
    if (DAT_801cba08.unk1440.bit.f13 == 1 && fdat_801ef930 >= 0) {
        if (fdat_801ef930 == 0) {
            func_801ef1c0();
            fdat_801ef938 = 0xff;
        } else {
            fdat_801ef930--;
        }
    }
    if (fdat_801ef934 == 1 && area_state.unk0 == 0) {
        RECT rect = {0, 487, 256, 1};

        MoveImage(&rect, 0, 489);
        fdat_801ef930 = -1;
        fdat_801ef934 = 0;
        vis_lists.unk47E0 |= 0x10;
    }
}
