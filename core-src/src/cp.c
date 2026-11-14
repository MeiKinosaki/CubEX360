#include "cubex360/cp.h"

#include <string.h>

void cubex_cp_init(cubex_cp_t *cp) {
    if (!cp) return;
    memset(cp, 0, sizeof(*cp));
    cp->status = CUBEX_CP_SR_GP_IDLE | CUBEX_CP_SR_READ_IDLE;
}

static int cp_offset(uint32_t address, uint32_t *offset) {
    if (!offset || address < CUBEX360_CP_BASE ||
        address - CUBEX360_CP_BASE >= CUBEX360_CP_SIZE ||
        ((address - CUBEX360_CP_BASE) & 1u) != 0u) return -1;
    *offset = address - CUBEX360_CP_BASE;
    return 0;
}

int cubex_cp_read16(const cubex_cp_t *cp, uint32_t address, uint16_t *value) {
    uint32_t offset;
    if (!cp || !value || cp_offset(address, &offset) != 0) return -1;
    switch (offset) {
    case 0x00u: *value = cp->status; return 0;
    case 0x02u: *value = cp->control; return 0;
    case 0x04u: return -1; /* write-only clear register */
    case 0x0eu: *value = cp->token; return 0;
    default:
        if (offset >= 0x10u && offset <= 0x16u) {
            *value = cp->registers[offset / 2u];
            return 0;
        }
        if (offset >= 0x20u && offset <= 0x3eu) {
            *value = cp->registers[offset / 2u];
            return 0;
        }
        return -1;
    }
}

int cubex_cp_write16(cubex_cp_t *cp, uint32_t address, uint16_t value) {
    uint32_t offset;
    if (!cp || cp_offset(address, &offset) != 0) return -1;
    switch (offset) {
    case 0x00u:
        /* Status bits are hardware-owned; writes cannot fabricate events. */
        return 0;
    case 0x02u:
        cp->control = (uint16_t)(value & CUBEX_CP_CR_MASK);
        return 0;
    case 0x04u:
        cp->status = (uint16_t)(cp->status &
            (uint16_t)~(value & (CUBEX_CP_SR_UNDERFLOW |
                                 CUBEX_CP_SR_OVERFLOW)));
        return 0;
    case 0x0eu:
        cp->token = value;
        return 0;
    default:
        if (offset >= 0x10u && offset <= 0x16u) {
            cp->registers[offset / 2u] = value;
            return 0;
        }
        if (offset >= 0x20u && offset <= 0x3eu) {
            cp->registers[offset / 2u] = value;
            return 0;
        }
        return -1;
    }
}

void cubex_cp_set_status(cubex_cp_t *cp, uint16_t set_bits,
                         uint16_t clear_bits) {
    if (!cp) return;
    cp->status = (uint16_t)((cp->status |
        (set_bits & CUBEX_CP_SR_STATUS_MASK)) &
        (uint16_t)~(clear_bits & CUBEX_CP_SR_STATUS_MASK));
}
