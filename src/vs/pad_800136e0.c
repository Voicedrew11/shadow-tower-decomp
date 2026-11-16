/*
 * VS.EXE: the controller decode, its reset and two frame counters (WIP
 * module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <vs/pad.h>

/* Decodes both pad buffers: held buttons, newly pressed ones, and a
 * repeat after 30 frames, then every 5. */
ADDRESS(0x800136e0, 0x13c)
void func_800136e0(void)
{
    if (DAT_801961e0[0].status == 0) {
        DAT_801603c0.pads[0].unk0 = ~DAT_801961e0[0].buttons;
        if (DAT_801603c0.pads[0].unk0 != DAT_801603c0.pads[0].unk4) {
            DAT_801603c0.pads[0].unk10 = 30;
            DAT_801603c0.pads[0].unk8 |= DAT_801603c0.pads[0].unk0 & ~DAT_801603c0.pads[0].unk4;
            DAT_801603c0.pads[0].unkC |= DAT_801603c0.pads[0].unk0 & ~DAT_801603c0.pads[0].unk4;
        } else if (DAT_801603c0.pads[0].unk10 != 0) {
            if (--DAT_801603c0.pads[0].unk10 == 0) {
                DAT_801603c0.pads[0].unk10 = 5;
                DAT_801603c0.pads[0].unkC |= DAT_801603c0.pads[0].unk0;
            }
        }
    }
    if (DAT_801961e0[1].status == 0) {
        DAT_801603c0.pads[1].unk0 = ~DAT_801961e0[1].buttons;
        if (DAT_801603c0.pads[1].unk0 != DAT_801603c0.pads[1].unk4) {
            DAT_801603c0.pads[1].unk10 = 30;
            DAT_801603c0.pads[1].unk8 |= DAT_801603c0.pads[1].unk0 & ~DAT_801603c0.pads[1].unk4;
            DAT_801603c0.pads[1].unkC |= DAT_801603c0.pads[1].unk0 & ~DAT_801603c0.pads[1].unk4;
        } else if (DAT_801603c0.pads[1].unk10 != 0) {
            if (--DAT_801603c0.pads[1].unk10 == 0) {
                DAT_801603c0.pads[1].unk10 = 5;
                DAT_801603c0.pads[1].unkC |= DAT_801603c0.pads[1].unk0;
            }
        }
    }
    DAT_801603c0.pads[0].unk4 = DAT_801603c0.pads[0].unk0;
    DAT_801603c0.pads[1].unk4 = DAT_801603c0.pads[1].unk0;
}

/* Clears both controller records. */
ADDRESS(0x8001381c, 0x2c)
void func_8001381c(void)
{
    DAT_801603c0.pads[1].unkC = 0;
    DAT_801603c0.pads[0].unkC = 0;
    DAT_801603c0.pads[1].unk8 = 0;
    DAT_801603c0.pads[0].unk8 = 0;
    DAT_801603c0.pads[1].unk4 = 0;
    DAT_801603c0.pads[0].unk4 = 0;
    DAT_801603c0.pads[1].unk0 = 0;
    DAT_801603c0.pads[0].unk0 = 0;
}

/* A frame callback: counts. */
ADDRESS(0x80013848, 0x20)
void func_80013848(void)
{
    DAT_801603c0.unk30++;
}

/* A frame callback: counts and decodes the controllers. */
ADDRESS(0x80013868, 0x38)
void func_80013868(void)
{
    DAT_801603c0.unk3C++;
    func_800136e0();
}
