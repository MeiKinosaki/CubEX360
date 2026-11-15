#include "cubex360/gx_fifo.h"
#include <string.h>

void cubex_gx_fifo_reset(cubex_gx_fifo_t *fifo) {
    if (!fifo) return;
    memset(fifo, 0, sizeof(*fifo));
}

int cubex_gx_fifo_write(cubex_gx_fifo_t *fifo, uint32_t word) {
    if (!fifo) return -1;
    if (fifo->count >= CUBEX_GX_FIFO_CAPACITY) {
        fifo->overflow = 1u;
        return -2;
    }
    fifo->words[fifo->write_index] = word;
    fifo->write_index = (uint16_t)((fifo->write_index + 1u) %
                                   CUBEX_GX_FIFO_CAPACITY);
    fifo->count++;
    if (fifo->count > fifo->high_water) fifo->high_water = fifo->count;
    return 0;
}

int cubex_gx_fifo_read(cubex_gx_fifo_t *fifo, uint32_t *word) {
    if (!fifo || !word) return -1;
    if (fifo->count == 0u) { fifo->underflow = 1u; return -2; }
    *word = fifo->words[fifo->read_index];
    fifo->read_index = (uint16_t)((fifo->read_index + 1u) %
                                  CUBEX_GX_FIFO_CAPACITY);
    fifo->count--;
    return 0;
}

size_t cubex_gx_fifo_count(const cubex_gx_fifo_t *fifo) {
    return fifo ? fifo->count : 0u;
}

int cubex_gx_fifo_is_full(const cubex_gx_fifo_t *fifo) {
    return fifo && fifo->count == CUBEX_GX_FIFO_CAPACITY;
}

uint16_t cubex_gx_fifo_high_water(const cubex_gx_fifo_t *fifo) {
    return fifo ? fifo->high_water : 0u;
}

uint32_t cubex_gx_fifo_error_flags(const cubex_gx_fifo_t *fifo) {
    uint32_t flags = 0u;
    if (!fifo) return 0u;
    if (fifo->overflow) flags |= CUBEX_GX_FIFO_ERROR_OVERFLOW;
    if (fifo->underflow) flags |= CUBEX_GX_FIFO_ERROR_UNDERFLOW;
    return flags;
}

void cubex_gx_fifo_clear_errors(cubex_gx_fifo_t *fifo, uint32_t mask) {
    if (!fifo) return;
    if (mask & CUBEX_GX_FIFO_ERROR_OVERFLOW) fifo->overflow = 0u;
    if (mask & CUBEX_GX_FIFO_ERROR_UNDERFLOW) fifo->underflow = 0u;
}
