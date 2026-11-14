#include "cubex360/gx_fifo.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    cubex_gx_fifo_t fifo;
    uint32_t value;
    unsigned i;

    cubex_gx_fifo_reset(&fifo);
    assert(cubex_gx_fifo_count(&fifo) == 0u);
    assert(cubex_gx_fifo_read(&fifo, &value) == -2);
    assert(cubex_gx_fifo_write(&fifo, 0x12345678u) == 0);
    assert(cubex_gx_fifo_write(&fifo, 0xabcdef01u) == 0);
    assert(cubex_gx_fifo_read(&fifo, &value) == 0 && value == 0x12345678u);
    assert(cubex_gx_fifo_read(&fifo, &value) == 0 && value == 0xabcdef01u);

    for (i = 0; i < CUBEX_GX_FIFO_CAPACITY; ++i)
        assert(cubex_gx_fifo_write(&fifo, i ^ 0x55aa55aau) == 0);
    assert(cubex_gx_fifo_is_full(&fifo));
    assert(cubex_gx_fifo_write(&fifo, 0u) == -2);
    assert(fifo.overflow == 1u);
    for (i = 0; i < CUBEX_GX_FIFO_CAPACITY; ++i) {
        assert(cubex_gx_fifo_read(&fifo, &value) == 0);
        assert(value == (i ^ 0x55aa55aau));
    }
    assert(cubex_gx_fifo_count(&fifo) == 0u);

    /* Exercise circular wrap while preserving FIFO order. */
    for (i = 0; i < 200u; ++i)
        assert(cubex_gx_fifo_write(&fifo, i) == 0);
    for (i = 0; i < 150u; ++i) {
        assert(cubex_gx_fifo_read(&fifo, &value) == 0);
        assert(value == i);
    }
    for (i = 200u; i < 350u; ++i)
        assert(cubex_gx_fifo_write(&fifo, i) == 0);
    for (i = 150u; i < 350u; ++i) {
        assert(cubex_gx_fifo_read(&fifo, &value) == 0);
        assert(value == i);
    }
    assert(cubex_gx_fifo_read(&fifo, &value) == -2);
    assert(cubex_gx_fifo_write(0, 1u) == -1);
    assert(cubex_gx_fifo_read(&fifo, 0) == -1);

    puts("GX FIFO bounded queue tests passed");
    return 0;
}
