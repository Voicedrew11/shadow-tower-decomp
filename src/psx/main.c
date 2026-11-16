/*
 * PSX.EXE: the boot program. It loads and executes the six other programs
 * from the disc in turn; each one leaves the index of its successor in the
 * byte at 0x800103f0 before it returns.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <psyq/kernel.h>

/*
 * 0x800103f0-0x800103f8 lie just past PSX.EXE's linked bss, inside its load
 * window and below the 0x80011000 load address of the other programs. Only
 * these fixed addresses are evidenced; whether the original spelled numbers
 * or link-time symbols is unknown.
 */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800103f0)
#define MAILBOX_BYTE_1 ((u8 *)0x800103f1)
#define MAILBOX_BYTE_2 ((u8 *)0x800103f2)
#define MAILBOX_BYTE_8 ((u8 *)0x800103f8)

static char *program_paths[] = {
    "cdrom:STR.EXE;1", "cdrom:TITLE.EXE;1", "cdrom:OPTXT.EXE;1",
    "cdrom:ST.EXE;1", "cdrom:END.EXE;1", "cdrom:VS.EXE;1",
};
int program_index = 0;
u8 *next_program = NEXT_PROGRAM_BYTE;
u8 *PTR_8001039c = MAILBOX_BYTE_1;
u8 *PTR_800103a0 = MAILBOX_BYTE_2;
struct EXEC program_header;

ADDRESS(0x80010068, 0xe8)
int main(void)
{
    _96_remove();
    _96_init();
    *next_program = 0;
    *PTR_8001039c = 0;
    *PTR_800103a0 = 0;
    *MAILBOX_BYTE_8 = 0;
    for (;;) {
        ExitCriticalSection();
        if (Load(program_paths[program_index], &program_header) == 1) {
            _96_remove();
            program_header.s_addr = 0;
            program_header.s_size = 0;
            EnterCriticalSection();
            Exec(&program_header, 1, 0);
            program_index = *next_program;
            _96_init();
        }
    }
    return 0;
}
