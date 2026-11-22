/*
 * ST.EXE: a data-only module (WIP) defining the 0x800881a8-0x800881af part of the
 * first .sbss run. It links before the code (link = "first"): the -G8
 * modules that use these words (st.render, st.menu,
 * st.menu_equip_arrange, ...) read most of them with lui pairs, so none
 * of them allocated the run, while st.loop, the second object, reads
 * game_exit_request inside it $gp-relative.
 * PSYLINK allocates each object's small COMMON after the previous
 * object's, starting eight-aligned, in symbol-hash bucket order
 * (docs/executable-linking.md); the run splits at eight-byte boundaries
 * into blocks whose spellings ascend in bucket, one object each.
 * A code unit that reads a word $gp-relative keeps a tentative
 * definition, which merges here.
 * The spelling area_info_list_shown sorts below game_exit_request in that
 * order (buckets 97 and 300), so it starts an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <st/data.h>
#include <st/menu.h>
#include <st/text.h>

DATA(0x800881a8, 0x2, ".sbss")
u16 area_info_list_shown;
DATA(0x800881aa, 0x2, ".sbss")
u16 menu_second_page;
DATA(0x800881ac, 0x2, ".sbss")
u16 text_scroll_mode;
DATA(0x800881ae, 0x2, ".sbss")
u16 number_narrow;
