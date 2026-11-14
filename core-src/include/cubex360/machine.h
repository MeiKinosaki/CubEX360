#ifndef CUBEX360_MACHINE_H
#define CUBEX360_MACHINE_H

#include <stdint.h>
#include "cubex360/cpu.h"
#include "cubex360/cp.h"
#include "cubex360/flipper_map.h"
#include "cubex360/dma.h"
#include "cubex360/mmio.h"
#include "cubex360/timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CUBEX360_MACHINE_MMIO_BASE 0xCC000000u
#define CUBEX360_MACHINE_MMIO_REGISTERS 64u

/* Integration coordinator for currently implemented prototypes.
 * MMIO is a generic bank, DMA is RAM-to-RAM, and timer IRQs are pending
 * signals only; this is not yet the GameCube Flipper hardware map. */
typedef struct cubex_machine {
    cubex_memory_t memory;
    cubex_cpu_state_t cpu;
    cubex_mmio_t mmio;
    cubex_cp_t cp;
    uint32_t mmio_storage[CUBEX360_MACHINE_MMIO_REGISTERS];
    cubex_timer_t timer;
    int initialized;
} cubex_machine_t;

int cubex_machine_init(cubex_machine_t *machine, uint32_t entry_pc,
                       uint32_t timer_period, int timer_periodic);
void cubex_machine_destroy(cubex_machine_t *machine);
cubex_cpu_result_t cubex_machine_step(cubex_machine_t *machine);
cubex_cpu_result_t cubex_machine_run(cubex_machine_t *machine,
                                     uint64_t instruction_budget);
int cubex_machine_dma_copy(cubex_machine_t *machine, uint32_t source,
                           uint32_t destination, size_t size);
int cubex_machine_mmio_read32(const cubex_machine_t *machine,
                              uint32_t address, uint32_t *value);
int cubex_machine_mmio_write32(cubex_machine_t *machine,
                               uint32_t address, uint32_t value);

#ifdef __cplusplus
}
#endif
#endif
