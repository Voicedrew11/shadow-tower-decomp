/*
 * ST.EXE: the game loop, which returns to PSX.EXE with the next program
 * (WIP module). Its unit is -G8: the loop reads its exit word $gp-relative
 * and copies the clear RECT from an .sdata literal. It is split from
 * src/st/main.c only because game_load_start_file there is not exact.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBSND.H>
#include <LIBPAD.H>
#include <st/memory.h>
#include <st/data.h>
#include <st/main.h>
#include <st/gfx.h>
#include <st/funcs.h>
#include <st/cd.h>
#include <st/hud.h>
#include <st/light.h>
#include <st/effect.h>

/* PSX.EXE program table indices. */
#define PROGRAM_TITLE 1
#define PROGRAM_END 4

/* Mailbox value of TITLE.EXE's Continue. */
#define MAILBOX_CONTINUE 2

/* game_exit_request values that end the game loop. */
#define EXIT_TITLE 2
#define EXIT_ENDING 3

SDATA(0x80087ec4, 0x8)

/*
 * Read by the game loop; other modules set it to end the game. A tentative
 * definition, so that ASPSX reaches it through $gp: the word lies inside
 * the .sbss run st.sbss_glyph_game_exit allocates, and the request merges
 * there.
 */
s32 game_exit_request;

ADDRESS(0x80014a30, 0x2cc)
void game_loop(void)
{
    u8 program;

    cd_init();
    gfx_init();
    loading_screen_find_file();
    loading_screen_show();
    sound_init();
    objects_clear();
    creatures_clear();
    ground_items_clear();
    effects_clear();
    game_load_common_data();
    save_reset_new_game();
    lighting_load_defaults();
    fade_reset();
    lighting_build_matrices();
    SsSetMVol(127, 127);
    if (*MAILBOX_BYTE_1 == MAILBOX_CONTINUE) {
        RECT rect = {0, 0, 320, 480};

        ClearImage(&rect, 0, 0, 0);
        card_screen_open(0, 0);
        pad_enable();
        if (card_screen_run(0) == 1) {
            pad_clear_input();
            pad_state.disabled = 1;
            card_screen_close();
            loading_screen_show();
            player_reload_equipment();
        } else {
            pad_clear_input();
            pad_state.disabled = 1;
            card_screen_close();
            loading_screen_show();
            game_load_start_file();
        }
    } else {
        game_load_start_file();
    }
    lighting_start();
    area_load_current_blocking();
    player_reset_motion();
    heap_init(model_heap, 0x56400);
    loading_screen_end();
    pad_enable();
    pad_clear_input();
    game_exit_request = 0;
    for (;;) {
        VECTOR a;
        SVECTOR b;

        frame_rate_update();
        fade_reset();
        player_update();
        objects_update();
        creatures_update();
        ground_items_update();
        effects_run();
        viewer.update_all_spawns = 0;
        area_run_frame_hook();
        area_load_update();
        player_get_camera(&a, &b);
        sound_set_listener(&a, &b);
        image_stream_update();
        sound_bank_transfer_body();
        world_render_frame(&a, &b);
        if (game_exit_request == 0)
            continue;
        VSync(10);
        fade_run_frames(2, 0, 4096, 128);
        VSync(10);
        switch (game_exit_request) {
        case EXIT_TITLE:
            game_shutdown();
            program = PROGRAM_TITLE;
            break;
        case EXIT_ENDING:
            menu_save_prompt_run();
            game_shutdown();
            program = PROGRAM_END;
            break;
        default:
            continue;
        }
        break;
    }
    *NEXT_PROGRAM_BYTE = program;
    *MAILBOX_BYTE_1 = 0;
    *MAILBOX_BYTE_2 = 0;
}
