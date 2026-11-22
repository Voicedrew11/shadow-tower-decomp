/*
 * ST.EXE: a data-only module (WIP) defining game .bss: the equipment model
 * placements. PSYLINK allocates each object's tentative definitions
 * (COMMON) after the previous object's, starting eight-aligned, in
 * symbol-hash bucket order (docs/executable-linking.md), and every user
 * declares it extern. The spelling equipment_model_placements sorts below
 * equipment_model_buffers in that order (buckets 237 and 427), so it starts
 * an object of its own.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <st/data.h>

DATA(0x801b5220, 0x700, ".bss")
EquipmentModel equipment_model_placements[64];
