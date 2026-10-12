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
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 42u);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 4u);
    assert(cpu.retired_instructions == 1u);
    assert(cubex_memory_write32be(&mem, 0, 0x12345678u) == 0);
    assert(cubex_memory_read32be(&mem, 0, &value) == 0 && value == 0x12345678u);
    assert(cubex_memory_read32be(&mem, CUBEX360_MAIN_RAM_SIZE, &value) != 0);
    cubex_memory_destroy(&mem);
    puts("CubEX360 core host smoke: PASS");
    return 0;
}
