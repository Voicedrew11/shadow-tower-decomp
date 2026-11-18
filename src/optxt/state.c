/*
 * OPTXT.EXE: the opening's state word, a global of an object of its own
 * (WIP module). src/optxt/main.c only declares it, which is why its code
 * reaches it with absolute addresses while its own small data is
 * $gp-relative; the linker places the small common after main's .sbss.
 */
#include <kf/lib/address.h>
#include <optxt/state.h>

DATA(0x8001f7ec, 0x4, ".sbss")
int opening_state;
