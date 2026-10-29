#ifndef CUBEX360_MMIO_H
#define CUBEX360_MMIO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal deterministic 32-bit MMIO register bank for device prototypes.
 * Device-specific side effects belong in the owning device module. */
typedef struct cubex_mmio {
    uint32_t base_address;
    uint32_t *registers;
    size_t register_count;
} cubex_mmio_t;

int cubex_mmio_init(cubex_mmio_t *mmio, uint32_t base_address,
                    uint32_t *register_storage, size_t register_count);
int cubex_mmio_read32(const cubex_mmio_t *mmio, uint32_t address,
                      uint32_t *value);
int cubex_mmio_write32(cubex_mmio_t *mmio, uint32_t address,
                       uint32_t value);

#ifdef __cplusplus
}
#endif
#endif
