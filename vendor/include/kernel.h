#ifndef KF_KERNEL_H
#define KF_KERNEL_H

/*
 * Runtime Library 4.2 LIBAPI.H includes "kernel.h"; the SDK ships it as
 * KERNEL.H. ASM.H supplies NREGS for its C layouts, and the native
 * preprocessor does not supply the SDK driver's LANGUAGE_C selection.
 */
#ifndef LANGUAGE_C
#define LANGUAGE_C 1
#endif
#include <ASM.H>
#include <KERNEL.H>

#endif
