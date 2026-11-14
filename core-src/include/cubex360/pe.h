#ifndef CUBEX360_PE_H
#define CUBEX360_PE_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define CUBEX360_PE_BASE 0xCC001000u
#define CUBEX360_PE_SIZE 0x100u
#define CUBEX_PE_INT_FINISH 0x0008u
#define CUBEX_PE_INT_TOKEN 0x0004u
#define CUBEX_PE_INT_FINISH_ENABLE 0x0002u
#define CUBEX_PE_INT_TOKEN_ENABLE 0x0001u
#define CUBEX_PE_INT_PENDING_MASK 0x000cu
#define CUBEX_PE_INT_ENABLE_MASK 0x0003u
typedef struct cubex_pe {
    uint16_t registers[8];
    uint16_t interrupt_status;
    uint16_t token;
} cubex_pe_t;
void cubex_pe_init(cubex_pe_t *pe);
int cubex_pe_read16(const cubex_pe_t *pe, uint32_t address, uint16_t *value);
int cubex_pe_write16(cubex_pe_t *pe, uint32_t address, uint16_t value);
void cubex_pe_raise_finish(cubex_pe_t *pe);
void cubex_pe_raise_token(cubex_pe_t *pe, uint16_t token);
int cubex_pe_finish_irq(const cubex_pe_t *pe);
int cubex_pe_token_irq(const cubex_pe_t *pe);
#ifdef __cplusplus
}
#endif
#endif
