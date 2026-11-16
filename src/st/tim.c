/*
 * ST.EXE: expanding the 16-byte placement records into the table at
 * DAT_8015c488.objects (0x8015c48c), and uploading a run of TIM images to VRAM (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <st/gfx.h>
#include <st/tim.h>

/*
 * Copies 128 placement records into the table: the id, the flag bytes,
 * each coordinate as block * 4096 + offset and a random start below
 * unk3. An id of 0xffff marks an empty record.
 */
ADDRESS(0x80031de4, 0x118)
void placements_expand(StPlacement *src)
{
    StObject24 *dst = DAT_8015c488.objects;
    int i = 128;
    u16 id;

    while (i--) {
        id = src->id;
        if (id != 0xffff) {
            dst->unk0 = id;
            dst->unk2 = src->unk4 | (src->unk8 << 6);
            dst->unk3 = src->unk2;
            dst->unk4 = src->unk3;
            dst->unk8.vx = (src->bx << 12) + src->x;
            dst->unk8.vy = (src->by << 12) + src->y;
            dst->unk8.vz = (src->bz << 12) + src->z;
            dst->unk5 = (rand() * dst->unk3) >> 15;
        } else {
            dst->unk0 = 0xffff;
        }
        dst++;
        src++;
    }
}

/*
 * Uploads the TIM images packed in `buf` (each its 8-byte header, an
 * optional CLUT block and the pixel block) and returns the bytes of the
 * complete images; it stops at the first truncated or inconsistent one.
 *
 * Not exact (96.9%, size-exact, same control flow):
 * retail keeps start/p/block/data in s3/s1/s2/s0 and reads each block
 * header through `block`; this build allocates s2/s0/s3/s1 and reads it
 * through `p`.
 */
ADDRESS(0x80031efc, 0x15c)
int tim_load_images(u_long *buf, u_long size)
{
    u_char *start = (u_char *)buf;
    u_char *p = start;
    u_char *done = start;

    for (;;) {
        TimHeader *tim;
        TimBlock *block;
        u_char *data;
        int bytes;

        if (size < p - start + sizeof(TimHeader))
            break;
        tim = (TimHeader *)p;
        p += sizeof(TimHeader);
        if (tim->flag & 8) {
            block = (TimBlock *)p;
            if (size < p - start + sizeof(TimBlock))
                break;
            bytes = block->w * block->h * 2;
            if (block->bnum != bytes + sizeof(TimBlock))
                break;
            p = (u_char *)(block + 1);
            if (size < p - start + bytes)
                break;
            data = p;
            p += bytes;
            VSync(0);
            LoadImage((RECT *)&block->x, (u_long *)data);
        }
        block = (TimBlock *)p;
        if (size < p - start + sizeof(TimBlock))
            break;
        bytes = block->w * block->h * 2;
        if (block->bnum != bytes + sizeof(TimBlock))
            break;
        p = (u_char *)(block + 1);
        if (size < p - start + bytes)
            break;
        data = p;
        p += bytes;
        VSync(0);
        LoadImage((RECT *)&block->x, (u_long *)data);
        done = p;
    }
    return done - start;
}
