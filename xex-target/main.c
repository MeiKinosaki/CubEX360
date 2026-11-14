#include "cubex360/cpu.h"
#include "xsdk/log.h"
#include <stdint.h>

int main(void) {
    cubex_memory_t mem;
    cubex_cpu_state_t cpu;
    if (cubex_memory_init(&mem) != 0) {
        xsdk_log_write("CubEX360: RAM allocation failed");
        return 1;
    }
    if (cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE, 0x3860002au) != 0 ||
        cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 4u, 0x38830001u) != 0) {
        cubex_memory_destroy(&mem);
        xsdk_log_write("CubEX360: smoke program setup failed");
        return 2;
    }
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE);
    if (cubex_cpu_run(&cpu, &mem, 2u) != CUBEX_CPU_OK ||
        cpu.gpr[3] != 42u || cpu.gpr[4] != 43u ||
        cpu.retired_instructions != 2u) {
        xsdk_log_write("CubEX360: CPU smoke failed r3=%u r4=%u pc=%08x",
                       (unsigned)cpu.gpr[3], (unsigned)cpu.gpr[4], (unsigned)cpu.pc);
        cubex_memory_destroy(&mem);
        return 3;
    }
    xsdk_log_write("CubEX360: CPU smoke PASS r3=%u r4=%u retired=%u",
                   (unsigned)cpu.gpr[3], (unsigned)cpu.gpr[4],
                   (unsigned)cpu.retired_instructions);
    cubex_memory_destroy(&mem);
    return 0;
}
