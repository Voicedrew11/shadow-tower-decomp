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

/* DAT_800881a4 values that end the game loop. */
#define EXIT_TITLE 2
#define EXIT_ENDING 3

SDATA(0x80087ec4, 0x8)

/*
 * Read by the game loop; other modules set it to end the game. A tentative
 * definition, so that ASPSX reaches it through $gp: the word lies inside
 * the .sbss run st.sbss_800881a0 allocates, and the request merges there.
 */
s32 DAT_800881a4;

ADDRESS(0x80014a30, 0x2cc)
void game_loop(void)
{
    u8 program;

    cd_init();
    gfx_init();
    loading_screen_find_file();
    loading_screen_show();
    func_80057da0();
    func_8004eda8();
    func_8004afdc();
    func_80052ca4();
    effects_clear();
    func_80044674();
    func_800559bc();
    lighting_load_defaults();
    fade_reset();
    lighting_build_matrices();
    SsSetMVol(127, 127);
    if (*MAILBOX_BYTE_1 == MAILBOX_CONTINUE) {
        RECT rect = {0, 0, 320, 480};

        ClearImage(&rect, 0, 0, 0);
        func_80058dc4(0, 0);
        func_80057a28();
        if (func_80062c28(0) == 1) {
            func_80057a3c();
            DAT_801cd180.unk10 = 1;
            func_80058ed4();
            loading_screen_show();
            func_8003c0e0();
        } else {
            func_80057a3c();
            DAT_801cd180.unk10 = 1;
            func_80058ed4();
            loading_screen_show();
            game_load_start_file();
        }
    } else {
        game_load_start_file();
    }
    lighting_start();
    func_80044ee8();
    func_8003eaf8();
    heap_init(DAT_8009c888, 0x56400);
    loading_screen_end();
    func_80057a28();
    func_80057a3c();
    DAT_800881a4 = 0;
    for (;;) {
        VECTOR a;
        SVECTOR b;

        frame_rate_update();
        fade_reset();
        func_80042d74();
        func_80050768();
        func_8004e324();
        func_80053aa4();
        effects_run();
        viewer.unk21C = 0;
        func_8005735c();
        func_800456f8();
        func_8003e734(&a, &b);
        func_80058288(&a, &b);
        func_80045ffc();
        func_80058620();
        world_render_frame(&a, &b);
        if (DAT_800881a4 == 0)
            continue;
        VSync(10);
        func_800500ac(2, 0, 4096, 128);
        VSync(10);
        switch (DAT_800881a4) {
        case EXIT_TITLE:
            game_shutdown();
            program = PROGRAM_TITLE;
            break;
        case EXIT_ENDING:
            func_800633d4();
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
