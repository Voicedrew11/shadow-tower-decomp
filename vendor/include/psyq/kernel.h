#ifndef KF_PSYQ_KERNEL_H
#define KF_PSYQ_KERNEL_H

/*
 * Runtime Library 4.2 kernel, controller, and BIOS file interfaces.
 * KERNEL.H and SYS/FILE.H provide the vendor constants and layouts and
 * LIBAPI.H the callable declarations. The few entry points LIBAPI.H omits
 * are declared below, spelled exactly as LIBETC.H and MALLOC.H declare the
 * ones they share, so a source may include any of those headers with this.
 */

/* ASM.H supplies NREGS for KERNEL.H's C layouts. The native preprocessor
 * does not supply the SDK driver's LANGUAGE_C selection. */
#ifndef LANGUAGE_C
#define LANGUAGE_C 1
#endif
#include <ASM.H>
#include <KERNEL.H>
#include <SYS/FILE.H>
/* Its "kernel.h" include resolves to the vendor/include/kernel.h shim. */
#include <LIBAPI.H>

/* LIBETC.H's spelling. */
extern int ResetCallback(void);
extern int StopCallback(void);

/* LIBAPI I_HEAP and I_HEAP3, as MALLOC.H declares them. */
extern void InitHeap(unsigned long *head, unsigned long size);
extern void InitHeap3(unsigned long *head, unsigned long size);

extern void InitCARD2(long pad_enable);
extern void StartCARD2(void);
extern void StopCARD2(void);

extern unsigned long PAD_init2(unsigned long mode, unsigned long *pad_state);
extern void PAD_dr(void);
extern void StopPAD2(void);

#if defined(__cplusplus)
/* delete is a C++ keyword; bind the BIOS entry point by its linked name. */
extern long bios_delete(char *name) __asm__("delete");
#else
extern long delete(char *name);
#define bios_delete delete
#endif

#endif
