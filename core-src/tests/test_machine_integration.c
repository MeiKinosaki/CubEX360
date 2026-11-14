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

    /* CPU accesses to the MMIO bank must traverse the unified bus. */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x20u, 0x3ca0cc00u) == 0); /* lis r5,0xcc00 */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x24u, 0x38c01234u) == 0); /* li r6,0x1234 */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x28u, 0x90c50004u) == 0); /* stw r6,4(r5) */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x2cu, 0x80e50004u) == 0); /* lwz r7,4(r5) */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x30u, 0x39000055u) == 0); /* li r8,0x55 */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x34u, 0x99050005u) == 0); /* stb r8,5(r5) */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x38u, 0x89250005u) == 0); /* lbz r9,5(r5) */
    machine.cpu.pc = entry + 0x20u;
    assert(cubex_machine_run(&machine, 7u) == CUBEX_CPU_OK);
    assert(machine.cpu.gpr[7] == 0x1234u);
    assert(machine.cpu.gpr[9] == 0x55u);
    assert(cubex_machine_mmio_read32(&machine, mmio_reg, &value) == 0);
    assert(value == 0x00551234u);

    /* CP is a native 16-bit device: no 32-bit read-modify-write is allowed. */
    {
        uint16_t half = 0u;
        assert(cubex_bus_read16be(&machine.memory, 0xCC000000u, &half) == 0);
        assert(half == (CUBEX_CP_SR_GP_IDLE | CUBEX_CP_SR_READ_IDLE));
        assert(cubex_bus_write16be(&machine.memory, 0xCC000002u,
                                   CUBEX_CP_CR_IRQ_ENABLE) == 0);
        assert(cubex_bus_read16be(&machine.memory, 0xCC000002u, &half) == 0);
        assert(half == CUBEX_CP_CR_IRQ_ENABLE);
        assert(cubex_bus_read32be(&machine.memory, 0xCC000000u, &value) != 0);
        assert(cubex_bus_write32be(&machine.memory, 0xCC000000u, 0u) != 0);
        assert(cubex_bus_read16be(&machine.memory, 0xCC000001u, &half) != 0);
        assert(cubex_bus_write16be(&machine.memory, 0xCC000003u, 0u) != 0);
        cubex_cp_set_status(&machine.cp, CUBEX_CP_SR_OVERFLOW, 0u);
        assert(cubex_bus_write16be(&machine.memory, 0xCC000004u,
                                   CUBEX_CP_SR_OVERFLOW) == 0);
        assert(cubex_bus_read16be(&machine.memory, 0xCC000000u, &half) == 0);
        assert((half & CUBEX_CP_SR_OVERFLOW) == 0u);
    }

    /* CPU STH/LHZ instructions reach the CP's 16-bit callback. */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x60u, 0x3ca0cc00u) == 0);
    assert(cubex_bus_write32be(&machine.memory, entry + 0x64u, 0x38c00022u) == 0);
    assert(cubex_bus_write32be(&machine.memory, entry + 0x68u, 0xb0c50002u) == 0);
    assert(cubex_bus_write32be(&machine.memory, entry + 0x6cu, 0xa0e50002u) == 0);
    machine.cpu.pc = entry + 0x60u;
    assert(cubex_machine_run(&machine, 4u) == CUBEX_CPU_OK);
    assert(machine.cpu.gpr[7] == CUBEX_CP_CR_IRQ_ENABLE);


    assert(cubex_bus_write32be(&machine.memory, entry + 0x40u, 0xaabbccddu) == 0);
    assert(cubex_machine_dma_copy(&machine, entry + 0x40u,
           CUBEX360_RAM_UNCACHED_BASE + 0x200u, 4u) == 0);
    assert(cubex_bus_read32be(&machine.memory,
           CUBEX360_RAM_CACHED_BASE + 0x200u, &value) == 0);
    assert(value == 0xaabbccddu);


    /* Periodic timer advances only for retired instructions, including run(). */
    cubex_machine_destroy(&machine);
    assert(cubex_machine_init(&machine, entry, 2u, 1) == 0);
    assert(cubex_bus_write32be(&machine.memory, entry, 0x48000000u) == 0);
    assert(cubex_timer_enable(&machine.timer, 1) == 0);
    assert(cubex_machine_run(&machine, 5u) == CUBEX_CPU_OK);
    assert(machine.cpu.retired_instructions == 5u);
    assert(machine.cpu.pc == entry);
    assert(machine.timer.interrupt_pending);
    assert(machine.timer.enabled && machine.timer.remaining == 1u);
    cubex_timer_clear_interrupt(&machine.timer);
    assert(cubex_machine_step(&machine) == CUBEX_CPU_OK);
    assert(machine.timer.interrupt_pending && machine.timer.remaining == 2u);

    /* A fetch fault must not consume a device tick or retire an instruction. */
    cubex_machine_destroy(&machine);
    assert(cubex_machine_init(&machine, 0x70000000u, 3u, 1) == 0);
    assert(cubex_timer_enable(&machine.timer, 1) == 0);
    assert(cubex_machine_step(&machine) == CUBEX_CPU_MEMORY_FAULT);
    assert(machine.cpu.retired_instructions == 0u);
    assert(machine.timer.remaining == 3u && !machine.timer.interrupt_pending);

    cubex_machine_destroy(&machine);
    assert(cubex_machine_step(&machine) == CUBEX_CPU_BAD_ARGUMENT);
    return 0;
}
