#include "cubex360/flipper_map.h"

#include <assert.h>
#include <stdint.h>

static void expect_region(uint32_t address, cubex_flipper_region_t region,
                          uint32_t offset) {
    cubex_flipper_decoded_address_t decoded;
    assert(cubex_flipper_decode_address(address, &decoded) == 0);
    assert(decoded.region == region);
    assert(decoded.offset == offset);
}

int main(void) {
    cubex_flipper_decoded_address_t decoded;
    expect_region(0xCC000000u, CUBEX_FLIPPER_REGION_CP, 0u);
    expect_region(0xCC00007Cu, CUBEX_FLIPPER_REGION_CP, 0x7Cu);
    expect_region(0xCC001000u, CUBEX_FLIPPER_REGION_PE, 0u);
    expect_region(0xCC002000u, CUBEX_FLIPPER_REGION_VI, 0u);
    expect_region(0xCC003000u, CUBEX_FLIPPER_REGION_PI, 0u);
    expect_region(0xCC004000u, CUBEX_FLIPPER_REGION_MI, 0u);
    expect_region(0xCC005000u, CUBEX_FLIPPER_REGION_DSP, 0u);
    expect_region(0xCC006000u, CUBEX_FLIPPER_REGION_DI, 0u);
    expect_region(0xCC006400u, CUBEX_FLIPPER_REGION_SI, 0u);
    expect_region(0xCC006800u, CUBEX_FLIPPER_REGION_EXI, 0u);
    expect_region(0xCC006C00u, CUBEX_FLIPPER_REGION_AI, 0u);
    expect_region(0xCC008000u, CUBEX_FLIPPER_REGION_GX_FIFO, 0u);
    expect_region(0xCC008003u, CUBEX_FLIPPER_REGION_GX_FIFO, 3u);

    assert(cubex_flipper_decode_address(0xCC000080u, &decoded) != 0);
    assert(cubex_flipper_decode_address(0xCC000FFFu, &decoded) != 0);
    assert(cubex_flipper_decode_address(0xCC0063FFu, &decoded) != 0);
    assert(cubex_flipper_decode_address(0xCC006C20u, &decoded) != 0);
    assert(cubex_flipper_decode_address(0xCC008004u, &decoded) != 0);
    assert(cubex_flipper_decode_address(0xCC00FFFFu, &decoded) != 0);
    assert(cubex_flipper_decode_address(0x80000000u, &decoded) != 0);
    assert(cubex_flipper_decode_address(0xCC000000u, 0) != 0);
    return 0;
}
