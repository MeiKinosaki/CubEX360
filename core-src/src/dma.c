#include "cubex360/dma.h"
#include "cubex360/bus.h"

#include <string.h>

int cubex_dma_copy_main_ram(cubex_memory_t *memory, uint32_t source_address,
                            uint32_t destination_address, size_t size) {
    uint32_t source_offset;
    uint32_t destination_offset;

    if (!memory || !memory->main_ram || memory->main_ram_size == 0u) return -1;
    if (size == 0u) return 0;
    if (cubex_bus_translate_main_ram(source_address, &source_offset) != 0 ||
        cubex_bus_translate_main_ram(destination_address, &destination_offset) != 0) {
        return -1;
    }
    if (size > memory->main_ram_size ||
        (size_t)source_offset > memory->main_ram_size - size ||
        (size_t)destination_offset > memory->main_ram_size - size) {
        return -1;
    }

    /* memmove gives deterministic behavior for overlapping RAM ranges/aliases. */
    memmove(memory->main_ram + destination_offset,
            memory->main_ram + source_offset, size);
    return 0;
}
