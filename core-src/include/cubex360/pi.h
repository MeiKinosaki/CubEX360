#ifndef CUBEX360_PI_H
#define CUBEX360_PI_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define CUBEX360_PI_BASE 0xCC003000u
#define CUBEX360_PI_SIZE 0x100u
#define CUBEX_PI_IRQ_CP (1u << 11)
#define CUBEX_PI_IRQ_PE_TOKEN (1u << 9)
#define CUBEX_PI_IRQ_PE_FINISH (1u << 10)
typedef struct cubex_pi { uint32_t interrupt_cause; uint32_t interrupt_mask; } cubex_pi_t;
void cubex_pi_init(cubex_pi_t *pi);
int cubex_pi_read32(const cubex_pi_t *pi, uint32_t address, uint32_t *value);
int cubex_pi_write32(cubex_pi_t *pi, uint32_t address, uint32_t value);
void cubex_pi_raise(cubex_pi_t *pi, uint32_t causes);
void cubex_pi_clear(cubex_pi_t *pi, uint32_t causes);
int cubex_pi_irq_pending(const cubex_pi_t *pi);
#ifdef __cplusplus
}
#endif
#endif
