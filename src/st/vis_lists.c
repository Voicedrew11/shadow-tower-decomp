/*
 * ST.EXE visible-set queries and list builders (WIP module): membership
 * tests against the visible id list, and three passes that collect the
 * entries of three object tables whose id is visible.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/vis.h>

ADDRESS(0x8001c800, 0x8c)
int vis_record_is_visible(int a, int b, int c)
{
    VisRecord *rec;
    int i;

    i = vis_lists.n_visible;
    while (i--) {
        rec = &vis_records[vis_lists.visible[i]];
        if (rec->unk08 == a && rec->unk09 == b && rec->unk0a == c) {
            return 1;
        }
    }
    return 0;
}

ADDRESS(0x8001c88c, 0x54)
int vis_id_is_visible(u16 id)
{
    int i;

    i = vis_lists.n_visible;
    while (i--) {
        if (id == vis_lists.visible[i]) {
            return 1;
        }
    }
    return 0;
}

ADDRESS(0x8001c8e0, 0xe8)
void vis_collect_table_a(void)
{
    Vis212 *e;
    int i;

    vis_table_a.count = 0;
    e = vis_table_a.entries;
    i = 64;
    while (i--) {
        if (e->unk03 != 2 && (e->unk05 == 1 || e->unk05 == 5)
            && (e->unk03 == 1 || vis_id_is_visible(e->unk26))) {
            if (vis_table_a.count >= 12) {
                return;
            }
            vis_table_a.list[vis_table_a.count] = 63 - i;
            vis_table_a.count++;
        } else {
            e->unk04 = 0;
        }
        e++;
    }
}

ADDRESS(0x8001c9c8, 0xf0)
void vis_collect_table_b(void)
{
    Vis88 *e;
    int i;

    e = vis_table_b.entries;
    vis_table_b.count = 0;
    i = 240;
    while (i--) {
        if (e->unk0a < 280) {
            e->unk04 &= 0x7f;
            if (vis_id_is_visible(e->unk08) || e->unk05 == 236) {
                if (vis_table_b.count >= 96) {
                    return;
                }
                e->unk04 |= 0x80;
                vis_table_b.list[vis_table_b.count] = 239 - i;
                vis_table_b.count++;
            }
        } else {
            e->unk07 = 0;
        }
        e++;
    }
}

ADDRESS(0x8001cab8, 0xd0)
void vis_collect_table_c(void)
{
    Vis48 *e;
    int i;

    e = vis_table_c.entries;
    vis_table_c.count = 0;
    i = 64;
    while (i--) {
        if (e->unk00 != 0xffff && e->unk2c == area_state.unk09 && vis_id_is_visible(e->unk02)) {
            vis_table_c.list[vis_table_c.count] = 63 - i;
            vis_table_c.count++;
        }
        e++;
    }
}
