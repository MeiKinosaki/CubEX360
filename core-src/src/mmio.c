#include "cubex360/mmio.h"

#include <string.h>

int cubex_mmio_init(cubex_mmio_t *mmio, uint32_t base_address,
                    uint32_t *register_storage, size_t register_count) {
    if (!mmio || !register_storage || register_count == 0u ||
        (base_address & 3u) != 0u ||
        register_count > SIZE_MAX / sizeof(*register_storage) ||
        register_count > (size_t)((UINT32_MAX - base_address) / 4u) + 1u) {
        return -1;
    }
    mmio->base_address = base_address;
    mmio->registers = register_storage;
    mmio->register_count = register_count;
    memset(register_storage, 0, register_count * sizeof(*register_storage));
    return 0;
}

static int register_index(const cubex_mmio_t *mmio, uint32_t address,
                          size_t *index) {
    uint32_t offset;
    if (!mmio || !mmio->registers || !index ||
        (address & 3u) != 0u || address < mmio->base_address) {
        return -1;
    }
    offset = address - mmio->base_address;
    if ((size_t)(offset / 4u) >= mmio->register_count) return -1;
    *index = (size_t)(offset / 4u);
    return 0;
}

int cubex_mmio_read32(const cubex_mmio_t *mmio, uint32_t address,
                      uint32_t *value) {
    size_t index;
    if (!value || register_index(mmio, address, &index) != 0) return -1;
    *value = mmio->registers[index];
    return 0;
}

int cubex_mmio_write32(cubex_mmio_t *mmio, uint32_t address,
                       uint32_t value) {
    size_t index;
    if (register_index(mmio, address, &index) != 0) return -1;
    mmio->registers[index] = value;
    return 0;
}
