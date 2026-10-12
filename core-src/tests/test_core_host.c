#include "cubex360/cpu.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    cubex_memory_t mem;
    cubex_cpu_state_t cpu;
    uint32_t value = 0;
    assert(cubex_memory_init(&mem) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE, 0x3860002au) == 0);
    assert(cubex_bus_read32be(&mem, CUBEX360_RAM_UNCACHED_BASE, &value) == 0);
    assert(value == 0x3860002au);
    /* andi. r3,r4,0x00ff => r3=0x2a and CR0.GT */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 4u, 0x706400ffu) == 0);
    /* andi. r5,r4,0 => r5=0 and CR0.EQ */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 8u, 0x70840000u) == 0);
    /* xori r6,r4,0x00ff => r6=0x12d */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 12u, 0x68c400ffu) == 0);
    /* oris r7,r4,1 => r7=0x0001012a */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 16u, 0x64e40001u) == 0);
    /* andis. r8,r4,1 => r8=0x00010000 and CR0.GT */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 20u, 0x75040001u) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE);
    cpu.gpr[4] = 0x0001012au;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 42u);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 4u);
    assert(cpu.retired_instructions == 1u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0x2au);
    assert((cpu.cr & 0xf0000000u) == 0x40000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[5] == 0u);
    assert((cpu.cr & 0xf0000000u) == 0x20000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 0x000100d5u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[7] == 0x0001012au);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[8] == 0x00010000u);
    assert((cpu.cr & 0xf0000000u) == 0x40000000u);
    assert(cubex_memory_write32be(&mem, 0, 0x12345678u) == 0);
    assert(cubex_memory_read32be(&mem, 0, &value) == 0 && value == 0x12345678u);
    assert(cubex_memory_read32be(&mem, CUBEX360_MAIN_RAM_SIZE, &value) != 0);
    cubex_memory_destroy(&mem);
    puts("CubEX360 core host smoke: PASS");
    return 0;
}
