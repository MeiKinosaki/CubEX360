#include <stdint.h>
#include "cubex360/cpu.h"
#include "xsdk/log.h"

int main(void) {
    cubex_memory_t memory;
    cubex_cpu_state_t cpu;
    cubex_cpu_result_t result;

    if (cubex_memory_init(&memory) != 0) {
        xsdk_log_write("CubEX360: FAIL: could not allocate GameCube main RAM");
        return 1;
    }

    /* addi r3,r0,42 ; addi r4,r3,1 */
    if (cubex_bus_write32be(&memory, CUBEX360_RAM_CACHED_BASE, 0x3860002au) != 0 ||
        cubex_bus_write32be(&memory, CUBEX360_RAM_CACHED_BASE + 4u, 0x38830001u) != 0) {
        xsdk_log_write("CubEX360: FAIL: could not stage Gekko smoke program");
        cubex_memory_destroy(&memory);
        return 2;
    }

    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE);
    result = cubex_cpu_run(&cpu, &memory, 2u);
    if (result != CUBEX_CPU_OK || cpu.gpr[3] != 42u || cpu.gpr[4] != 43u ||
        cpu.retired_instructions != 2u) {
        xsdk_log_write("CubEX360: FAIL: result=%u r3=%u r4=%u retired=%llu",
                       (unsigned)result, (unsigned)cpu.gpr[3],
                       (unsigned)cpu.gpr[4],
                       (unsigned long long)cpu.retired_instructions);
        cubex_memory_destroy(&memory);
        return 3;
    }

    xsdk_log_write("CubEX360: PASS: Gekko core executed 2 instructions; r3=%u r4=%u pc=%08X",
                   (unsigned)cpu.gpr[3], (unsigned)cpu.gpr[4], (unsigned)cpu.pc);
    cubex_memory_destroy(&memory);
    return 0;
}
