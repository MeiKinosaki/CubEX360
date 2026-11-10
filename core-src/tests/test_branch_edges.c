#include "cubex360/cpu.h"
#include "cubex360/bus.h"
#include <assert.h>
#include <stdint.h>

static void put_ins(cubex_memory_t *m, uint32_t address, uint32_t ins) {
    assert(cubex_bus_write32be(m, address, ins) == 0);
}

int main(void) {
    cubex_memory_t mem;
    cubex_cpu_state_t cpu;
    const uint32_t base = CUBEX360_RAM_CACHED_BASE + 0x300u;
    assert(cubex_memory_init(&mem) == 0);

    /* BC: unconditional relative branch with LK updates LR to next PC. */
    put_ins(&mem, base, (16u << 26) | (20u << 21) | 8u | 1u);
    cubex_cpu_reset(&cpu, base);
    cpu.lr = 0xdeadbeefu;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == base + 8u);
    assert(cpu.lr == base + 4u);
    assert(cpu.retired_instructions == 1u);

    /* BC with CR condition: taken when BI=2 is one; not taken when zero. */
    put_ins(&mem, base + 4u, (16u << 26) | (12u << 21) |
            (2u << 16) | 8u);
    cubex_cpu_reset(&cpu, base + 4u);
    cpu.cr = 0x20000000u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == base + 12u);
    cubex_cpu_reset(&cpu, base + 4u);
    cpu.cr = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == base + 8u);

    /* BCLR uses the old LR as target, then writes the link address. */
    put_ins(&mem, base + 8u, (19u << 26) | (20u << 21) |
            (16u << 1) | 1u);
    cubex_cpu_reset(&cpu, base + 8u);
    cpu.lr = base + 0x40u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == base + 0x40u);
    assert(cpu.lr == base + 12u);

    /* BCCTR branches to CTR without decrementing it; LK remains independent. */
    put_ins(&mem, base + 12u, (19u << 26) | (20u << 21) |
            (528u << 1) | 1u);
    cubex_cpu_reset(&cpu, base + 12u);
    cpu.ctr = base + 0x80u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == base + 0x80u);
    assert(cpu.ctr == base + 0x80u);
    assert(cpu.lr == base + 16u);

    /* BCCTR with CR condition not met must fall through and preserve CTR. */
    put_ins(&mem, base + 16u, (19u << 26) | (12u << 21) |
            (1u << 16) | (528u << 1));
    cubex_cpu_reset(&cpu, base + 16u);
    cpu.ctr = base + 0x90u;
    cpu.cr = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == base + 20u);
    assert(cpu.ctr == base + 0x90u);

    cubex_memory_destroy(&mem);
    return 0;
}
