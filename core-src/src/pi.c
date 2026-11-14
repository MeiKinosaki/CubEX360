#include "cubex360/pi.h"
#include <string.h>
#define PI_IMPLEMENTED_IRQ_MASK 0x00003fffu
void cubex_pi_init(cubex_pi_t *pi) { if (pi) memset(pi, 0, sizeof(*pi)); }
static int pi_offset(uint32_t address, uint32_t *offset) {
    if (!offset || address < CUBEX360_PI_BASE ||
        address - CUBEX360_PI_BASE >= CUBEX360_PI_SIZE ||
        ((address - CUBEX360_PI_BASE) & 3u)) return -1;
    *offset = address - CUBEX360_PI_BASE; return 0;
}
int cubex_pi_read32(const cubex_pi_t *pi, uint32_t address, uint32_t *value) {
    uint32_t off;
    if (!pi || !value || pi_offset(address, &off)) return -1;
    if (off == 0u) { *value = pi->interrupt_cause; return 0; }
    if (off == 4u) { *value = pi->interrupt_mask; return 0; }
    return -1;
}
int cubex_pi_write32(cubex_pi_t *pi, uint32_t address, uint32_t value) {
    uint32_t off;
    if (!pi || pi_offset(address, &off)) return -1;
    if (off == 0u) { pi->interrupt_cause &= ~(value & PI_IMPLEMENTED_IRQ_MASK); return 0; } /* W1C */
    if (off == 4u) { pi->interrupt_mask = value & PI_IMPLEMENTED_IRQ_MASK; return 0; }
    return -1;
}
void cubex_pi_raise(cubex_pi_t *pi, uint32_t causes) {
    if (pi) pi->interrupt_cause |= causes & PI_IMPLEMENTED_IRQ_MASK;
}
void cubex_pi_clear(cubex_pi_t *pi, uint32_t causes) {
    if (pi) pi->interrupt_cause &= ~(causes & PI_IMPLEMENTED_IRQ_MASK);
}
int cubex_pi_irq_pending(const cubex_pi_t *pi) {
    return pi && ((pi->interrupt_cause & pi->interrupt_mask) != 0u);
}
