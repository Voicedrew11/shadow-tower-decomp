/*
 * ST.EXE visible id list builder (WIP module): vis_collect_sources fills the
 * source id list; the union of the sources' 512-bit masks becomes the
 * visible id list that vis_record_is_visible/vis_id_is_visible search.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/vis.h>

ADDRESS(0x8001c100, 0x164)
void vis_build_visible(u8 *arg)
{
    u_long mask[16];
    u_long unused[4];
    u_long *bits;
    int i;
    int j;
    int k;
    u16 id;

    vis_lists.n_sources = 0;
    vis_collect_sources((VECTOR *)arg, 0, 0);
    fill_words(mask, 0, 16);
    i = vis_lists.n_sources;
    while (i--) {
        bits = vis_source_masks[vis_lists.sources[i]].bits;
        for (j = 0; j < 16; j++) {
            mask[j] |= *bits++;
        }
    }
    vis_lists.n_visible = 0;
    for (id = j = 0; j < 16; j++) {
        for (k = 0; k < 32; k++) {
            if (mask[j] & 1) {
                if (area_state.unk0 != 1 || area_state.unk1D == 0) {
                    vis_lists.visible[vis_lists.n_visible] = id;
                    vis_lists.n_visible++;
                }
            }
            mask[j] >>= 1;
            id++;
        }
    }
}
