#include "cubex360/cpu.h"
#include <stdio.h>
int main(void) {
    cubex_memory_t mem;
    cubex_cpu_state_t cpu;
    if (cubex_memory_init(&mem)) return 1;
    if (cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE, 0x3860002au) ||
        cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 4u, 0x38830001u)) {
        cubex_memory_destroy(&mem);
        return 2;
    }
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE);
    if (cubex_cpu_run(&cpu, &mem, 2u) != CUBEX_CPU_OK ||
        cpu.gpr[3] != 42u || cpu.gpr[4] != 43u ||
        cpu.retired_instructions != 2u) {
        cubex_memory_destroy(&mem);
        return 3;
    }
    printf("CubEX360 integrated core smoke: r3=%u r4=%u pc=%08x retired=%llu\n",
           (unsigned)cpu.gpr[3], (unsigned)cpu.gpr[4], (unsigned)cpu.pc,
           (unsigned long long)cpu.retired_instructions);
    cubex_memory_destroy(&mem);
    return 0;
}
