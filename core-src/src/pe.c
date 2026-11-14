#include "cubex360/pe.h"
#include <string.h>
void cubex_pe_init(cubex_pe_t *pe) { if (pe) memset(pe, 0, sizeof(*pe)); }
static int pe_offset(uint32_t address, uint32_t *offset) {
    if (!offset || address < CUBEX360_PE_BASE ||
        address - CUBEX360_PE_BASE >= CUBEX360_PE_SIZE ||
        ((address - CUBEX360_PE_BASE) & 1u)) return -1;
    *offset = address - CUBEX360_PE_BASE;
    return 0;
}
int cubex_pe_read16(const cubex_pe_t *pe, uint32_t address, uint16_t *value) {
    uint32_t off;
    if (!pe || !value || pe_offset(address, &off)) return -1;
    switch (off) {
    case 0x00: case 0x02: case 0x04: case 0x06: case 0x08:
        *value = pe->registers[off / 2u]; return 0;
    case 0x0a: *value = pe->interrupt_status; return 0;
    case 0x0e: *value = pe->token; return 0;
    default: return -1;
    }
}
int cubex_pe_write16(cubex_pe_t *pe, uint32_t address, uint16_t value) {
    uint32_t off;
    if (!pe || pe_offset(address, &off)) return -1;
    switch (off) {
    case 0x00: case 0x02: case 0x04: case 0x06: case 0x08:
        pe->registers[off / 2u] = value; return 0;
    case 0x0a:
        pe->interrupt_status = (uint16_t)(
            (pe->interrupt_status & (uint16_t)~(value & CUBEX_PE_INT_PENDING_MASK)) |
            (value & CUBEX_PE_INT_ENABLE_MASK));
        return 0;
    case 0x0e:
        pe->token = value; return 0;
    default: return -1;
    }
}
void cubex_pe_raise_finish(cubex_pe_t *pe) {
    if (pe) pe->interrupt_status |= CUBEX_PE_INT_FINISH;
}
void cubex_pe_raise_token(cubex_pe_t *pe, uint16_t token) {
    if (!pe) return;
    pe->token = token;
    pe->interrupt_status |= CUBEX_PE_INT_TOKEN;
}
int cubex_pe_finish_irq(const cubex_pe_t *pe) {
    return pe && (pe->interrupt_status & CUBEX_PE_INT_FINISH) &&
           (pe->interrupt_status & CUBEX_PE_INT_FINISH_ENABLE);
}
int cubex_pe_token_irq(const cubex_pe_t *pe) {
    return pe && (pe->interrupt_status & CUBEX_PE_INT_TOKEN) &&
           (pe->interrupt_status & CUBEX_PE_INT_TOKEN_ENABLE);
}
