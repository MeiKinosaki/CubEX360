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
    return 0;
}

int cubex_gx_fifo_read(cubex_gx_fifo_t *fifo, uint32_t *word) {
    if (!fifo || !word) return -1;
    if (fifo->count == 0u) return -2;
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
