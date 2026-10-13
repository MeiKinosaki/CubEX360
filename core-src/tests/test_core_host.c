#include "cubex360/cpu.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    cubex_memory_t mem;
    cubex_cpu_state_t cpu;
    uint32_t value = 0;
    uint8_t byte_value = 0;
    uint16_t half_value = 0;
    assert(cubex_memory_init(&mem) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE, 0x3860002au) == 0);
    assert(cubex_bus_read32be(&mem, CUBEX360_RAM_UNCACHED_BASE, &value) == 0);
    assert(value == 0x3860002au);
    /* andi. r3,r4,0x00ff => r3=0x2a and CR0.GT */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 4u, 0x708300ffu) == 0);
    /* andi. r5,r4,0 => r5=0 and CR0.EQ */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 8u, 0x70850000u) == 0);
    /* xori r6,r4,0x00ff => r6=0x12d */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 12u, 0x688600ffu) == 0);
    /* oris r7,r4,1 => r7=0x0001012a */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 16u, 0x64870001u) == 0);
    /* andis. r8,r4,1 => r8=0x00010000 and CR0.GT */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 20u, 0x74880001u) == 0);
    /* lbz r15,0(r16); lhz r17,2(r16); lha r18,4(r16); stb r19,6(r16); sth r20,8(r16) */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 44u, (34u<<26)|(15u<<21)|(16u<<16)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 48u, (40u<<26)|(17u<<21)|(16u<<16)|2u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 52u, (42u<<26)|(18u<<21)|(16u<<16)|4u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 56u, (38u<<26)|(19u<<21)|(16u<<16)|6u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 60u, (44u<<26)|(20u<<21)|(16u<<16)|8u) == 0);
    /* Misaligned LWZ and unsupported floating-point opcode must fail safely. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 64u, (32u<<26)|(21u<<21)|(16u<<16)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 68u, (63u<<26)) == 0);
    assert(cubex_memory_write8(&mem, 0x100u, 0xabu) == 0);
    assert(cubex_memory_write16be(&mem, 0x102u, 0x1234u) == 0);
    assert(cubex_memory_write16be(&mem, 0x104u, 0xff80u) == 0);
    /* mulli r12,r4,2; addic r13,r4,1; addic. r14,r4,-1 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 32u, (7u<<26)|(12u<<21)|(4u<<16)|2u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 36u, (12u<<26)|(13u<<21)|(4u<<16)|1u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 40u, (13u<<26)|(14u<<21)|(4u<<16)|0xffffu) == 0);
    /* add r9,r4,r11; subf r10,r11,r4 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 24u, (31u<<26)|(9u<<21)|(4u<<16)|(11u<<11)|(266u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 28u, (31u<<26)|(10u<<21)|(11u<<16)|(4u<<11)|(40u<<1)) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE);
    cpu.gpr[4] = 0x0001012au;
    cpu.gpr[16] = 0x100u;
    cpu.gpr[19] = 0x5au;
    cpu.gpr[20] = 0xcafeu;
    cpu.gpr[11] = 3u;
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
    assert(cpu.gpr[6] == 0x000101d5u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[7] == 0x0001012au);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[8] == 0x00010000u);
    assert((cpu.cr & 0xf0000000u) == 0x40000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[9] == 0x0001012du);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[10] == 0x00010127u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[12] == 0x00020254u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[13] == 0x0001012bu);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[14] == 0x00010129u);
    assert((cpu.cr & 0xf0000000u) == 0x40000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[15] == 0xabu);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[17] == 0x1234u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[18] == 0xffffff80u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cubex_memory_read8(&mem, 0x106u, &byte_value) == 0 && byte_value == 0x5au);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cubex_memory_read16be(&mem, 0x108u, &half_value) == 0 && half_value == 0xcafeu);
    assert(cubex_memory_write32be(&mem, 0, 0x12345678u) == 0);
    assert(cubex_memory_read32be(&mem, 0, &value) == 0 && value == 0x12345678u);
    assert(cubex_memory_read32be(&mem, CUBEX360_MAIN_RAM_SIZE, &value) != 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 64u);
    cpu.gpr[16] = 0x101u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_MEMORY_FAULT);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 64u);
    assert(cpu.retired_instructions == 0u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 68u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_UNSUPPORTED);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 68u);
    assert(cpu.retired_instructions == 0u);
    cubex_memory_destroy(&mem);
    puts("CubEX360 core host smoke: PASS");
    return 0;
}
