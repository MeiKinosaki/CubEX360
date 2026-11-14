#ifndef CUBEX360_CP_H
#define CUBEX360_CP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CUBEX360_CP_BASE 0xCC000000u
#define CUBEX360_CP_SIZE 0x80u

#define CUBEX_CP_SR_BP       0x0010u
#define CUBEX_CP_SR_GP_IDLE  0x0008u
#define CUBEX_CP_SR_READ_IDLE 0x0004u
#define CUBEX_CP_SR_UNDERFLOW 0x0002u
#define CUBEX_CP_SR_OVERFLOW  0x0001u
#define CUBEX_CP_SR_STATUS_MASK 0x001fu

#define CUBEX_CP_CR_BP_ENABLE       0x0020u
#define CUBEX_CP_CR_LINK_ENABLE     0x0010u
#define CUBEX_CP_CR_UNDERFLOW_IRQ   0x0008u
#define CUBEX_CP_CR_OVERFLOW_IRQ    0x0004u
#define CUBEX_CP_CR_IRQ_ENABLE      0x0002u
#define CUBEX_CP_CR_FIFO_READ_ENABLE 0x0001u
#define CUBEX_CP_CR_MASK 0x003fu

typedef struct cubex_cp {
    uint16_t status;
    uint16_t control;
    uint16_t token;
    uint16_t registers[32];
} cubex_cp_t;

void cubex_cp_init(cubex_cp_t *cp);
int cubex_cp_read16(const cubex_cp_t *cp, uint32_t address, uint16_t *value);
int cubex_cp_write16(cubex_cp_t *cp, uint32_t address, uint16_t value);
/* Device events update status independently of CPU register writes. */
void cubex_cp_set_status(cubex_cp_t *cp, uint16_t set_bits,
                         uint16_t clear_bits);
int cubex_cp_irq_pending(const cubex_cp_t *cp);

#ifdef __cplusplus
}
#endif
#endif
