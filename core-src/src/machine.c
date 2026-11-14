#include "cubex360/machine.h"

#include <string.h>

static void machine_sync_device_irqs(cubex_machine_t *machine) {
    uint32_t causes = 0u;
    if (!machine) return;
    if (cubex_cp_irq_pending(&machine->cp)) causes |= CUBEX_PI_IRQ_CP;
    if (cubex_pe_finish_irq(&machine->pe)) causes |= CUBEX_PI_IRQ_PE_FINISH;
    if (cubex_pe_token_irq(&machine->pe)) causes |= CUBEX_PI_IRQ_PE_TOKEN;
    if (causes) cubex_pi_raise(&machine->pi, causes);
}

static int machine_bus_mmio_read16(void *context, uint32_t address,
                                   uint16_t *value) {
    cubex_machine_t *machine = (cubex_machine_t *)context;
    cubex_flipper_decoded_address_t decoded;
    if (!machine || !value) return -1;
    if (cubex_flipper_decode_address(address, &decoded) != 0) return -2;
    if (decoded.region == CUBEX_FLIPPER_REGION_CP)
        return cubex_cp_read16(&machine->cp, address, value);
    if (decoded.region == CUBEX_FLIPPER_REGION_PE)
        return cubex_pe_read16(&machine->pe, address, value);
    return -2;
}

static int machine_bus_mmio_write16(void *context, uint32_t address,
                                    uint16_t value) {
    cubex_machine_t *machine = (cubex_machine_t *)context;
    cubex_flipper_decoded_address_t decoded;
    int result;
    if (!machine) return -1;
    if (cubex_flipper_decode_address(address, &decoded) != 0) return -2;
    if (decoded.region == CUBEX_FLIPPER_REGION_CP)
        return cubex_cp_write16(&machine->cp, address, value);
    if (decoded.region != CUBEX_FLIPPER_REGION_PE) return -1;
    result = cubex_pe_write16(&machine->pe, address, value);
    machine_sync_device_irqs(machine);
    return result;
}

static int machine_bus_mmio_read32(void *context, uint32_t address,
                                   uint32_t *value) {
    cubex_machine_t *machine = (cubex_machine_t *)context;
    cubex_flipper_decoded_address_t decoded;
    if (!machine || !value) return -1;
    if (cubex_flipper_decode_address(address, &decoded) == 0) {
        if (decoded.region == CUBEX_FLIPPER_REGION_PI)
            return cubex_pi_read32(&machine->pi, address, value);
        return -1;
    }
    return cubex_mmio_read32(&machine->mmio, address, value);
}

static int machine_bus_mmio_write32(void *context, uint32_t address,
                                    uint32_t value) {
    cubex_machine_t *machine = (cubex_machine_t *)context;
    cubex_flipper_decoded_address_t decoded;
    if (!machine) return -1;
    if (cubex_flipper_decode_address(address, &decoded) == 0) {
        if (decoded.region == CUBEX_FLIPPER_REGION_PI)
            return cubex_pi_write32(&machine->pi, address, value);
        return -1;
    }
    return cubex_mmio_write32(&machine->mmio, address, value);
}

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
    machine->memory.bus_context = machine;
    machine->memory.mmio_read32 = machine_bus_mmio_read32;
    machine->memory.mmio_write32 = machine_bus_mmio_write32;
    machine->memory.mmio_read16 = machine_bus_mmio_read16;
    machine->memory.mmio_write16 = machine_bus_mmio_write16;
    cubex_cp_init(&machine->cp);
    cubex_pe_init(&machine->pe);
    cubex_pi_init(&machine->pi);
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

void cubex_machine_cp_set_status(cubex_machine_t *machine, uint16_t set_bits,
                                 uint16_t clear_bits) {
    if (!machine || !machine->initialized) return;
    cubex_cp_set_status(&machine->cp, set_bits, clear_bits);
    machine_sync_device_irqs(machine);
}

void cubex_machine_pe_raise_finish(cubex_machine_t *machine) {
    if (!machine || !machine->initialized) return;
    cubex_pe_raise_finish(&machine->pe);
    machine_sync_device_irqs(machine);
}

void cubex_machine_pe_raise_token(cubex_machine_t *machine, uint16_t token) {
    if (!machine || !machine->initialized) return;
    cubex_pe_raise_token(&machine->pe, token);
    machine_sync_device_irqs(machine);
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
