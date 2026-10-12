#include "cubex360/cpu.h"
#include <string.h>

#define PPC_CR0_LT 0x80000000u
#define PPC_CR0_GT 0x40000000u
#define PPC_CR0_EQ 0x20000000u
#define PPC_CR0_SO 0x10000000u

void cubex_cpu_reset(cubex_cpu_state_t *c, uint32_t pc) {
    if (!c) return;
    memset(c, 0, sizeof(*c));
    c->pc = pc;
}
static int32_t sx16(uint32_t v) { return (int16_t)(v & 0xffffu); }
static void set_cr0(cubex_cpu_state_t *c, uint32_t value) {
    uint32_t field = ((int32_t)value < 0) ? PPC_CR0_LT :
                     (value == 0u) ? PPC_CR0_EQ : PPC_CR0_GT;
    if (c->xer & 0x80000000u) field |= PPC_CR0_SO;
    c->cr = (c->cr & 0x0fffffffu) | field;
}

cubex_cpu_result_t cubex_cpu_step(cubex_cpu_state_t *c, cubex_memory_t *m) {
    uint32_t ins, op, rt, ra, imm, next;
    if (!c || !m) return CUBEX_CPU_BAD_ARGUMENT;
    if (cubex_bus_read32be(m, c->pc, &ins)) return CUBEX_CPU_MEMORY_FAULT;
    op = ins >> 26;
    rt = (ins >> 21) & 31u;
    ra = (ins >> 16) & 31u;
    imm = ins & 0xffffu;
    next = c->pc + 4u;
    switch (op) {
    case 14: c->gpr[rt] = (ra ? c->gpr[ra] : 0u) + (uint32_t)sx16(imm); break;
    case 15: c->gpr[rt] = (ra ? c->gpr[ra] : 0u) + ((uint32_t)sx16(imm) << 16); break;
    case 24: c->gpr[rt] = c->gpr[ra] | imm; break;
    case 25: c->gpr[rt] = c->gpr[ra] | (imm << 16); break;
    case 26: c->gpr[rt] = c->gpr[ra] ^ imm; break;
    case 28:
        c->gpr[rt] = c->gpr[ra] & imm;
        set_cr0(c, c->gpr[rt]);
        break;
    case 29:
        c->gpr[rt] = c->gpr[ra] & (imm << 16);
        set_cr0(c, c->gpr[rt]);
        break;
    case 32: {
        uint32_t a = (ra ? c->gpr[ra] : 0u) + (uint32_t)sx16(imm);
        if (cubex_bus_read32be(m, a, &c->gpr[rt])) return CUBEX_CPU_MEMORY_FAULT;
        break;
    }
    case 36: {
        uint32_t a = (ra ? c->gpr[ra] : 0u) + (uint32_t)sx16(imm);
        if (cubex_bus_write32be(m, a, c->gpr[rt])) return CUBEX_CPU_MEMORY_FAULT;
        break;
    }
    case 18: {
        int32_t d = (int32_t)(ins & 0x03fffffcu);
        if (d & 0x02000000) d |= (int32_t)0xfc000000u;
        next = (ins & 2u) ? (uint32_t)d : c->pc + (uint32_t)d;
        break;
    }
    default: return CUBEX_CPU_UNSUPPORTED;
    }
    c->pc = next;
    c->retired_instructions++;
    return CUBEX_CPU_OK;
}
