#include "cubex360/dma.h"
#include "cubex360/bus.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

int main(void) {
    cubex_memory_t memory;
    uint8_t source[8] = {0x10u, 0x21u, 0x32u, 0x43u,
                         0x54u, 0x65u, 0x76u, 0x87u};
    uint8_t actual[8] = {0u};
    uint8_t sentinel = 0xA5u;

    assert(cubex_memory_init(&memory) == 0);

    /* Empty transfer is a no-op, including at the end boundary. */
    assert(cubex_dma_copy_main_ram(&memory, CUBEX360_MAIN_RAM_SIZE,
                                   CUBEX360_MAIN_RAM_SIZE, 0u) == 0);
    assert(cubex_dma_copy_main_ram(&memory, 0u, 0u, 0u) == 0);

    /* Last valid byte and exact end-of-RAM range. */
    assert(cubex_memory_copy_to(&memory, CUBEX360_MAIN_RAM_SIZE - 1u,
                                &sentinel, 1u) == 0);
    assert(cubex_dma_copy_main_ram(&memory, CUBEX360_MAIN_RAM_SIZE - 1u,
                                   CUBEX360_MAIN_RAM_SIZE - 2u, 1u) == 0);
    assert(cubex_memory_read8(&memory, CUBEX360_MAIN_RAM_SIZE - 2u,
                              &actual[0]) == 0);
    assert(actual[0] == sentinel);
    assert(cubex_dma_copy_main_ram(&memory, CUBEX360_MAIN_RAM_SIZE - 1u,
                                   CUBEX360_MAIN_RAM_SIZE - 2u, 2u) != 0);

    /* Exact in-range payload through cached/uncached aliases. */
    assert(cubex_memory_copy_to(&memory, 0x3000u, source, sizeof(source)) == 0);
    assert(cubex_dma_copy_main_ram(&memory,
                                   CUBEX360_RAM_CACHED_BASE + 0x3000u,
                                   CUBEX360_RAM_UNCACHED_BASE + 0x4000u,
                                   sizeof(source)) == 0);
    assert(cubex_memory_copy_from(&memory, 0x4000u, actual, sizeof(actual)) == 0);
    assert(memcmp(source, actual, sizeof(source)) == 0);

    /* Invalid source/destination must not partially modify destination. */
    assert(cubex_memory_copy_to(&memory, 0x5000u, &sentinel, 1u) == 0);
    assert(cubex_dma_copy_main_ram(&memory, 0x70000000u, 0x5000u, 4u) != 0);
    assert(cubex_memory_read8(&memory, 0x5000u, &actual[0]) == 0);
    assert(actual[0] == sentinel);
    assert(cubex_dma_copy_main_ram(&memory, 0x3000u, 0x70000000u, 4u) != 0);
    assert(cubex_memory_read8(&memory, 0x5000u, &actual[0]) == 0);
    assert(actual[0] == sentinel);

    /* Overlap in either direction must have memmove semantics across aliases. */
    assert(cubex_memory_copy_to(&memory, 0x6000u, source, sizeof(source)) == 0);
    assert(cubex_dma_copy_main_ram(&memory, 0x6000u,
                                   CUBEX360_RAM_CACHED_BASE + 0x6002u, 6u) == 0);
    assert(cubex_memory_copy_from(&memory, 0x6000u, actual, sizeof(actual)) == 0);
    assert(actual[0] == 0x10u && actual[1] == 0x21u &&
           actual[2] == 0x10u && actual[3] == 0x21u &&
           actual[4] == 0x32u && actual[5] == 0x43u);

    assert(cubex_memory_copy_to(&memory, 0x6000u, source, sizeof(source)) == 0);
    assert(cubex_dma_copy_main_ram(&memory,
                                   CUBEX360_RAM_UNCACHED_BASE + 0x6002u,
                                   0x6000u, 6u) == 0);
    assert(cubex_memory_copy_from(&memory, 0x6000u, actual, sizeof(actual)) == 0);
    assert(actual[0] == 0x32u && actual[1] == 0x43u &&
           actual[2] == 0x54u && actual[3] == 0x65u &&
           actual[4] == 0x76u && actual[5] == 0x87u);

    assert(cubex_dma_copy_main_ram(NULL, 0u, 0u, 0u) != 0);
    assert(cubex_dma_copy_main_ram(&memory, 0u, 0u, (size_t)-1) != 0);

    cubex_memory_destroy(&memory);
    return 0;
}
