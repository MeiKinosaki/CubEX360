#include "cubex360/machine.h"

#include <string.h>

int cubex_machine_init(cubex_machine_t *machine, uint32_t entry_pc,
                       uint32_t timer_period, int timer_periodic) {
    if (!machine || timer_period == 0u) return -1;
    memset(machine, 0, sizeof(*machine));
    if (cubex_memory_init(&machine->memory) != 0) return -1;
    if (cubex_mmio_init(&machine->mmio, CUBEX360_MACHINE_MMIO_BASE,
                        machine->mmio_storage,
                        CUBEX360_MACHINE_MMIO_REGISTERS) != 0) {
        cubex_memory_destroy(&machine->memory);
        return -1;
    }
    if (cubex_timer_init(&machine->timer, timer_period,
                         timer_periodic != 0) != 0) {
        cubex_memory_destroy(&machine->memory);
        memset(machine, 0, sizeof(*machine));
        return -1;
    }
    cubex_cpu_reset(&machine->cpu, entry_pc);
    machine->initialized = 1;
    return 0;
}

void cubex_machine_destroy(cubex_machine_t *machine) {
    if (!machine) return;
    if (machine->initialized) cubex_memory_destroy(&machine->memory);
    memset(machine, 0, sizeof(*machine));
}

cubex_cpu_result_t cubex_machine_step(cubex_machine_t *machine) {
    cubex_cpu_result_t result;
    if (!machine || !machine->initialized) return CUBEX_CPU_BAD_ARGUMENT;
    result = cubex_cpu_step(&machine->cpu, &machine->memory);
    if (result != CUBEX_CPU_OK) return result;
    if (cubex_timer_advance(&machine->timer, 1u) != 0)
        return CUBEX_CPU_BAD_ARGUMENT;
    return CUBEX_CPU_OK;
}

cubex_cpu_result_t cubex_machine_run(cubex_machine_t *machine,
                                     uint64_t instruction_budget) {
    uint64_t i;
    cubex_cpu_result_t result;
    if (!machine || !machine->initialized) return CUBEX_CPU_BAD_ARGUMENT;
    for (i = 0; i < instruction_budget; ++i) {
        result = cubex_machine_step(machine);
        if (result != CUBEX_CPU_OK) return result;
    }
    return CUBEX_CPU_OK;
}

int cubex_machine_dma_copy(cubex_machine_t *machine, uint32_t source,
                           uint32_t destination, size_t size) {
    if (!machine || !machine->initialized) return -1;
    return cubex_dma_copy_main_ram(&machine->memory, source, destination, size);
}

int cubex_machine_mmio_read32(const cubex_machine_t *machine,
                              uint32_t address, uint32_t *value) {
    if (!machine || !machine->initialized) return -1;
    return cubex_mmio_read32(&machine->mmio, address, value);
}

int cubex_machine_mmio_write32(cubex_machine_t *machine,
                               uint32_t address, uint32_t value) {
    if (!machine || !machine->initialized) return -1;
    return cubex_mmio_write32(&machine->mmio, address, value);
}
