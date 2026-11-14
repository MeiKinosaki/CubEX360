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
    const uint32_t base = CUBEX360_RAM_CACHED_BASE + 0x500u;

    assert(cubex_memory_init(&mem) == 0);

    /* MFCR copies all eight condition fields without modifying CR. */
    put_ins(&mem, base, (31u << 26) | (5u << 21) | (19u << 1));
    cubex_cpu_reset(&cpu, base);
    cpu.cr = 0x12345678u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[5] == 0x12345678u);
    assert(cpu.cr == 0x12345678u);

    /* MTCRF updates only the fields selected by FXM (CR0 and CR7). */
    put_ins(&mem, base + 4u, (31u << 26) | (6u << 21) |
            (0x81u << 12) | (144u << 1));
    cubex_cpu_reset(&cpu, base + 4u);
    cpu.gpr[6] = 0xa5b6c7d8u;
    cpu.cr = 0x11223344u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.cr == 0xa1223348u);

    /* MCRXR transfers SO/OV/CA to the selected CR field and clears them. */
    put_ins(&mem, base + 8u, (31u << 26) | (3u << 23) | (512u << 1));
    cubex_cpu_reset(&cpu, base + 8u);
    cpu.cr = 0x12345678u;
    cpu.xer = 0xe0001234u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.cr == 0x123e5678u);
    assert(cpu.xer == 0x00001234u);

    /* A second transfer checks zero flags and preservation of other CR fields. */
    put_ins(&mem, base + 12u, (31u << 26) | (0u << 23) | (512u << 1));
    cubex_cpu_reset(&cpu, base + 12u);
    cpu.cr = 0xabcdef01u;
    cpu.xer = 0x20000055u; /* CA only */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.cr == 0x2bcdef01u);
    assert(cpu.xer == 0x00000055u);

    cubex_memory_destroy(&mem);
    return 0;
}
