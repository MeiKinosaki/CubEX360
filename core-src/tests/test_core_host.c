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
    /* Byte/halfword bus access must share the same RAM through aliases. */
    assert(cubex_bus_write8(&mem, CUBEX360_RAM_CACHED_BASE + 0x2000u, 0xa5u) == 0);
    assert(cubex_bus_read8(&mem, CUBEX360_RAM_UNCACHED_BASE + 0x2000u, &byte_value) == 0);
    assert(byte_value == 0xa5u);
    assert(cubex_bus_write16be(&mem, CUBEX360_RAM_UNCACHED_BASE + 0x2002u, 0xbeefu) == 0);
    assert(cubex_bus_read16be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2002u, &half_value) == 0);
    assert(half_value == 0xbeefu);
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
    /* bc branch-if-CR0-EQ to +8; intervening addi must be skipped. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 72u, (16u<<26)|(12u<<21)|(2u<<16)|(2u<<2)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 76u, (14u<<26)|(22u<<21)|1u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 80u, (14u<<26)|(22u<<21)|2u) == 0);
    /* bc not taken when CR0.EQ is false; bcl always updates LR and branches. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 84u, (16u<<26)|(12u<<21)|(2u<<16)|(2u<<2)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 88u, (14u<<26)|(23u<<21)|3u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 92u, (16u<<26)|(20u<<21)|(2u<<2)|1u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 96u, (14u<<26)|(24u<<21)|1u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 100u, (14u<<26)|(24u<<21)|2u) == 0);
    /* blr: bclr 20,0,0 returns through LR; also test CTR-conditioned BC. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 104u, (19u<<26)|(20u<<21)|(16u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 108u, (14u<<26)|(25u<<21)|1u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 112u, (16u<<26)|(16u<<21)|(0u<<16)|(2u<<2)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 116u, (14u<<26)|(26u<<21)|1u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 120u, (14u<<26)|(26u<<21)|2u) == 0);
    /* BC CTR condition not taken when CTR decrements to zero. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 124u, (16u<<26)|(16u<<21)|(2u<<2)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 128u, (14u<<26)|(27u<<21)|1u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 132u, (14u<<26)|(27u<<21)|2u) == 0);
    /* bctr: indirect branch through CTR, without decrementing CTR. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 136u, (19u<<26)|(20u<<21)|(528u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 140u, (14u<<26)|(28u<<21)|1u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 144u, (14u<<26)|(28u<<21)|2u) == 0);
    /* cmpwi r29,0 and cmplwi r30,0x100; mflr r31; mfctr r1; mtlr r2; mtctr r3 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 148u, (11u<<26)|(2u<<23)|(29u<<16)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 152u, (10u<<26)|(3u<<23)|(30u<<16)|0x100u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 156u, (31u<<26)|(31u<<21)|(8u<<16)|(339u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 160u, (31u<<26)|(1u<<21)|(9u<<16)|(339u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 164u, (31u<<26)|(2u<<21)|(8u<<16)|(467u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 168u, (31u<<26)|(3u<<21)|(9u<<16)|(467u<<1)) == 0);
    /* rlwinm and rlwimi: rotate/mask and insert while preserving other bits. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 180u, (21u<<26)|(4u<<21)|(5u<<16)|(8u<<11)|(23u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 184u, (21u<<26)|(4u<<21)|(6u<<16)|(24u<<6)|(31u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 188u, (20u<<26)|(4u<<21)|(7u<<16)|(24u<<6)|(31u<<1)) == 0);
    /* rlwnm r8,r4,r9,8,15; rotate by register then apply wrapped PPC mask. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 208u,
        (23u<<26)|(4u<<21)|(8u<<16)|(9u<<11)|(8u<<6)|(15u<<1)) == 0);
    /* slw r13,r14,r15; srw r9,r4,r10; srawi. r11,r12,4 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 192u,
        (31u<<26)|(14u<<21)|(13u<<16)|(15u<<11)|(24u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 196u,
        (31u<<26)|(4u<<21)|(9u<<16)|(10u<<11)|(536u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 200u,
        (31u<<26)|(12u<<21)|(11u<<16)|(4u<<11)|(824u<<1)|1u) == 0);
    /* cmpw cr4,r29,r30; cmplw cr4,r29,r30 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 172u, (31u<<26)|(4u<<23)|(29u<<16)|(30u<<11)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 176u, (31u<<26)|(4u<<23)|(29u<<16)|(30u<<11)|(32u<<1)) == 0);
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
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 72u);
    cpu.cr = 0x20000000u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 80u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[22] == 2u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 84u);
    cpu.cr = 0x40000000u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 88u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[23] == 3u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 92u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 100u);
    assert(cpu.lr == CUBEX360_RAM_CACHED_BASE + 96u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[24] == 2u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 104u);
    cpu.lr = CUBEX360_RAM_CACHED_BASE + 112u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 112u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 112u);
    cpu.ctr = 2u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.ctr == 1u);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 120u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[26] == 2u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 124u);
    cpu.ctr = 1u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.ctr == 0u);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 128u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[27] == 1u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 136u);
    cpu.ctr = CUBEX360_RAM_CACHED_BASE + 144u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 144u);
    assert(cpu.ctr == CUBEX360_RAM_CACHED_BASE + 144u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[28] == 2u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 148u);
    cpu.gpr[29] = 0u;
    cpu.gpr[30] = 0x100u;
    cpu.lr = 0xabcdef00u;
    cpu.ctr = 7u;
    cpu.gpr[2] = 0x12345678u;
    cpu.gpr[3] = 0x87654321u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert((cpu.cr & 0x00f00000u) == 0x00200000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert((cpu.cr & 0x000f0000u) == 0x00020000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[31] == 0xabcdef00u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[1] == 7u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.lr == 0x12345678u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.ctr == 0x87654321u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 172u);
    cpu.gpr[29] = 0xffffff80u;
    cpu.gpr[30] = 3u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert((cpu.cr & 0x0000f000u) == 0x00008000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert((cpu.cr & 0x0000f000u) == 0x00004000u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 180u);
    cpu.gpr[4] = 0x0001012au;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[5] == 0x01012a00u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 0x2au);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 188u);
    cpu.gpr[4] = 0xabcdef2au;
    cpu.gpr[7] = 0x12345600u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[7] == 0x1234562au);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 208u);
    cpu.gpr[4] = 0x12345678u;
    cpu.gpr[9] = 8u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[8] == 0x00560000u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 192u);
    cpu.gpr[14] = 0x00000003u;
    cpu.gpr[15] = 4u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[13] == 0x30u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 196u);
    cpu.gpr[4] = 0x80000000u;
    cpu.gpr[10] = 4u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[9] == 0x08000000u);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 200u);
    cpu.gpr[12] = 0xfffffff1u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[11] == 0xffffffffu);
    assert((cpu.xer & 0x20000000u) != 0u);
    assert((cpu.cr & 0xf0000000u) == 0x80000000u);
    /* Shift by zero clears CA even when the source is negative. */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 200u);
    cpu.gpr[12] = 0xffffffffu;
    cpu.xer = 0x20000000u;
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 200u,
        (31u<<26)|(12u<<21)|(11u<<16)|(824u<<1)|1u) == 0);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[11] == 0xffffffffu);
    assert((cpu.xer & 0x20000000u) == 0u);
    /* xoris r21,r4,0x0001; cntlzw r22,r4; andc r23,r4,r5; nor r24,r4,r5; extsb r25,r26; extsh r27,r28 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 212u,
        (27u<<26)|(4u<<21)|(21u<<16)|1u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 216u,
        (31u<<26)|(4u<<21)|(22u<<16)|(26u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 220u,
        (31u<<26)|(4u<<21)|(23u<<16)|(5u<<11)|(60u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 224u,
        (31u<<26)|(4u<<21)|(24u<<16)|(5u<<11)|(124u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 228u,
        (31u<<26)|(26u<<21)|(25u<<16)|(954u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 232u,
        (31u<<26)|(28u<<21)|(27u<<16)|(922u<<1)) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 212u);
    cpu.gpr[4] = 0x00001234u;
    cpu.gpr[5] = 0x00000ff0u;
    cpu.gpr[26] = 0x00000080u;
    cpu.gpr[28] = 0x00008001u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[21] == 0x00011234u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[22] == 19u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[23] == 0x00001004u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[24] == 0xffffe00bu);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[25] == 0xffffff80u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[27] == 0xffff8001u);
    /* NEG, EQV and NAND instruction coverage. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x240u,
        (31u<<26)|(4u<<21)|(29u<<16)|(104u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x244u,
        (31u<<26)|(4u<<21)|(30u<<16)|(5u<<11)|(284u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x248u,
        (31u<<26)|(4u<<21)|(31u<<16)|(5u<<11)|(476u<<1)) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x240u);
    cpu.gpr[4] = 0x0000000fu;
    cpu.gpr[5] = 0x000000f0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[29] == 0xfffffff1u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[30] == 0xffffff00u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[31] == 0xffffffffu);
    /* ORC and XER SPR move coverage. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x260u,
        (31u<<26)|(4u<<21)|(5u<<16)|(5u<<11)|(412u<<1)) == 0);
    /* mfspr r6,XER (SPR encoding for SPR 1). */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x264u,
        (31u<<26)|(6u<<21)|(1u<<16)|(339u<<1)) == 0);
    /* mtspr XER,r7 (SPR encoding for SPR 1). */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x268u,
        (31u<<26)|(7u<<21)|(1u<<16)|(467u<<1)) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x260u);
    cpu.gpr[4] = 0x00ff00ffu;
    cpu.gpr[5] = 0x0f0f0f0fu;
    cpu.xer = 0x20000000u;
    cpu.gpr[7] = 0x80000000u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[5] == 0xf0fff0ffu);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.gpr[6] == 0x20000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK && cpu.xer == 0x80000000u);
    /* Bounded execution loop runs exactly the requested instruction budget. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x300u, 0x3860002au) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x304u, 0x38830001u) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x300u);
    assert(cubex_cpu_run(&cpu, &mem, 2u) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 42u && cpu.gpr[4] == 43u);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 0x308u);
    assert(cpu.retired_instructions == 2u);
    cubex_memory_destroy(&mem);
    puts("CubEX360 core host smoke: PASS");
    return 0;
}
