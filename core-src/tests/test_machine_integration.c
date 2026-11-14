#include "cubex360/machine.h"
#include "cubex360/bus.h"

#include <assert.h>
#include <stdint.h>

int main(void) {
    cubex_machine_t machine;
    uint32_t value;
    const uint32_t entry = CUBEX360_RAM_CACHED_BASE + 0x100u;
    const uint32_t mmio_reg = CUBEX360_MACHINE_MMIO_BASE + 4u;

    assert(cubex_machine_init(&machine, entry, 2u, 0) == 0);
    /* ADDI r3,0,42; ADDI r4,r3,1; B . */
    assert(cubex_bus_write32be(&machine.memory, entry, 0x3860002au) == 0);
    assert(cubex_bus_write32be(&machine.memory, entry + 4u, 0x38830001u) == 0);
    assert(cubex_bus_write32be(&machine.memory, entry + 8u, 0x48000000u) == 0);
    assert(cubex_timer_enable(&machine.timer, 1) == 0);
    assert(cubex_machine_step(&machine) == CUBEX_CPU_OK);
    assert(machine.cpu.gpr[3] == 42u);
    assert(!machine.timer.interrupt_pending);
    assert(cubex_machine_step(&machine) == CUBEX_CPU_OK);
    assert(machine.cpu.gpr[4] == 43u);
    assert(machine.timer.interrupt_pending);
    assert(!machine.timer.enabled);

    assert(cubex_machine_mmio_write32(&machine, mmio_reg, 0x1234abcdu) == 0);
    assert(cubex_machine_mmio_read32(&machine, mmio_reg, &value) == 0);
    assert(value == 0x1234abcdu);
    assert(cubex_machine_mmio_read32(&machine,
           CUBEX360_MACHINE_MMIO_BASE + CUBEX360_MACHINE_MMIO_REGISTERS * 4u,
           &value) != 0);

    assert(cubex_bus_write32be(&machine.memory, entry + 0x40u, 0xaabbccddu) == 0);
    assert(cubex_machine_dma_copy(&machine, entry + 0x40u,
           CUBEX360_RAM_UNCACHED_BASE + 0x200u, 4u) == 0);
    assert(cubex_bus_read32be(&machine.memory,
           CUBEX360_RAM_CACHED_BASE + 0x200u, &value) == 0);
    assert(value == 0xaabbccddu);

    cubex_machine_destroy(&machine);
    assert(cubex_machine_step(&machine) == CUBEX_CPU_BAD_ARGUMENT);
    return 0;
}
