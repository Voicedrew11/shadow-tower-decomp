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
 * handler. */
ADDRESS(0x8004eebc, 0x680)
void objects_load(u8 *src)
{
    AreaObject *e;
    AreaObjectType *rec;
    int now;
    int i;
    u16 n;
    u16 secs;
    int kind;
    int state;

    {
        AreaObjectTable *table = &vis_table_b;

        now = cd_state.vsync_count;
        e = table->entries;
        table->push_z = 0;
        table->push_y = 0;
        table->push_x = 0;
    }
    for (i = 239; i != -1; i--) {
        if (*(u16 *)(src + 4) == 0xffff)
            e->type = 0xffff;
        else
            e->type = *(u16 *)(src + 4);
        e->kind = 0xff;
        if (e->type != 0xffff) {
            u16 parameter = *(u16 *)(src + 14);
            u16 yaw;

            e->state = 0;
            e->examined = 0;
            e->anim = 0x80;
            e->anim_frame = 0;
            e->draw_flags = 0;
            e->rotation.vz = 0;
            e->rotation.vx = 0;
            e->block = parameter;
            yaw = *(u16 *)(src + 6);
            e->scale.vz = 0x1000;
            e->scale.vy = 0x1000;
            e->scale.vx = 0x1000;
            e->rotation.vy = yaw & 0xfff;
            rec = &vis_table_b.types[e->type];
            e->visibility = 0;
            e->flags = rec->flags;
            e->depth_bias = rec->depth_bias;
            e->unk06 = 0xff;
            e->light = 0;
            if (e->flags & 0x10) {
                e->height = (rec->height * src[16]) >> 7;
                e->scale.vx = e->scale.vy = e->scale.vz = src[16] << 5;
            } else {
                e->height = rec->height;
            }
            e->position.vx = src[0] * 4096 + *(s16 *)(src + 8);
            e->position.vy = src[1] * 4096 + *(s16 *)(src + 10);
            e->position.vz = src[2] * 4096 + *(s16 *)(src + 12);
            copy_words((u32 *)&e->unk48, (u32 *)(src + 16), 2);
            fill_words((u32 *)&e->unk50, 0xff, 2);
            kind = rec->kind;
            switch (kind) {
            case 0:
            case 1:
                n = e->params.door.n_blocks;
                while (n--) {
                    u8 t = vis_records[e->params.door.block_b + n].part;

                    vis_records[e->params.door.block_b + n].part =
                        vis_records[e->params.door.block_a + n].part;
                    vis_records[e->params.door.block_a + n].part = t;
                }
                e->kind = rec->kind;
                if (!(e->unk49 & 0x80))
                    e->anim = e->unk49;
                else
                    e->anim = 0;
                state = e->unk49;
                if (state == 0xfc || state == 0xfa || state == 0xf9)
                    e->state = 0;
                else if (state == 0xf8)
                    e->state = 11;
                else
                    e->state = 0;
                break;
            case 50:
            case 51:
                e->kind = kind;
                if (!(e->unk49 & 0x80))
                    e->anim = e->unk49;
                else
                    e->anim = 0;
                state = e->unk49;
                if (state == 0xfe)
                    e->state = 4;
                else if (state == 0xfd)
                    e->state = 5;
                else if (state == 0xfc || state == 0xfa || state == 0xf9)
                    e->state = 0;
                else if (state == 0xfb) {
                    e->visibility = 2;
                    e->state = 9;
                } else if (state == 0xf8)
                    e->state = 11;
                else
                    e->state = 0;
                *(u16 *)&e->unk50 = 0;
                break;
            case 52:
                e->kind = 52;
                e->state = 0;
                e->anim_frame = 0;
                e->anim = 0;
                break;
            case 2:
                e->kind = 2;
                e->state = 0;
                e->scale.vx = 0;
                e->scale.vy = 0;
                e->scale.vz = 0;
                e->draw_flags = 5;
                ((u8 *)&e->unk50)[0] = 0;
                e->flags |= 0x40;
                break;
            case 7:
                e->kind = 7;
                e->visibility = 2;
                break;
            case 0xe0:
                e->rotation.vy = 0;
                e->kind = 0xe0;
                e->unk01 = 0;
                if ((*(u32 *)&e->unk48 & 0xffffff) == 0xffffff) {
                    e->position.vx = src[0] * 4096 + *(s16 *)(src + 8) + 0x800;
                    e->position.vy = src[1] * 4096 + *(s16 *)(src + 10);
                    e->position.vz = src[2] * 4096 + *(s16 *)(src + 12) + 0x800;
                    ((u8 *)&e->unk50)[2] = 127;
                    ((u8 *)&e->unk50)[1] = 127;
                    ((u8 *)&e->unk50)[0] = 127;
                } else {
                    e->position.vx = src[0] * 4096 + *(s16 *)(src + 8) + 0x800;
                    e->position.vy = src[1] * 4096 + *(s16 *)(src + 10);
                    e->position.vz = src[2] * 4096 + *(s16 *)(src + 12) + 0x800;
                    ((u8 *)&e->unk50)[0] = e->unk48 - (u8)(src[0] - 64);
                    ((u8 *)&e->unk50)[1] = e->unk49 - (u8)(src[1] - 64);
                    ((u8 *)&e->unk50)[2] = e->params.snd.sound - (u8)(src[2] - 64);
                    e->params.door.lock -= *(s16 *)(src + 10) / 128;
                }
                break;
            case 0xed:
            case 0xef:
                e->kind = kind;
                ((u8 *)&e->unk50)[0] = 1;
                break;
            case 0xec:
                secs = e->params.snd.period;
                e->kind = 0xec;
                e->unk54 = -1;
                e->unk50 = now + secs * 60;
                break;
            case 31:
                secs = e->params.snd.period;
                e->unk01 = 0;
                e->unk54 = -1;
                e->kind = 31;
                e->unk50 = now + secs * 60;
                break;
            case 0xfe:
                e->kind = 0xfe;
                e->state = 0;
                break;
            case 3:
            case 5:
            case 14:
            case 0xe1:
                e->kind = kind;
                break;
            case 0xff:
                break;
            default:
                area_state.hooks->load_object(e, rec);
                break;
            }
        }
        src += 24;
        e++;
    }
}
