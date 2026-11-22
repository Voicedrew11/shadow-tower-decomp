/*
 * ST.EXE: a data-only module (WIP) defining game .data: the default area
 * handlers, the identity matrix, the four quarter-turn yaw matrices and the XA
 * stream lengths.
 * No code unit but st.main contributes .data, and retail keeps these after
 * st.main's run, so a module linked after the code defines them; every user
 * declares them extern.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>
#include <st/area.h>
#include <st/anim.h>
#include <st/vis.h>
#include <st/xa.h>

/* The default area handlers: every slot is the empty handler. */
DATA(0x8008038c, 0x80, ".data")
AreaHooks area_hooks_default = {
    area_hook_none,
    area_hook_none,
    area_hook_none,
    area_hook_none,
    area_hook_none,
    area_hook_none,
    (void (*)())area_hook_none,
    area_hook_none,
    (void (*)())area_hook_none,
    area_hook_none,
    area_hook_none,
    area_hook_none,
    area_hook_none,
    area_hook_none,
    area_hook_none,
    area_hook_none,
    area_hook_none,
    area_hook_none,
    {area_hook_none, area_hook_none, area_hook_none, area_hook_none,
     area_hook_none, area_hook_none, area_hook_none, area_hook_none,
     area_hook_none, area_hook_none, area_hook_none, area_hook_none,
     area_hook_none, area_hook_none},
};

DATA(0x8008040c, 0x20, ".data")
MATRIX identity_matrix = {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}, {0, 0, 0}};

DATA(0x8008042c, 0x80, ".data")
MATRIX quarter_turn_matrices[4] = {
    {{{4096, 0, 0}, {0, 4096, 0}, {0, 0, 4096}}, {0, 0, 0}},
    {{{0, 0, -4096}, {0, 4096, 0}, {4096, 0, 0}}, {0, 0, 0}},
    {{{-4096, 0, 0}, {0, 4096, 0}, {0, 0, -4096}}, {0, 0, 0}},
    {{{0, 0, 4096}, {0, 4096, 0}, {-4096, 0, 0}}, {0, 0, 0}},
};

/*
 * The XA stream lengths in units of two sectors (xa_stream_finished
 * compares the sectors played with twice the length), per stream file and
 * channel: 48 files, the file count of the save block's per-file array (game_progress.talk_counts); game_load_common_data locates files 0..39, the last eight rows are empty.
 */
DATA(0x800804ac, 0x480, ".data")
u32 xa_stream_lengths[48][6] = {
    {930, 861, 696, 842, 523, 0},
    {456, 273, 487, 495, 368, 0},
    {491, 357, 412, 377, 0, 0},
    {300, 282, 397, 432, 0, 0},
    {353, 294, 372, 226, 0, 0},
    {603, 831, 691, 0, 0, 0},
    {443, 0, 0, 0, 0, 0},
    {655, 68, 923, 0, 0, 0},
    {404, 248, 225, 218, 0, 0},
    {478, 0, 0, 0, 0, 0},
    {396, 286, 387, 245, 0, 0},
    {1026, 811, 0, 0, 0, 0},
    {697, 148, 0, 0, 0, 0},
    {504, 716, 522, 0, 0, 0},
    {1073, 337, 359, 0, 0, 0},
    {684, 443, 537, 0, 0, 0},
    {647, 585, 0, 0, 0, 0},
    {233, 0, 0, 0, 0, 0},
    {1513, 505, 648, 567, 0, 0},
    {223, 345, 195, 0, 0, 0},
    {521, 450, 149, 284, 0, 0},
    {705, 362, 213, 0, 0, 0},
    {1166, 917, 643, 0, 0, 0},
    {240, 1112, 724, 397, 0, 0},
    {871, 404, 0, 0, 0, 0},
    {402, 1037, 634, 0, 0, 0},
    {584, 0, 0, 0, 0, 0},
    {284, 710, 263, 0, 0, 0},
    {863, 496, 0, 0, 0, 0},
    {368, 243, 0, 0, 0, 0},
    {598, 534, 320, 0, 0, 0},
    {390, 978, 0, 0, 0, 0},
    {1247, 0, 0, 0, 0, 0},
    {396, 0, 0, 0, 0, 0},
    {705, 265, 213, 0, 0, 0},
    {436, 0, 0, 0, 0, 0},
    {826, 0, 0, 0, 0, 0},
    {868, 0, 0, 0, 0, 0},
    {146, 491, 0, 0, 0, 0},
    {156, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
};
