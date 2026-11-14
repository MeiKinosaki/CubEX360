#ifndef CUBEX360_FLIPPER_MAP_H
#define CUBEX360_FLIPPER_MAP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Documented GameCube Flipper MMIO windows. Decoding a window does not
 * imply that its device registers or side effects are implemented. */
typedef enum cubex_flipper_region {
    CUBEX_FLIPPER_REGION_CP = 0,
    CUBEX_FLIPPER_REGION_PE,
    CUBEX_FLIPPER_REGION_VI,
    CUBEX_FLIPPER_REGION_PI,
    CUBEX_FLIPPER_REGION_MI,
    CUBEX_FLIPPER_REGION_DSP,
    CUBEX_FLIPPER_REGION_DI,
    CUBEX_FLIPPER_REGION_SI,
    CUBEX_FLIPPER_REGION_EXI,
    CUBEX_FLIPPER_REGION_AI,
    CUBEX_FLIPPER_REGION_GX_FIFO
} cubex_flipper_region_t;

typedef struct cubex_flipper_decoded_address {
    cubex_flipper_region_t region;
    uint32_t offset;
} cubex_flipper_decoded_address_t;

/* Returns 0 for a documented device aperture, -1 for holes/invalid input.
 * The FIFO is a write-gather port at one word address, not a register bank. */
int cubex_flipper_decode_address(uint32_t address,
                                 cubex_flipper_decoded_address_t *decoded);

#ifdef __cplusplus
}
#endif
#endif
