/*
 * ST.EXE: setting up table B (vis_table_b.entries) from an area file's
 * 240 24-byte entries (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/cd.h>
#include <st/data.h>
#include <st/memory.h>
#include <st/vis.h>

/* Each entry takes its record, position and parameters from the file; the
 * record's kind (byte 0) sets its state, unknown kinds go to the area's
 * handler.
 * Not exact: relative byte-parameter subtraction order and the sound-period
 * temporary still differ.
 */
ADDRESS(0x8004eebc, 0x680)
void func_8004eebc(u8 *src)
{
    Vis88 *e;
    Vis24 *rec;
    int now;
    int i;
    u16 n;
    int kind;
    int state;

    {
        VisTableB *table = &vis_table_b;

        now = cd_state.unk4;
        e = table->entries;
        table->unk71A8 = 0;
        table->unk71A6 = 0;
        table->unk71A4 = 0;
    }
    for (i = 239; i != -1; i--) {
        if (*(u16 *)(src + 4) == 0xffff)
            e->unk0a = 0xffff;
        else
            e->unk0a = *(u16 *)(src + 4);
        e->unk05 = 0xff;
        if (e->unk0a != 0xffff) {
            u16 parameter = *(u16 *)(src + 14);
            u16 yaw;

            e->unk0C = 0;
            e->unk0E = 0;
            e->unk02 = 0x80;
            e->unk10 = 0;
            e->unk03 = 0;
            e->unk38 = 0;
            e->unk34 = 0;
            e->unk08 = parameter;
            yaw = *(u16 *)(src + 6);
            e->unk3C.vz = 0x1000;
            e->unk3C.vy = 0x1000;
            e->unk3C.vx = 0x1000;
            e->unk36 = yaw & 0xfff;
            rec = &vis_table_b.unk0000[e->unk0a];
            e->unk00 = 0;
            e->unk04 = rec->unk03;
            e->unk14 = rec->unk0A;
            e->unk06 = 0xff;
            e->unk16 = 0;
            if (e->unk04 & 0x10) {
                e->unk12 = (rec->unk08 * src[16]) >> 7;
                e->unk3C.vx = e->unk3C.vy = e->unk3C.vz = src[16] << 5;
            } else {
                e->unk12 = rec->unk08;
            }
            e->unk24.vx = src[0] * 4096 + *(s16 *)(src + 8);
            e->unk24.vy = src[1] * 4096 + *(s16 *)(src + 10);
            e->unk24.vz = src[2] * 4096 + *(s16 *)(src + 12);
            func_80014cfc((u32 *)&e->unk48, (u32 *)(src + 16), 2);
            fill_words((u32 *)&e->unk50, 0xff, 2);
            kind = rec->unk00[0];
            switch (kind) {
            case 0:
            case 1:
                n = e->unk4A.door.unk4E;
                while (n--) {
                    u8 t = vis_records[e->unk4A.door.unk4C + n].part;

                    vis_records[e->unk4A.door.unk4C + n].part =
                        vis_records[e->unk4A.door.unk4A + n].part;
                    vis_records[e->unk4A.door.unk4A + n].part = t;
                }
                e->unk05 = rec->unk00[0];
                if (!(e->unk49 & 0x80))
                    e->unk02 = e->unk49;
                else
                    e->unk02 = 0;
                state = e->unk49;
                if (state == 0xfc || state == 0xfa || state == 0xf9)
                    e->unk0C = 0;
                else if (state == 0xf8)
                    e->unk0C = 11;
                else
                    e->unk0C = 0;
                break;
            case 50:
            case 51:
                e->unk05 = kind;
                if (!(e->unk49 & 0x80))
                    e->unk02 = e->unk49;
                else
                    e->unk02 = 0;
                state = e->unk49;
                if (state == 0xfe)
                    e->unk0C = 4;
                else if (state == 0xfd)
                    e->unk0C = 5;
                else if (state == 0xfc || state == 0xfa || state == 0xf9)
                    e->unk0C = 0;
                else if (state == 0xfb) {
                    e->unk00 = 2;
                    e->unk0C = 9;
                } else if (state == 0xf8)
                    e->unk0C = 11;
                else
                    e->unk0C = 0;
                *(u16 *)&e->unk50 = 0;
                break;
            case 52:
                e->unk05 = 52;
                e->unk0C = 0;
                e->unk10 = 0;
                e->unk02 = 0;
                break;
            case 2:
                e->unk05 = 2;
                e->unk0C = 0;
                e->unk3C.vx = 0;
                e->unk3C.vy = 0;
                e->unk3C.vz = 0;
                e->unk03 = 5;
                ((u8 *)&e->unk50)[0] = 0;
                e->unk04 |= 0x40;
                break;
            case 7:
                e->unk05 = 7;
                e->unk00 = 2;
                break;
            case 0xe0:
                e->unk36 = 0;
                e->unk05 = 0xe0;
                e->unk01 = 0;
                if ((*(u32 *)&e->unk48 & 0xffffff) == 0xffffff) {
                    e->unk24.vx = src[0] * 4096 + *(s16 *)(src + 8) + 0x800;
                    e->unk24.vy = src[1] * 4096 + *(s16 *)(src + 10);
                    e->unk24.vz = src[2] * 4096 + *(s16 *)(src + 12) + 0x800;
                    ((u8 *)&e->unk50)[2] = 127;
                    ((u8 *)&e->unk50)[1] = 127;
                    ((u8 *)&e->unk50)[0] = 127;
                } else {
                    e->unk24.vx = src[0] * 4096 + *(s16 *)(src + 8) + 0x800;
                    e->unk24.vy = src[1] * 4096 + *(s16 *)(src + 10);
                    e->unk24.vz = src[2] * 4096 + *(s16 *)(src + 12) + 0x800;
                    ((u8 *)&e->unk50)[0] = e->unk48 - (src[0] - 64);
                    ((u8 *)&e->unk50)[1] = e->unk49 - (src[1] - 64);
                    ((u8 *)&e->unk50)[2] = e->unk4A.snd.unk4A - (src[2] - 64);
                    e->unk4A.door.unk4F -= *(s16 *)(src + 10) / 128;
                }
                break;
            case 0xed:
            case 0xef:
                e->unk05 = kind;
                ((u8 *)&e->unk50)[0] = 1;
                break;
            case 0xec:
                n = e->unk4A.snd.unk4E;
                e->unk05 = 0xec;
                e->unk54 = -1;
                e->unk50 = now + n * 60;
                break;
            case 31:
                n = e->unk4A.snd.unk4E;
                e->unk01 = 0;
                e->unk54 = -1;
                e->unk05 = 31;
                e->unk50 = now + n * 60;
                break;
            case 0xfe:
                e->unk05 = 0xfe;
                e->unk0C = 0;
                break;
            case 3:
            case 5:
            case 14:
            case 0xe1:
                e->unk05 = kind;
                break;
            case 0xff:
                break;
            default:
                area_state.unkC->unk20(e, rec);
                break;
            }
        }
        src += 24;
        e++;
    }
}
