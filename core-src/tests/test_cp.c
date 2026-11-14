#include "cubex360/cp.h"

#include <assert.h>
#include <stdint.h>

int main(void) {
    cubex_cp_t cp;
    uint16_t value = 0u;

    cubex_cp_init(&cp);
    assert(cubex_cp_read16(&cp, 0xCC000000u, &value) == 0);
    assert(value == (CUBEX_CP_SR_GP_IDLE | CUBEX_CP_SR_READ_IDLE));

    assert(cubex_cp_write16(&cp, 0xCC000002u, 0xffffu) == 0);
    assert(cubex_cp_read16(&cp, 0xCC000002u, &value) == 0);
    assert(value == CUBEX_CP_CR_MASK);

    cubex_cp_set_status(&cp, CUBEX_CP_SR_OVERFLOW |
        CUBEX_CP_SR_UNDERFLOW | CUBEX_CP_SR_BP, 0u);
    assert(cubex_cp_read16(&cp, 0xCC000000u, &value) == 0);
    assert((value & (CUBEX_CP_SR_OVERFLOW | CUBEX_CP_SR_UNDERFLOW |
                     CUBEX_CP_SR_BP)) ==
                   (CUBEX_CP_SR_OVERFLOW | CUBEX_CP_SR_UNDERFLOW |
                    CUBEX_CP_SR_BP));

    assert(cubex_cp_write16(&cp, 0xCC000004u,
        CUBEX_CP_SR_OVERFLOW) == 0);
    assert(cubex_cp_read16(&cp, 0xCC000000u, &value) == 0);
    assert((value & CUBEX_CP_SR_OVERFLOW) == 0u);
    assert((value & CUBEX_CP_SR_UNDERFLOW) != 0u);
    assert((value & CUBEX_CP_SR_BP) != 0u);

    assert(cubex_cp_write16(&cp, 0xCC00000Eu, 0x1234u) == 0);
    assert(cubex_cp_read16(&cp, 0xCC00000Eu, &value) == 0);
    assert(value == 0x1234u);

    assert(cubex_cp_write16(&cp, 0xCC000020u, 0x5678u) == 0);
    assert(cubex_cp_read16(&cp, 0xCC000020u, &value) == 0);
    assert(value == 0x5678u);

    assert(cubex_cp_read16(&cp, 0xCC000004u, &value) != 0);
    assert(cubex_cp_write16(&cp, 0xCC000006u, 0u) != 0);
    assert(cubex_cp_read16(&cp, 0xCC000001u, &value) != 0);
    assert(cubex_cp_read16(&cp, 0xCC000080u, &value) != 0);
    assert(cubex_cp_read16(&cp, 0xCC000000u, 0) != 0);
    return 0;
}
