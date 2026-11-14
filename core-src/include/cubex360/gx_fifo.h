#ifndef CUBEX360_GX_FIFO_H
#define CUBEX360_GX_FIFO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CUBEX_GX_FIFO_CAPACITY 256u

typedef struct cubex_gx_fifo {
    uint32_t words[CUBEX_GX_FIFO_CAPACITY];
    uint16_t read_index;
    uint16_t write_index;
    uint16_t count;
    uint32_t overflow;
} cubex_gx_fifo_t;

void cubex_gx_fifo_reset(cubex_gx_fifo_t *fifo);
/* Returns 0 on enqueue, -1 for invalid input, -2 when full. */
int cubex_gx_fifo_write(cubex_gx_fifo_t *fifo, uint32_t word);
/* Returns 0 on dequeue, -1 for invalid input, -2 when empty. */
int cubex_gx_fifo_read(cubex_gx_fifo_t *fifo, uint32_t *word);
size_t cubex_gx_fifo_count(const cubex_gx_fifo_t *fifo);
int cubex_gx_fifo_is_full(const cubex_gx_fifo_t *fifo);

#ifdef __cplusplus
}
#endif
#endif
