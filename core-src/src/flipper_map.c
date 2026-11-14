#include "cubex360/flipper_map.h"

typedef struct flipper_window {
    uint32_t base;
    uint32_t size;
    cubex_flipper_region_t region;
} flipper_window_t;

/* Apertures are conservative documented block windows, not a claim that
 * every offset is a readable/writable register. Register-level semantics
 * and access widths are implemented by each device model in later stages. */
static const flipper_window_t windows[] = {
    {0xCC000000u, 0x00000080u, CUBEX_FLIPPER_REGION_CP},
    {0xCC001000u, 0x00000100u, CUBEX_FLIPPER_REGION_PE},
    {0xCC002000u, 0x00000100u, CUBEX_FLIPPER_REGION_VI},
    {0xCC003000u, 0x00000100u, CUBEX_FLIPPER_REGION_PI},
    {0xCC004000u, 0x00000100u, CUBEX_FLIPPER_REGION_MI},
    {0xCC005000u, 0x00000200u, CUBEX_FLIPPER_REGION_DSP},
    {0xCC006000u, 0x00000040u, CUBEX_FLIPPER_REGION_DI},
    {0xCC006400u, 0x00000100u, CUBEX_FLIPPER_REGION_SI},
    {0xCC006800u, 0x00000040u, CUBEX_FLIPPER_REGION_EXI},
    {0xCC006C00u, 0x00000020u, CUBEX_FLIPPER_REGION_AI},
    {0xCC008000u, 0x00000004u, CUBEX_FLIPPER_REGION_GX_FIFO}
};

int cubex_flipper_decode_address(uint32_t address,
                                 cubex_flipper_decoded_address_t *decoded) {
    unsigned i;
    if (!decoded) return -1;
    for (i = 0; i < sizeof(windows) / sizeof(windows[0]); ++i) {
        const flipper_window_t *window = &windows[i];
        if (address >= window->base &&
            address - window->base < window->size) {
            decoded->region = window->region;
            decoded->offset = address - window->base;
            return 0;
        }
    }
    return -1;
}
