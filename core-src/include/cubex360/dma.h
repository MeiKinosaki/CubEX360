#ifndef CUBEX360_DMA_H
#define CUBEX360_DMA_H

#include <stddef.h>
#include <stdint.h>
#include "cubex360/memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Bounded synchronous RAM-to-RAM transfer prototype; not a hardware channel model. */
int cubex_dma_copy_main_ram(cubex_memory_t *memory, uint32_t source_address,
                            uint32_t destination_address, size_t size);

#ifdef __cplusplus
}
#endif
#endif
