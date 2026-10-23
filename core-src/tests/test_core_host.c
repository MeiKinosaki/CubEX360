#include "cubex360/cpu.h"
#include "cubex360/bus.h"
#include "cubex360/state.h"
#include "cubex360/covers.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int mock_cover_downloads;
static int mock_cover_download(const char *url, const char *destination, void *user_data) {
    FILE *f;
    (void)user_data;
    if (strcmp(url, "https://covers.example/gc/GAMECOVR.png") != 0) return -1;
    f = fopen(destination, "wb");
    if (!f) return -1;
    if (fwrite("PNG", 1, 3, f) != 3) { fclose(f); return -1; }
    if (fclose(f) != 0) return -1;
    ++mock_cover_downloads;
    return 0;
}
int main(void) {
    cubex_memory_t mem;
    cubex_cpu_state_t cpu;
    uint32_t value = 0;
    uint8_t byte_value = 0;
    uint16_t half_value = 0;
    assert(cubex_memory_init(&mem) == 0);
    /* Cover-cache logic: downloads once, then reuses cached file. */
    remove("/tmp/GAMECOVR.png");
    mock_cover_downloads = 0;
    assert(cubex_cover_ensure("https://covers.example/gc", "/tmp", "GAMECOVR",
                              mock_cover_download, NULL) == 1);
    assert(mock_cover_downloads == 1);
    assert(cubex_cover_ensure("https://covers.example/gc", "/tmp", "GAMECOVR",
                              mock_cover_download, NULL) == 0);
    assert(mock_cover_downloads == 1);
    assert(cubex_cover_ensure("https://covers.example/gc", "/tmp", "../BAD",
                              mock_cover_download, NULL) < 0);
    remove("/tmp/GAMECOVR.png");

    /* Boundary, alignment and unmapped-bus failures must be rejected safely. */
    assert(cubex_memory_read8(&mem, CUBEX360_MAIN_RAM_SIZE, &byte_value) != 0);
    assert(cubex_memory_read16be(&mem, 1u, &half_value) != 0);
    assert(cubex_memory_read32be(&mem, 2u, &value) != 0);
    assert(cubex_memory_write8(&mem, CUBEX360_MAIN_RAM_SIZE, 0x5au) != 0);
    assert(cubex_memory_write16be(&mem, 1u, 0x1234u) != 0);
    assert(cubex_memory_write32be(&mem, 2u, 0x12345678u) != 0);
    assert(cubex_bus_read32be(&mem, 0x70000000u, &value) != 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE, 0x3860002au) == 0);
    assert(cubex_bus_read32be(&mem, CUBEX360_RAM_UNCACHED_BASE, &value) == 0);
    assert(value == 0x3860002au);
    /* Bounded bulk transfers support future DMA and save-state paths. */
    {
        const uint8_t source[5] = {0x10u, 0x20u, 0x30u, 0x40u, 0x50u};
        uint8_t destination[5] = {0u, 0u, 0u, 0u, 0u};
        assert(cubex_memory_copy_to(&mem, 0x2200u, source, sizeof(source)) == 0);
        assert(cubex_memory_copy_from(&mem, 0x2200u, destination, sizeof(destination)) == 0);
        assert(memcmp(source, destination, sizeof(source)) == 0);
        assert(cubex_memory_copy_to(&mem, CUBEX360_MAIN_RAM_SIZE - 2u, source, sizeof(source)) != 0);
        assert(cubex_memory_copy_from(&mem, CUBEX360_MAIN_RAM_SIZE - 2u, destination, sizeof(destination)) != 0);
        assert(cubex_memory_copy_to(&mem, 0u, NULL, 1u) != 0);
        assert(cubex_memory_copy_from(&mem, 0u, NULL, 1u) != 0);
        assert(cubex_memory_copy_to(&mem, CUBEX360_MAIN_RAM_SIZE, NULL, 0u) == 0);
        assert(cubex_memory_copy_from(&mem, CUBEX360_MAIN_RAM_SIZE, NULL, 0u) == 0);
    }

    /* Overlapping bulk copies must behave like memmove in both directions. */
    {
        uint8_t expected[8] = {1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u};
        uint8_t actual[8] = {0u};
        assert(cubex_memory_copy_to(&mem, 0x2300u, expected, sizeof(expected)) == 0);
        assert(cubex_memory_copy_to(&mem, 0x2302u, mem.main_ram + 0x2300u, 6u) == 0);
        assert(cubex_memory_copy_from(&mem, 0x2300u, actual, sizeof(actual)) == 0);
        assert(actual[0] == 1u && actual[1] == 2u);
        assert(actual[2] == 1u && actual[3] == 2u && actual[4] == 3u);
        assert(actual[5] == 4u && actual[6] == 5u && actual[7] == 6u);
        assert(cubex_memory_copy_to(&mem, 0x2300u, expected, sizeof(expected)) == 0);
        assert(cubex_memory_copy_from(&mem, 0x2302u, mem.main_ram + 0x2300u, 6u) == 0);
        assert(cubex_memory_copy_from(&mem, 0x2300u, actual, sizeof(actual)) == 0);
        assert(actual[0] == 3u && actual[1] == 4u && actual[2] == 5u);
        assert(actual[3] == 6u && actual[4] == 7u && actual[5] == 8u);
    }

    /* Bulk bus transfers translate physical, cached and uncached RAM aliases. */
    {
        const uint8_t payload[4] = {0xdeu, 0xadu, 0xbeu, 0xefu};
        uint8_t readback[4] = {0u, 0u, 0u, 0u};
        assert(cubex_bus_copy_to(&mem, CUBEX360_RAM_CACHED_BASE + 0x2400u,
                                 payload, sizeof(payload)) == 0);
        assert(cubex_bus_copy_from(&mem, CUBEX360_RAM_UNCACHED_BASE + 0x2400u,
                                   readback, sizeof(readback)) == 0);
        assert(memcmp(payload, readback, sizeof(payload)) == 0);
        assert(cubex_bus_copy_from(&mem, 0x2400u, readback, sizeof(readback)) == 0);
        assert(memcmp(payload, readback, sizeof(payload)) == 0);
        assert(cubex_bus_copy_to(&mem, CUBEX360_RAM_CACHED_BASE +
                                 CUBEX360_MAIN_RAM_SIZE - 2u,
                                 payload, sizeof(payload)) != 0);
        assert(cubex_bus_copy_from(&mem, 0x70000000u, readback, sizeof(readback)) != 0);
        assert(cubex_bus_copy_to(&mem, 0x70000000u, NULL, 0u) == 0);
    }

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
    /* MFCR reads all CR fields; MTCRF updates only fields selected by FXM. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x26cu,
        (31u<<26)|(8u<<21)|(19u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x270u,
        (31u<<26)|(9u<<21)|(0x81u<<12)|(144u<<1)) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x26cu);
    cpu.cr = 0x12345678u;
    cpu.gpr[9] = 0xabcdef01u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[8] == 0x12345678u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.cr == 0xa2345671u);
    /* CR logical operations use CR bit indices, not whole-field masks. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x280u,
        (19u<<26)|(3u<<21)|(1u<<16)|(2u<<11)|(33u<<1)) == 0); /* CRNOR */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x284u,
        (19u<<26)|(4u<<21)|(1u<<16)|(2u<<11)|(129u<<1)) == 0); /* CRANDC */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x288u,
        (19u<<26)|(5u<<21)|(1u<<16)|(2u<<11)|(193u<<1)) == 0); /* CRXOR */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x28cu,
        (19u<<26)|(6u<<21)|(1u<<16)|(2u<<11)|(225u<<1)) == 0); /* CRNAND */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x290u,
        (19u<<26)|(7u<<21)|(1u<<16)|(2u<<11)|(257u<<1)) == 0); /* CRAND */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x294u,
        (19u<<26)|(8u<<21)|(1u<<16)|(2u<<11)|(289u<<1)) == 0); /* CREQV */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x298u,
        (19u<<26)|(9u<<21)|(1u<<16)|(2u<<11)|(417u<<1)) == 0); /* CRORC */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x29cu,
        (19u<<26)|(10u<<21)|(1u<<16)|(2u<<11)|(449u<<1)) == 0); /* CROR */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x280u);
    cpu.cr = 0x40000000u; /* BI=1 is one; BI=2 is zero. */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert((cpu.cr & 0x0ff00000u) == 0x0e600000u);
    /* MCRXR transfers XER SO/OV/CA into a selected CR field, then clears them. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2a0u,
        (31u<<26)|(2u<<23)|(512u<<1)) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2a0u);
    cpu.xer = 0xe0000000u;
    cpu.cr = 0x12345678u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert((cpu.cr & 0x00f00000u) == 0x00e00000u);
    assert((cpu.xer & 0xe0000000u) == 0u);
    /* ADDC sets XER.CA on carry-out; SUBFC sets CA on no-borrow. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x310u,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(10u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x314u,
        (31u<<26)|(6u<<21)|(7u<<16)|(8u<<11)|(8u<<1)) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x318u,
        (31u<<26)|(10u<<21)|(11u<<16)|(12u<<11)|(8u<<1)) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x310u);
    cpu.gpr[4] = 0xffffffffu; cpu.gpr[5] = 1u;
    cpu.gpr[7] = 3u; cpu.gpr[8] = 5u;
    cpu.gpr[11] = 3u; cpu.gpr[12] = 2u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0u && (cpu.xer & 0x20000000u) != 0u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 2u && (cpu.xer & 0x20000000u) != 0u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[10] == 0xffffffffu && (cpu.xer & 0x20000000u) == 0u);
    /* Carry-chain operations consume XER.CA and write the next carry state. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x340u,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(138u<<1)) == 0); /* adde r3,r4,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x344u,
        (31u<<26)|(6u<<21)|(7u<<16)|(8u<<11)|(136u<<1)) == 0); /* subfe r6,r7,r8 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x348u,
        (31u<<26)|(9u<<21)|(10u<<16)|(234u<<1)) == 0); /* addme r9,r10 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x34cu,
        (31u<<26)|(11u<<21)|(12u<<16)|(200u<<1)) == 0); /* subfze r11,r12 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x340u);
    cpu.gpr[4] = 0xffffffffu; cpu.gpr[5] = 0u; cpu.xer = 0x20000000u;
    cpu.gpr[7] = 3u; cpu.gpr[8] = 5u;
    cpu.gpr[10] = 0u; cpu.gpr[12] = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0u && (cpu.xer & 0x20000000u) != 0u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 2u && (cpu.xer & 0x20000000u) != 0u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[9] == 0u && (cpu.xer & 0x20000000u) != 0u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[11] == 0u && (cpu.xer & 0x20000000u) != 0u);
    /* ADDZE and SUBFME extend the carry-chain instruction family. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x360u,
        (31u<<26)|(3u<<21)|(4u<<16)|(202u<<1)) == 0); /* addze r3,r4 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x364u,
        (31u<<26)|(5u<<21)|(6u<<16)|(232u<<1)) == 0); /* subfme r5,r6 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x360u);
    cpu.gpr[4] = 0xffffffffu;
    cpu.gpr[6] = 0u;
    cpu.xer = 0x20000000u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0u && (cpu.xer & 0x20000000u) != 0u);
    cpu.xer = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[5] == 0xfffffffeu && (cpu.xer & 0x20000000u) != 0u);
    /* ADDME: RA + 0xffffffff + CA; check carry-in set and clear. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x368u,
        (31u<<26)|(7u<<21)|(8u<<16)|(234u<<1)) == 0); /* addme r7,r8 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x368u);
    cpu.gpr[8] = 0u;
    cpu.xer = 0x20000000u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[7] == 0u && (cpu.xer & 0x20000000u) != 0u);
    cpu.xer = 0u;
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x368u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[7] == 0xffffffffu && (cpu.xer & 0x20000000u) == 0u);
    /* ADDO/SUBFO set OV on signed overflow and keep SO sticky. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x380u,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(778u<<1)) == 0); /* addo r3,r4,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x384u,
        (31u<<26)|(6u<<21)|(7u<<16)|(8u<<11)|(552u<<1)|1u) == 0); /* subfo. r6,r7,r8 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x388u,
        (31u<<26)|(9u<<21)|(10u<<16)|(11u<<11)|(778u<<1)) == 0); /* addo r9,r10,r11 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x38cu,
        (31u<<26)|(12u<<21)|(10u<<16)|(11u<<11)|(778u<<1)|1u) == 0); /* addo. r12,r10,r11 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x380u);
    cpu.gpr[4] = 0x7fffffffu; cpu.gpr[5] = 1u;
    cpu.gpr[7] = 1u; cpu.gpr[8] = 0x80000000u;
    cpu.gpr[10] = 1u; cpu.gpr[11] = 2u;
    cpu.xer = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0x80000000u);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 0x7fffffffu);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u); /* subtraction also overflowed */
    assert((cpu.cr & 0xf0000000u) == 0x50000000u); /* positive result plus sticky SO */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[9] == 3u);
    assert((cpu.xer & 0xc0000000u) == 0x80000000u); /* OV clears, SO stays set */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[12] == 3u);
    assert((cpu.cr & 0xf0000000u) == 0x50000000u); /* Rc includes sticky SO */
    /* MULLW returns the low word of the signed product; Rc updates CR0. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x390u,
        (31u<<26)|(13u<<21)|(14u<<16)|(15u<<11)|(235u<<1)) == 0); /* mullw r13,r14,r15 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x394u,
        (31u<<26)|(16u<<21)|(17u<<16)|(18u<<11)|(235u<<1)|1u) == 0); /* mullw. r16,r17,r18 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x390u);
    cpu.gpr[14] = 0xfffffffeu; cpu.gpr[15] = 3u;
    cpu.gpr[17] = 0x80000000u; cpu.gpr[18] = 2u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[13] == 0xfffffffau);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[16] == 0u);
    assert((cpu.cr & 0xf0000000u) == 0x20000000u); /* zero result, no SO */
    /* SRAW uses the register shift count and updates XER.CA from discarded bits. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x398u,
        (31u<<26)|(19u<<21)|(23u<<16)|(20u<<11)|(792u<<1)|1u) == 0); /* sraw. r23,r19,r20 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x39cu,
        (31u<<26)|(21u<<21)|(24u<<16)|(22u<<11)|(792u<<1)|1u) == 0); /* sraw. r24,r21,r22 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x398u);
    cpu.gpr[19] = 0xfffffff9u; cpu.gpr[20] = 2u; /* -7 >> 2 = -2; discarded one */
    cpu.gpr[21] = 7u; cpu.gpr[22] = 1u; /* 7 >> 1 = 3; no discarded one */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[23] == 0xfffffffeu);
    assert((cpu.xer & 0x20000000u) != 0u);
    assert((cpu.cr & 0xf0000000u) == 0x80000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[24] == 3u);
    assert((cpu.xer & 0x20000000u) == 0u);
    assert((cpu.cr & 0xf0000000u) == 0x40000000u);
    /* MULHWU returns the upper word of the unsigned product. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3a0u,
        (31u<<26)|(29u<<21)|(30u<<16)|(31u<<11)|(11u<<1)|1u) == 0); /* mulhwu. r29,r30,r31 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3a4u,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(11u<<1)|1u) == 0); /* mulhwu. r3,r4,r5 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3a0u);
    cpu.gpr[30] = 0xffffffffu; cpu.gpr[31] = 2u;
    cpu.gpr[4] = 0x7fffffffu; cpu.gpr[5] = 1u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[29] == 1u);
    assert((cpu.cr & 0xf0000000u) == 0x40000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0u);
    assert((cpu.cr & 0xf0000000u) == 0x20000000u);
    /* MULHW returns the upper word of the signed product. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3a8u,
        (31u<<26)|(29u<<21)|(30u<<16)|(31u<<11)|(75u<<1)|1u) == 0); /* mulhw. r29,r30,r31 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3acu,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(75u<<1)|1u) == 0); /* mulhw. r3,r4,r5 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3a8u);
    cpu.gpr[30] = 0xfffffffeu; cpu.gpr[31] = 3u;
    cpu.gpr[4] = 0x7fffffffu; cpu.gpr[5] = 2u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[29] == 0xffffffffu);
    assert((cpu.cr & 0xf0000000u) == 0x80000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0u);
    assert((cpu.cr & 0xf0000000u) == 0x20000000u);
    /* MULLWO writes the low product word and updates OV/SO on signed overflow. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3b0u,
        (31u<<26)|(29u<<21)|(30u<<16)|(31u<<11)|(747u<<1)|1u) == 0); /* mullwo. r29,r30,r31 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3b4u,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(747u<<1)|1u) == 0); /* mullwo. r3,r4,r5 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3b0u);
    cpu.gpr[30] = 0x80000000u; cpu.gpr[31] = 0xffffffffu; /* INT_MIN * -1 */
    cpu.gpr[4] = 2u; cpu.gpr[5] = 3u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[29] == 0x80000000u);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u);
    assert((cpu.cr & 0xf0000000u) == 0x90000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 6u);
    assert((cpu.xer & 0xc0000000u) == 0x80000000u); /* OV clears; SO sticky */
    assert((cpu.cr & 0xf0000000u) == 0x50000000u);
    /* DIVWU and DIVWUO: quotient, zero divisor, OV and sticky SO. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3b8u,
        (31u<<26)|(29u<<21)|(30u<<16)|(31u<<11)|(459u<<1)|1u) == 0); /* divwu. r29,r30,r31 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3bcu,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(971u<<1)|1u) == 0); /* divwuo. r3,r4,r5 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3b8u);
    cpu.gpr[30] = 100u; cpu.gpr[31] = 4u;
    cpu.gpr[4] = 123u; cpu.gpr[5] = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[29] == 25u);
    assert((cpu.cr & 0xf0000000u) == 0x40000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0u);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u);
    assert((cpu.cr & 0xf0000000u) == 0x30000000u);
    /* NEG and NEGO: signed negation, overflow on INT_MIN, sticky SO. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3c0u,
        (31u<<26)|(30u<<21)|(29u<<16)|(104u<<1)) == 0); /* neg r29,r30 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3c4u,
        (31u<<26)|(4u<<21)|(3u<<16)|(616u<<1)|1u) == 0); /* nego. r3,r4 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3c8u,
        (31u<<26)|(7u<<21)|(6u<<16)|(616u<<1)|1u) == 0); /* nego. r6,r7 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3c0u);
    cpu.gpr[30] = 5u; cpu.gpr[4] = 0x80000000u; cpu.gpr[7] = 1u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[29] == 0xfffffffbu);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0x80000000u);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u);
    assert((cpu.cr & 0xf0000000u) == 0x90000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 0xffffffffu);
    assert((cpu.xer & 0xc0000000u) == 0x80000000u); /* OV clears, SO stays sticky */
    assert((cpu.cr & 0xf0000000u) == 0x90000000u); /* negative result plus SO */
    /* DIVW and DIVWO: signed quotient, divide-by-zero and INT_MIN / -1. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3d0u,
        (31u<<26)|(29u<<21)|(30u<<16)|(31u<<11)|(491u<<1)|1u) == 0); /* divw. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3d4u,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(1003u<<1)|1u) == 0); /* divwo. */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3d0u);
    cpu.gpr[30] = (uint32_t)-21; cpu.gpr[31] = 4u;
    cpu.gpr[4] = 0x80000000u; cpu.gpr[5] = 0xffffffffu;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[29] == (uint32_t)-5);
    assert((cpu.cr & 0xf0000000u) == 0x80000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0u);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u);
    assert((cpu.cr & 0xf0000000u) == 0x30000000u);
    /* DIVWO zero divisor must avoid C undefined behavior and set OV/SO. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3d8u,
        (31u<<26)|(8u<<21)|(9u<<16)|(10u<<11)|(1003u<<1)|1u) == 0); /* divwo. r8,r9,r10 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3d8u);
    cpu.gpr[9] = 77u; cpu.gpr[10] = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[8] == 0u);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u);
    assert((cpu.cr & 0xf0000000u) == 0x30000000u);
    /* ADDZEO updates carry, OV and sticky SO, including Rc/CR0. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3e0u,
        (31u<<26)|(8u<<21)|(9u<<16)|(10u<<11)|(714u<<1)|1u) == 0); /* addzeo. r8,r9 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3e4u,
        (31u<<26)|(11u<<21)|(12u<<16)|(13u<<11)|(714u<<1)|1u) == 0); /* addzeo. r11,r12 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3e0u);
    cpu.gpr[9] = 0x7fffffffu;
    cpu.xer = 0x20000000u; /* CA=1 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[8] == 0x80000000u);
    assert((cpu.xer & 0xe0000000u) == 0xc0000000u); /* OV and SO set; CA cleared */
    assert((cpu.cr & 0xf0000000u) == 0x90000000u);
    cpu.gpr[12] = 40u;
    cpu.xer &= ~0x20000000u; /* CA=0 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[11] == 40u);
    assert((cpu.xer & 0xe0000000u) == 0x80000000u); /* OV clears, SO remains */
    assert((cpu.cr & 0xf0000000u) == 0x50000000u);
    /* SUBFIC sets CA only when the subtraction has no unsigned borrow. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3e8u,
        (8u<<26)|(14u<<21)|(15u<<16)|10u) == 0); /* subfic r14,r15,10 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3ecu,
        (8u<<26)|(16u<<21)|(17u<<16)|20u) == 0); /* subfic r16,r17,20 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3e8u);
    cpu.gpr[15] = 15u;
    cpu.gpr[17] = 5u;
    cpu.xer = 0x80000000u; /* preserve sticky SO */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[14] == 0xfffffffbu); /* 10 - 15 = -5 */
    assert((cpu.xer & 0xa0000000u) == 0x80000000u); /* borrow clears CA; SO stays */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[16] == 15u); /* 20 - 5 = 15 */
    assert((cpu.xer & 0xa0000000u) == 0xa0000000u); /* no borrow sets CA */

    /* SUBFZE consumes XER.CA, updates CA, and optionally records CR0. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3f0u,
        (31u<<26)|(21u<<21)|(22u<<16)|(200u<<1)|1u) == 0); /* subfze. r21,r22 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3f4u,
        (31u<<26)|(23u<<21)|(24u<<16)|(200u<<1)|1u) == 0); /* subfze. r23,r24 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3f0u);
    cpu.gpr[22] = 0u;
    cpu.xer = 0x20000000u; /* CA=1: 0 - 0 - 0 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[21] == 0u);
    assert((cpu.xer & 0x20000000u) != 0u);
    assert((cpu.cr & 0xf0000000u) == 0x20000000u); /* EQ */
    cpu.gpr[24] = 0u;
    cpu.xer &= ~0x20000000u; /* CA=0: 0 - 0 - 1 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[23] == 0xffffffffu);
    assert((cpu.xer & 0x20000000u) == 0u);
    assert((cpu.cr & 0xf0000000u) == 0x80000000u); /* LT, SO is not set in this case */

    /* SUBFZE edge: RA=0xffffffff with CA=0 wraps to zero but still borrows. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x3f8u,
        (31u<<26)|(25u<<21)|(26u<<16)|(200u<<1)|1u) == 0); /* subfze. r25,r26 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x3f8u);
    cpu.gpr[26] = 0xffffffffu;
    cpu.xer &= ~0x20000000u; /* CA=0 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[25] == 0u);
    assert((cpu.xer & 0x20000000u) == 0u); /* borrow clears CA */
    assert((cpu.cr & 0xf0000000u) == 0x20000000u); /* EQ, SO clear */

    /* SUBFME computes ~RA + 0xffffffff + CA and updates carry/CR0. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x400u,
        (31u<<26)|(27u<<21)|(28u<<16)|(232u<<1)|1u) == 0); /* subfme. r27,r28 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x404u,
        (31u<<26)|(29u<<21)|(30u<<16)|(232u<<1)|1u) == 0); /* subfme. r29,r30 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x400u);
    cpu.gpr[28] = 0u;
    cpu.xer = 0x20000000u; /* CA=1 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[27] == 0xffffffffu); /* ~0 + 0xffffffff + CA(1), modulo 2^32 */
    assert((cpu.xer & 0x20000000u) != 0u); /* carry out */
    assert((cpu.cr & 0xf0000000u) == 0x80000000u); /* LT */
    cpu.gpr[30] = 1u;
    cpu.xer &= ~0x20000000u; /* CA=0 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[29] == 0xfffffffdu); /* ~1 + 0xffffffff */
    assert((cpu.xer & 0x20000000u) != 0u); /* carry out remains set */
    assert((cpu.cr & 0xf0000000u) == 0x80000000u); /* LT */

    /* ADDE Rc: consume CA, clear carry-out, preserve SO, and update CR0. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x408u,
        (31u<<26)|(13u<<21)|(14u<<16)|(15u<<11)|(138u<<1)|1u) == 0); /* adde. r13,r14,r15 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x408u);
    cpu.gpr[14] = 0x7fffffffu;
    cpu.gpr[15] = 0u;
    cpu.xer = 0xa0000000u; /* SO=1, CA=1 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[13] == 0x80000000u);
    assert((cpu.xer & 0xa0000000u) == 0x80000000u); /* SO preserved, CA cleared */
    assert((cpu.cr & 0xf0000000u) == 0x90000000u); /* LT | SO */

    /* SLW/SRW: register shift counts at 1 and >=32, including Rc. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x410u,
        (31u<<26)|(4u<<21)|(3u<<16)|(5u<<11)|(24u<<1)|1u) == 0); /* slw. r3,r4,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x414u,
        (31u<<26)|(7u<<21)|(6u<<16)|(8u<<11)|(536u<<1)|1u) == 0); /* srw. r6,r7,r8 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x410u);
    cpu.gpr[4] = 0x80000001u;
    cpu.gpr[5] = 1u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 2u);
    assert((cpu.cr & 0xf0000000u) == 0x40000000u); /* GT */
    cpu.gpr[5] = 32u; /* PowerPC word shift with bit 5 set yields zero. */
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x410u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0u);
    assert((cpu.cr & 0xf0000000u) == 0x20000000u); /* EQ */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x414u);
    cpu.gpr[7] = 0x80000001u;
    cpu.gpr[8] = 1u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 0x40000000u);
    assert((cpu.cr & 0xf0000000u) == 0x40000000u); /* GT */
    cpu.gpr[8] = 32u;
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x414u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 0u);
    assert((cpu.cr & 0xf0000000u) == 0x20000000u); /* EQ */

    /* MCRXR: transfer SO/OV/CA into the selected CR field and clear them. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x40cu,
        (31u<<26)|(2u<<23)|(512u<<1)) == 0); /* mcrxr cr2 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x40cu);
    cpu.cr = 0x12345678u;
    cpu.xer = 0xe0000000u; /* SO=1, OV=1, CA=1 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.cr == ((0x12345678u & ~0x00f00000u) | 0x00e00000u));
    assert((cpu.xer & 0xe0000000u) == 0u);

    /* SRAW boundary: counts >=32 sign-fill and update XER.CA from discarded ones. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2a4u,
        (31u<<26)|(4u<<21)|(3u<<16)|(5u<<11)|(792u<<1)|1u) == 0); /* sraw. r3,r4,r5 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2a4u);
    cpu.gpr[4] = 0x80000000u;
    cpu.gpr[5] = 32u;
    cpu.xer = 0x20000000u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0xffffffffu);
    assert((cpu.xer & 0x20000000u) == 0u); /* sign bit alone does not set CA */
    assert((cpu.cr & 0xf0000000u) == 0x80000000u); /* LT */
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2a4u;
    cpu.gpr[4] = 0x80000001u;
    cpu.gpr[5] = 32u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0xffffffffu);
    assert((cpu.xer & 0x20000000u) != 0u); /* lower one bits shifted out set CA */
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2a4u;
    cpu.gpr[4] = 0x7fffffffu;
    cpu.gpr[5] = 33u;
    cpu.xer = 0x20000000u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0u);
    assert((cpu.xer & 0x20000000u) == 0u);
    assert((cpu.cr & 0xf0000000u) == 0x20000000u); /* EQ */

    /* ADDO: set OV/SO on signed overflow, then clear OV while SO stays sticky. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2a8u,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(778u<<1)|1u) == 0); /* addo. r3,r4,r5 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2a8u);
    cpu.gpr[4] = 0x7fffffffu;
    cpu.gpr[5] = 1u;
    cpu.xer = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0x80000000u);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u); /* OV | SO */
    assert((cpu.cr & 0xf0000000u) == 0x90000000u); /* LT | SO */
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2a8u;
    cpu.gpr[4] = 1u;
    cpu.gpr[5] = 1u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 2u);
    assert((cpu.xer & 0xc0000000u) == 0x80000000u); /* OV clears; SO remains */
    assert((cpu.cr & 0xf0000000u) == 0x50000000u); /* GT | SO */

    /* SUBFO: RB - RA, signed overflow updates OV and sticky SO; Rc updates CR0. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2acu,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(552u<<1)|1u) == 0); /* subfo. r3,r4,r5 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2acu);
    cpu.gpr[4] = 0xffffffffu; /* -1 */
    cpu.gpr[5] = 0x7fffffffu;
    cpu.xer = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0x80000000u);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u); /* OV | SO */
    assert((cpu.cr & 0xf0000000u) == 0x90000000u); /* LT | SO */
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2acu;
    cpu.gpr[4] = 3u;
    cpu.gpr[5] = 5u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 2u);
    assert((cpu.xer & 0xc0000000u) == 0x80000000u); /* OV clears; SO remains sticky */
    assert((cpu.cr & 0xf0000000u) == 0x50000000u); /* GT | SO */

    /* OE variants: ADDEO, SUBFCO and SUBFEO update OV and sticky SO. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2b0u,
        (31u<<26)|(3u<<21)|(4u<<16)|(5u<<11)|(650u<<1)|1u) == 0); /* addeo. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2b4u,
        (31u<<26)|(6u<<21)|(7u<<16)|(8u<<11)|(520u<<1)|1u) == 0); /* subfco. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2b8u,
        (31u<<26)|(9u<<21)|(10u<<16)|(11u<<11)|(648u<<1)|1u) == 0); /* subfeo. */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2b0u);
    cpu.gpr[4] = 0x7fffffffu;
    cpu.gpr[5] = 0u;
    cpu.xer = 0x20000000u; /* CA=1 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 0x80000000u);
    assert((cpu.xer & 0xe0000000u) == 0xc0000000u); /* OV=1, SO=1, CA=0 */
    assert((cpu.cr & 0xf0000000u) == 0x90000000u); /* LT | SO */

    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2b4u;
    cpu.gpr[7] = 1u;
    cpu.gpr[8] = 0x80000000u;
    cpu.xer = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 0x7fffffffu);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u); /* OV | SO */
    assert((cpu.cr & 0xf0000000u) == 0x50000000u); /* GT | SO */

    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2b8u;
    cpu.gpr[10] = 1u;
    cpu.gpr[11] = 0x80000000u;
    cpu.xer = 0u; /* CA=0, effective operation is RB - RA - 1 */
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[9] == 0x7ffffffeu);
    assert((cpu.xer & 0xc0000000u) == 0xc0000000u); /* OV | SO */
    assert((cpu.cr & 0xf0000000u) == 0x50000000u); /* GT | SO */

    /* DCBZ clears the aligned 32-byte cache block through cached RAM alias. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2bcu,
        (31u<<26)|(4u<<16)|(5u<<11)|(1014u<<1)) == 0); /* dcbz r4,r5 */
    {
        uint32_t i;
        uint8_t byte;
        for (i = 0u; i < 32u; ++i)
            assert(cubex_bus_write8(&mem, CUBEX360_RAM_CACHED_BASE + 0x520u + i,
                                    (uint8_t)(i + 1u)) == 0);
        cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2bcu);
        cpu.gpr[4] = CUBEX360_RAM_CACHED_BASE + 0x500u;
        cpu.gpr[5] = 0x23u; /* effective 0x523; block aligns down to 0x520 */
        assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
        for (i = 0u; i < 32u; ++i) {
            assert(cubex_bus_read8(&mem, CUBEX360_RAM_CACHED_BASE + 0x520u + i, &byte) == 0);
            assert(byte == 0u);
        }
    }
    /* DCBZ must fault for a non-RAM effective address. */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2bcu);
    cpu.gpr[4] = 0x70000000u;
    cpu.gpr[5] = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_MEMORY_FAULT);

    /* Update-form loads/stores: effective address and RA update are both checked. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2c0u,
        (33u<<26)|(4u<<21)|(3u<<16)|4u) == 0); /* lwzu r4,4(r3) */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2c4u,
        (37u<<26)|(5u<<21)|(3u<<16)|8u) == 0); /* stwu r5,8(r3) */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2c8u,
        (35u<<26)|(6u<<21)|(3u<<16)|1u) == 0); /* lbzu r6,1(r3) */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2ccu,
        (39u<<26)|(7u<<21)|(3u<<16)|2u) == 0); /* stbu r7,2(r3) */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2c0u);
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE + 0x600u;
    cpu.gpr[5] = 0x12345678u;
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x604u, 0xaabbccddu) == 0);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[4] == 0xaabbccddu && cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0x604u);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2c4u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0x60cu);
    {
        uint32_t word;
        assert(cubex_bus_read32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x60cu, &word) == 0);
        assert(word == 0x12345678u);
    }
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2c8u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 0x34u && cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0x60du);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2ccu;
    cpu.gpr[7] = 0x55u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0x60fu);
    {
        uint8_t byte;
        assert(cubex_bus_read8(&mem, CUBEX360_RAM_CACHED_BASE + 0x60fu, &byte) == 0);
        assert(byte == 0x55u);
    }

    /* LMW/STMW transfer RT..r31 in big-endian words and validate bounds. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2d0u,
        (46u<<26)|(28u<<21)|(3u<<16)|0x700u) == 0); /* lmw r28,0x700(r3) */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2d4u,
        (47u<<26)|(29u<<21)|(3u<<16)|0x720u) == 0); /* stmw r29,0x720(r3) */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x700u, 0x11223344u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x704u, 0x55667788u) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x708u, 0x99aabbccu) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x70cu, 0xddeeff00u) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2d0u);
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[28] == 0x11223344u && cpu.gpr[29] == 0x55667788u);
    assert(cpu.gpr[30] == 0x99aabbccu && cpu.gpr[31] == 0xddeeff00u);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2d4u;
    cpu.gpr[29] = 0x01020304u;
    cpu.gpr[30] = 0x05060708u;
    cpu.gpr[31] = 0x090a0b0cu;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    {
        uint32_t word;
        assert(cubex_bus_read32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x720u, &word) == 0);
        assert(word == 0x01020304u);
        assert(cubex_bus_read32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x724u, &word) == 0);
        assert(word == 0x05060708u);
        assert(cubex_bus_read32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x728u, &word) == 0);
        assert(word == 0x090a0b0cu);
    }
    /* Misaligned LMW faults without changing destination registers. */
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2d0u;
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE + 1u;
    cpu.gpr[28] = 0xdeadbeefu;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_MEMORY_FAULT);
    assert(cpu.gpr[28] == 0xdeadbeefu);

    /* Indexed update forms calculate RA+RB and update RA after success. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2e0u,
        (31u<<26)|(4u<<21)|(3u<<16)|(5u<<11)|(55u<<1)) == 0); /* lwzux r4,r3,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2e4u,
        (31u<<26)|(6u<<21)|(3u<<16)|(5u<<11)|(183u<<1)) == 0); /* stwux r6,r3,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2e8u,
        (31u<<26)|(7u<<21)|(3u<<16)|(5u<<11)|(119u<<1)) == 0); /* lbzux r7,r3,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2ecu,
        (31u<<26)|(8u<<21)|(3u<<16)|(5u<<11)|(247u<<1)) == 0); /* stbux r8,r3,r5 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2e0u);
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE + 0x800u;
    cpu.gpr[5] = 0x10u;
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x810u, 0x1234abcdu) == 0);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[4] == 0x1234abcdu && cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0x810u);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2e4u;
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE + 0x800u;
    cpu.gpr[6] = 0xa1b2c3d4u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    {
        uint32_t value;
        assert(cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0x810u);
        assert(cubex_bus_read32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x810u, &value) == 0);
        assert(value == 0xa1b2c3d4u);
    }
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2e8u;
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE + 0x810u;
    cpu.gpr[5] = 0u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[7] == 0xa1u && cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0x810u);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2ecu;
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE + 0x820u;
    cpu.gpr[5] = 0u;
    cpu.gpr[8] = 0x5au;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    {
        uint8_t value;
        assert(cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0x820u);
        assert(cubex_bus_read8(&mem, CUBEX360_RAM_CACHED_BASE + 0x820u, &value) == 0);
        assert(value == 0x5au);
    }

    /* Indexed non-update memory instructions use RA+RB without modifying RA. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2f0u,
        (31u<<26)|(4u<<21)|(3u<<16)|(5u<<11)|(23u<<1)) == 0); /* lwzx r4,r3,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2f4u,
        (31u<<26)|(6u<<21)|(3u<<16)|(5u<<11)|(87u<<1)) == 0); /* lbzx r6,r3,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2f8u,
        (31u<<26)|(7u<<21)|(3u<<16)|(5u<<11)|(151u<<1)) == 0); /* stwx r7,r3,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x2fcu,
        (31u<<26)|(8u<<21)|(3u<<16)|(5u<<11)|(215u<<1)) == 0); /* stbx r8,r3,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x300u,
        (31u<<26)|(9u<<21)|(3u<<16)|(5u<<11)|(279u<<1)) == 0); /* lhzx r9,r3,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x304u,
        (31u<<26)|(10u<<21)|(3u<<16)|(5u<<11)|(343u<<1)) == 0); /* lhax r10,r3,r5 */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x308u,
        (31u<<26)|(11u<<21)|(3u<<16)|(5u<<11)|(407u<<1)) == 0); /* sthx r11,r3,r5 */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x2f0u);
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE + 0x900u;
    cpu.gpr[5] = 4u;
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x904u, 0x80ff1234u) == 0);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[4] == 0x80ff1234u && cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0x900u);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2f4u;
    cpu.gpr[5] = 4u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[6] == 0x80u);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2f8u;
    cpu.gpr[5] = 8u;
    cpu.gpr[7] = 0x12345678u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    {
        uint32_t value;
        assert(cubex_bus_read32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x908u, &value) == 0);
        assert(value == 0x12345678u);
    }
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x2fcu;
    cpu.gpr[5] = 9u;
    cpu.gpr[8] = 0xaau;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    {
        uint8_t value;
        assert(cubex_bus_read8(&mem, CUBEX360_RAM_CACHED_BASE + 0x909u, &value) == 0);
        assert(value == 0xaau);
    }
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x300u;
    cpu.gpr[5] = 4u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[9] == 0x80ffu);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x304u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[10] == 0xffff80ffu);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x308u;
    cpu.gpr[5] = 12u;
    cpu.gpr[11] = 0xbeefu;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    {
        uint16_t value;
        assert(cubex_bus_read16be(&mem, CUBEX360_RAM_CACHED_BASE + 0x90cu, &value) == 0);
        assert(value == 0xbeefu);
    }

    /* Indexed halfword update forms update RA only after a successful access. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x310u,
        (31u<<26)|(12u<<21)|(3u<<16)|(5u<<11)|(311u<<1)) == 0); /* lhzux */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x314u,
        (31u<<26)|(13u<<21)|(3u<<16)|(5u<<11)|(375u<<1)) == 0); /* lhaux */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x318u,
        (31u<<26)|(14u<<21)|(3u<<16)|(5u<<11)|(439u<<1)) == 0); /* sthux */
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x310u);
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE + 0xa00u;
    cpu.gpr[5] = 4u;
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0xa04u, 0x80ff1234u) == 0);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[12] == 0x80ffu && cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0xa04u);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x314u;
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE + 0xa00u;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[13] == 0xffff80ffu && cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0xa04u);
    cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x318u;
    cpu.gpr[3] = CUBEX360_RAM_CACHED_BASE + 0xa00u;
    cpu.gpr[14] = 0xbeefu;
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == CUBEX360_RAM_CACHED_BASE + 0xa04u);
    {
        uint16_t value;
        assert(cubex_bus_read16be(&mem, CUBEX360_RAM_CACHED_BASE + 0xa04u, &value) == 0);
        assert(value == 0xbeefu);
    }

    /* Bounded execution loop runs exactly the requested instruction budget. */
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x300u, 0x3860002au) == 0);
    assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x304u, 0x38830001u) == 0);
    cubex_cpu_reset(&cpu, CUBEX360_RAM_CACHED_BASE + 0x300u);
    assert(cubex_cpu_run(&cpu, &mem, 2u) == CUBEX_CPU_OK);
    assert(cpu.gpr[3] == 42u && cpu.gpr[4] == 43u);
    assert(cpu.pc == CUBEX360_RAM_CACHED_BASE + 0x308u);
    assert(cpu.retired_instructions == 2u);
    /* Fetch from an unmapped address faults without retiring an instruction. */
    cubex_cpu_reset(&cpu, 0x70000000u);
    assert(cubex_cpu_step(&cpu, &mem) == CUBEX_CPU_MEMORY_FAULT);
    assert(cpu.pc == 0x70000000u && cpu.retired_instructions == 0u);
    /* Save/load round-trip covers CPU registers and main RAM; file is host-test-only. */
    {
        const char *state_path = "/tmp/cubex360-state-test.bin";
        uint32_t before;
        cpu.gpr[3] = 0x13579bdfu;
        cpu.pc = CUBEX360_RAM_CACHED_BASE + 0x400u;
        cpu.lr = 0x12345678u;
        cpu.cr = 0xa5a5a5a5u;
        cpu.xer = 0x000000e0u;
        assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x1000u, 0xc001d00du) == 0);
        assert(cubex_state_save(state_path, &cpu, &mem) == 0);
        cpu.gpr[3] = 0u; cpu.pc = 0u; cpu.lr = 0u; cpu.cr = 0u; cpu.xer = 0u;
        assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x1000u, 0u) == 0);
        assert(cubex_state_load(state_path, &cpu, &mem) == 0);
        assert(cpu.gpr[3] == 0x13579bdfu && cpu.pc == CUBEX360_RAM_CACHED_BASE + 0x400u);
        assert(cpu.lr == 0x12345678u && cpu.cr == 0xa5a5a5a5u && cpu.xer == 0xe0u);
        assert(cubex_bus_read32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x1000u, &before) == 0);
        assert(before == 0xc001d00du);
        /* Corrupt a RAM byte: checksum failure must not partially restore state. */
        {
            FILE *state_file = fopen(state_path, "r+b");
            int byte;
            assert(state_file != NULL);
            assert(fseek(state_file, 20L + 160L + 0x1000L, SEEK_SET) == 0);
            byte = fgetc(state_file);
            assert(byte != EOF);
            assert(fseek(state_file, -1L, SEEK_CUR) == 0);
            assert(fputc(byte ^ 0xff, state_file) != EOF);
            assert(fclose(state_file) == 0);
        }
        cpu.gpr[3] = 0x2468ace0u;
        assert(cubex_bus_write32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x1000u, 0xdeadbeefu) == 0);
        assert(cubex_state_load(state_path, &cpu, &mem) != 0);
        assert(cpu.gpr[3] == 0x2468ace0u);
        assert(cubex_bus_read32be(&mem, CUBEX360_RAM_CACHED_BASE + 0x1000u, &before) == 0);
        assert(before == 0xdeadbeefu);
        remove(state_path);
    }
    cubex_memory_destroy(&mem);
    puts("CubEX360 core host smoke: PASS");
    return 0;
}
