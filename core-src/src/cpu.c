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
static void set_cr_field(cubex_cpu_state_t *c, uint32_t bf, uint32_t field) {
    uint32_t shift = 28u - (bf * 4u);
    uint32_t mask = 0xfu << shift;
    c->cr = (c->cr & ~mask) | ((field & 0xfu) << shift);
}
static void set_cr0(cubex_cpu_state_t *c, uint32_t value) {
    uint32_t field = ((int32_t)value < 0) ? 8u :
                     (value == 0u) ? 2u : 4u;
    if (c->xer & 0x80000000u) field |= 1u;
    set_cr_field(c, 0u, field);
}
static uint32_t ppc_mask_mb_me(uint32_t mb, uint32_t me) {
    uint32_t mask = 0u, bit = mb & 31u;
    for (;;) {
        mask |= 1u << (31u - bit);
        if (bit == (me & 31u)) break;
        bit = (bit + 1u) & 31u;
    }
    return mask;
}
static void set_compare_field(cubex_cpu_state_t *c, uint32_t bf, int lt, int gt, int eq) {
    uint32_t field = lt ? 8u : (gt ? 4u : (eq ? 2u : 0u));
    if (c->xer & 0x80000000u) field |= 1u;
    set_cr_field(c, bf, field);
}

cubex_cpu_result_t cubex_cpu_step(cubex_cpu_state_t *c, cubex_memory_t *m) {
    uint32_t ins, op, rt, ra, rb, xo, imm, next;
    if (!c || !m) return CUBEX_CPU_BAD_ARGUMENT;
    if (cubex_bus_read32be(m, c->pc, &ins)) return CUBEX_CPU_MEMORY_FAULT;
    op = ins >> 26;
    rt = (ins >> 21) & 31u;
    ra = (ins >> 16) & 31u;
    imm = ins & 0xffffu;
    rb = (ins >> 11) & 31u;
    xo = (ins >> 1) & 0x3ffu;
    next = c->pc + 4u;
    switch (op) {
    case 10:
    case 11: {
        uint32_t bf = (ins >> 23) & 7u;
        uint32_t ra_value = c->gpr[ra];
        if (ins & 0x00200000u) return CUBEX_CPU_UNSUPPORTED;
        if (op == 11) {
            int32_t a = (int32_t)ra_value, b = sx16(imm);
            set_compare_field(c, bf, a < b, a > b, a == b);
        } else {
            uint32_t b = imm;
            set_compare_field(c, bf, ra_value < b, ra_value > b, ra_value == b);
        }
        break;
    }
    case 7:
        c->gpr[rt] = (uint32_t)((int64_t)(int32_t)c->gpr[ra] * (int64_t)sx16(imm));
        break;
    case 12:
    case 13: {
        uint64_t sum = (uint64_t)c->gpr[ra] + (uint64_t)(uint32_t)sx16(imm);
        c->gpr[rt] = (uint32_t)sum;
        if (sum >> 32) c->xer |= 0x20000000u;
        else c->xer &= ~0x20000000u;
        if (op == 13) set_cr0(c, c->gpr[rt]);
        break;
    }
    case 14: c->gpr[rt] = (ra ? c->gpr[ra] : 0u) + (uint32_t)sx16(imm); break;
    case 15: c->gpr[rt] = (ra ? c->gpr[ra] : 0u) + ((uint32_t)sx16(imm) << 16); break;
    case 24: c->gpr[ra] = c->gpr[rt] | imm; break;
    case 25: c->gpr[ra] = c->gpr[rt] | (imm << 16); break;
    case 26: c->gpr[ra] = c->gpr[rt] ^ imm; break;
    case 28:
        c->gpr[ra] = c->gpr[rt] & imm;
        set_cr0(c, c->gpr[ra]);
        break;
    case 29:
        c->gpr[ra] = c->gpr[rt] & (imm << 16);
        set_cr0(c, c->gpr[ra]);
        break;
    case 32: {
        uint32_t a = (ra ? c->gpr[ra] : 0u) + (uint32_t)sx16(imm);
        if (cubex_bus_read32be(m, a, &c->gpr[rt])) return CUBEX_CPU_MEMORY_FAULT;
        break;
    }
    case 34: {
        uint32_t a = (ra ? c->gpr[ra] : 0u) + (uint32_t)sx16(imm), p;
        uint8_t v;
        if (cubex_bus_translate_main_ram(a, &p) || cubex_memory_read8(m, p, &v)) return CUBEX_CPU_MEMORY_FAULT;
        c->gpr[rt] = v;
        break;
    }
    case 40:
    case 42: {
        uint32_t a = (ra ? c->gpr[ra] : 0u) + (uint32_t)sx16(imm), p;
        uint16_t v;
        if (cubex_bus_translate_main_ram(a, &p) || cubex_memory_read16be(m, p, &v)) return CUBEX_CPU_MEMORY_FAULT;
        c->gpr[rt] = (op == 42 && (v & 0x8000u)) ? (uint32_t)(int32_t)(int16_t)v : (uint32_t)v;
        break;
    }
    case 36: {
        uint32_t a = (ra ? c->gpr[ra] : 0u) + (uint32_t)sx16(imm);
        if (cubex_bus_write32be(m, a, c->gpr[rt])) return CUBEX_CPU_MEMORY_FAULT;
        break;
    }
    case 38: {
        uint32_t a = (ra ? c->gpr[ra] : 0u) + (uint32_t)sx16(imm), p;
        if (cubex_bus_translate_main_ram(a, &p) || cubex_memory_write8(m, p, (uint8_t)c->gpr[rt])) return CUBEX_CPU_MEMORY_FAULT;
        break;
    }
    case 44: {
        uint32_t a = (ra ? c->gpr[ra] : 0u) + (uint32_t)sx16(imm), p;
        if (cubex_bus_translate_main_ram(a, &p) || cubex_memory_write16be(m, p, (uint16_t)c->gpr[rt])) return CUBEX_CPU_MEMORY_FAULT;
        break;
    }
    case 21: {
        uint32_t sh = (ins >> 11) & 31u;
        uint32_t mb = (ins >> 6) & 31u;
        uint32_t me = (ins >> 1) & 31u;
        uint32_t mask = ppc_mask_mb_me(mb, me);
        uint32_t source = c->gpr[rt];
        uint32_t rotated = (source << sh) | (source >> ((32u - sh) & 31u));
        c->gpr[ra] = rotated & mask;
        if (ins & 1u) set_cr0(c, c->gpr[ra]);
        break;
    }
    case 19:
        if (xo == 16u || xo == 528u) {
            uint32_t bo = (ins >> 21) & 31u;
            uint32_t bi = (ins >> 16) & 31u;
            uint32_t target = (xo == 16u ? c->lr : c->ctr) & ~3u;
            int ctr_ok = 1, cr_ok = 1;
            if (xo == 16u && !(bo & 0x04u)) {
                c->ctr--;
                ctr_ok = ((c->ctr != 0u) != ((bo & 0x02u) != 0u));
            }
            if (!(bo & 0x10u)) {
                uint32_t cr_bit = (c->cr >> (31u - bi)) & 1u;
                cr_ok = cr_bit == ((bo & 0x08u) != 0u);
            }
            if (ctr_ok && cr_ok) next = target;
            if (ins & 1u) c->lr = c->pc + 4u;
        } else {
            return CUBEX_CPU_UNSUPPORTED;
        }
        break;
    case 31:
        switch (xo) {
        case 0:
        case 32: {
            uint32_t bf = (ins >> 23) & 7u;
            if (ins & 0x00200000u) return CUBEX_CPU_UNSUPPORTED;
            if (xo == 0u) {
                int32_t a = (int32_t)c->gpr[ra], b = (int32_t)c->gpr[rb];
                set_compare_field(c, bf, a < b, a > b, a == b);
            } else {
                uint32_t a = c->gpr[ra], b = c->gpr[rb];
                set_compare_field(c, bf, a < b, a > b, a == b);
            }
            break;
        }
        case 339: {
            uint32_t spr = ((ins >> 16) & 31u) | ((ins >> 6) & 0x3e0u);
            if (spr == 8u) c->gpr[rt] = c->lr;
            else if (spr == 9u) c->gpr[rt] = c->ctr;
            else return CUBEX_CPU_UNSUPPORTED;
            break;
        }
        case 467: {
            uint32_t spr = ((ins >> 16) & 31u) | ((ins >> 6) & 0x3e0u);
            if (spr == 8u) c->lr = c->gpr[rt];
            else if (spr == 9u) c->ctr = c->gpr[rt];
            else return CUBEX_CPU_UNSUPPORTED;
            break;
        }
        case 266: c->gpr[rt] = c->gpr[ra] + c->gpr[rb]; if (ins & 1u) set_cr0(c, c->gpr[rt]); break;
        case 40: c->gpr[rt] = c->gpr[rb] - c->gpr[ra]; if (ins & 1u) set_cr0(c, c->gpr[rt]); break;
        case 28: c->gpr[ra] = c->gpr[rt] & c->gpr[rb]; if (ins & 1u) set_cr0(c, c->gpr[ra]); break;
        case 444: c->gpr[ra] = c->gpr[rt] | c->gpr[rb]; if (ins & 1u) set_cr0(c, c->gpr[ra]); break;
        case 316: c->gpr[ra] = c->gpr[rt] ^ c->gpr[rb]; if (ins & 1u) set_cr0(c, c->gpr[ra]); break;
        default: return CUBEX_CPU_UNSUPPORTED;
        }
        break;
    case 16: {
        uint32_t bo = (ins >> 21) & 31u;
        uint32_t bi = (ins >> 16) & 31u;
        int32_t d = (int32_t)(ins & 0x0000fffcu);
        int ctr_ok = 1, cr_ok = 1;
        if (d & 0x00008000) d |= (int32_t)0xffff0000u;
        if (!(bo & 0x04u)) {
            c->ctr--;
            ctr_ok = ((c->ctr != 0u) != ((bo & 0x02u) != 0u));
        }
        if (!(bo & 0x10u)) {
            uint32_t cr_bit = (c->cr >> (31u - bi)) & 1u;
            cr_ok = cr_bit == ((bo & 0x08u) != 0u);
        }
        if (ctr_ok && cr_ok)
            next = (ins & 2u) ? (uint32_t)d : c->pc + (uint32_t)d;
        if (ins & 1u) c->lr = c->pc + 4u;
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
