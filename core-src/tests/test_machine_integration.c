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
    assert(cubex_bus_write32be(&machine.memory, entry + 0x28u, 0x90c50104u) == 0); /* stw r6,4(r5) */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x2cu, 0x80e50104u) == 0); /* lwz r7,4(r5) */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x30u, 0x39000055u) == 0); /* li r8,0x55 */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x34u, 0x99050105u) == 0); /* stb r8,5(r5) */
    assert(cubex_bus_write32be(&machine.memory, entry + 0x38u, 0x89250105u) == 0); /* lbz r9,5(r5) */
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
    assert(cubex_bus_write32be(&machine.memory, entry + 0x64u, 0x38c00002u) == 0);
    assert(cubex_bus_write32be(&machine.memory, entry + 0x68u, 0xb0c50002u) == 0);
    assert(cubex_bus_write32be(&machine.memory, entry + 0x6cu, 0xa0e50002u) == 0);
    machine.cpu.pc = entry + 0x60u;
    assert(cubex_machine_run(&machine, 4u) == CUBEX_CPU_OK);
    assert(machine.cpu.gpr[7] == CUBEX_CP_CR_IRQ_ENABLE);




    /* CP FIFO watermark event is routed into PI cause bit 11. */
    {
        uint16_t cp_status = 0u;
        assert(cubex_bus_write16be(&machine.memory, 0xCC000002u,
                                   CUBEX_CP_CR_OVERFLOW_IRQ) == 0);
        assert(cubex_bus_write32be(&machine.memory, 0xCC003004u,
                                   CUBEX_PI_IRQ_CP) == 0);
        cubex_machine_cp_set_status(&machine, CUBEX_CP_SR_OVERFLOW, 0u);
        assert(cubex_bus_read32be(&machine.memory, 0xCC003000u, &value) == 0);
        assert((value & CUBEX_PI_IRQ_CP) != 0u);
        assert(cubex_pi_irq_pending(&machine.pi));
        assert(cubex_bus_write16be(&machine.memory, 0xCC000004u,
                                   CUBEX_CP_SR_OVERFLOW) == 0);
        assert(cubex_bus_read16be(&machine.memory, 0xCC000000u,
                                  &cp_status) == 0);
        assert((cp_status & CUBEX_CP_SR_OVERFLOW) == 0u);
        assert(cubex_bus_read32be(&machine.memory, 0xCC003000u, &value) == 0);
        assert((value & CUBEX_PI_IRQ_CP) != 0u);
        assert(cubex_bus_write32be(&machine.memory, 0xCC003000u,
                                   CUBEX_PI_IRQ_CP) == 0);
        assert(!cubex_pi_irq_pending(&machine.pi));
        assert(cubex_bus_write16be(&machine.memory, 0xCC000002u, 0u) == 0);
    }

    /* PE registers are 16-bit; PE events latch PI cause independently of PI mask. */
    {
        uint16_t half = 0u;
        assert(cubex_bus_write16be(&machine.memory, 0xCC00100Au,
                                   CUBEX_PE_INT_FINISH_ENABLE |
                                   CUBEX_PE_INT_TOKEN_ENABLE) == 0);
        assert(cubex_bus_read16be(&machine.memory, 0xCC00100Au, &half) == 0);
        assert(half == (CUBEX_PE_INT_FINISH_ENABLE |
                        CUBEX_PE_INT_TOKEN_ENABLE));
        assert(cubex_bus_write32be(&machine.memory, 0xCC00100Au, 0u) != 0);
        assert(cubex_bus_write16be(&machine.memory, 0xCC001001u, 0u) != 0);
        assert(cubex_bus_write32be(&machine.memory, 0xCC003004u,
                                   CUBEX_PI_IRQ_PE_FINISH |
                                   CUBEX_PI_IRQ_PE_TOKEN) == 0);
        cubex_machine_pe_raise_finish(&machine);
        cubex_machine_pe_raise_token(&machine, 0x4a5bu);
        assert(cubex_bus_read16be(&machine.memory, 0xCC00100Eu, &half) == 0);
        assert(half == 0x4a5bu);
        assert(cubex_bus_read16be(&machine.memory, 0xCC00100Au, &half) == 0);
        assert((half & (CUBEX_PE_INT_FINISH | CUBEX_PE_INT_TOKEN)) ==
               (CUBEX_PE_INT_FINISH | CUBEX_PE_INT_TOKEN));
        assert(cubex_bus_read32be(&machine.memory, 0xCC003000u, &value) == 0);
        assert((value & (CUBEX_PI_IRQ_PE_FINISH | CUBEX_PI_IRQ_PE_TOKEN)) ==
               (CUBEX_PI_IRQ_PE_FINISH | CUBEX_PI_IRQ_PE_TOKEN));
        assert(cubex_pi_irq_pending(&machine.pi));
        assert(cubex_bus_write32be(&machine.memory, 0xCC003000u,
                                   CUBEX_PI_IRQ_PE_FINISH) == 0);
        assert(cubex_bus_read32be(&machine.memory, 0xCC003000u, &value) == 0);
        assert((value & CUBEX_PI_IRQ_PE_FINISH) == 0u);
        assert((value & CUBEX_PI_IRQ_PE_TOKEN) != 0u);
        assert(cubex_pi_irq_pending(&machine.pi));
        assert(cubex_bus_write16be(&machine.memory, 0xCC00100Au,
                                   CUBEX_PE_INT_FINISH |
                                   CUBEX_PE_INT_TOKEN) == 0);
        assert(cubex_bus_read16be(&machine.memory, 0xCC00100Au, &half) == 0);
        assert((half & CUBEX_PE_INT_PENDING_MASK) == 0u);
        assert(cubex_bus_write32be(&machine.memory, 0xCC003000u,
                                   CUBEX_PI_IRQ_PE_TOKEN) == 0);
        assert(!cubex_pi_irq_pending(&machine.pi));
    }

    assert(cubex_bus_write32be(&machine.memory, entry + 0x40u, 0xaabbccddu) == 0);
    assert(cubex_machine_dma_copy(&machine, entry + 0x40u,
           CUBEX360_RAM_UNCACHED_BASE + 0x200u, 4u) == 0);
    assert(cubex_bus_read32be(&machine.memory,
           CUBEX360_RAM_CACHED_BASE + 0x200u, &value) == 0);
    assert(value == 0xaabbccddu);



    /* PI external interrupt enters the Gekko exception vector and RFI restores context. */
    cubex_machine_destroy(&machine);
    assert(cubex_machine_init(&machine, entry, 2u, 0) == 0);
    assert(cubex_bus_write32be(&machine.memory, entry, 0x48000000u) == 0);
    assert(cubex_bus_write32be(&machine.memory, 0x00000500u, 0x4c000064u) == 0); /* rfi */
    assert(cubex_bus_write16be(&machine.memory, 0xCC00100Au,
                               CUBEX_PE_INT_FINISH_ENABLE) == 0);
    assert(cubex_bus_write32be(&machine.memory, 0xCC003004u,
                               CUBEX_PI_IRQ_PE_FINISH) == 0);
    machine.cpu.msr = 0x0005ff3bu; /* EE + privilege/FP/trace/MMU + ILE/ME */
    cubex_machine_pe_raise_finish(&machine);
    assert(cubex_pi_irq_pending(&machine.pi));
    assert(cubex_machine_step(&machine) == CUBEX_CPU_OK);
    assert(machine.cpu.pc == 0x00000500u);
    assert(machine.cpu.srr0 == entry);
    assert(machine.cpu.srr1 == 0x0000ff3bu);
    assert(machine.cpu.msr == 0x00011001u); /* ME, ILE and LE survive per 750CL rules */
    assert(machine.cpu.retired_instructions == 0u);
    assert(cubex_machine_step(&machine) == CUBEX_CPU_OK); /* RFI */
    assert(machine.cpu.pc == entry);
    assert(machine.cpu.msr == 0x0000ff3bu); /* RFI restores architecturally saved SRR1 fields */
    assert(machine.cpu.retired_instructions == 1u);
    assert(cubex_bus_write32be(&machine.memory, 0xCC003000u,
                               CUBEX_PI_IRQ_PE_FINISH) == 0);
    assert(!cubex_pi_irq_pending(&machine.pi));


    /* Decrementer expiry enters the 0x900 exception vector and RFI returns. */
    cubex_machine_destroy(&machine);
    assert(cubex_machine_init(&machine, entry, 3u, 0) == 0);
    assert(cubex_bus_write32be(&machine.memory, entry, 0x60000000u) == 0); /* nop */
    assert(cubex_bus_write32be(&machine.memory, 0x00000900u, 0x4c000064u) == 0);
    machine.cpu.msr = 0x00008000u;
    machine.cpu.dec = 0u;
    assert(cubex_machine_step(&machine) == CUBEX_CPU_OK);
    assert(machine.cpu.dec == 0xffffffffu && machine.cpu.dec_pending == 1u);
    assert(machine.cpu.pc == entry + 4u);
    assert(cubex_machine_step(&machine) == CUBEX_CPU_OK);
    assert(machine.cpu.pc == 0x00000900u);
    assert(machine.cpu.srr0 == entry + 4u);
    assert(machine.cpu.dec_pending == 0u);
    assert(machine.cpu.retired_instructions == 1u);
    assert(cubex_machine_step(&machine) == CUBEX_CPU_OK);
    assert(machine.cpu.pc == entry + 4u);
    assert(machine.cpu.msr == 0x00008000u);

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
