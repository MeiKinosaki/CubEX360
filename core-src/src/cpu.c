#include "cubex360/cpu.h"
#include <string.h>
#include <limits.h>

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
    case 8: { /* SUBFIC: signed immediate minus RA; CA means no unsigned borrow */
        uint32_t minuend = ra ? c->gpr[ra] : 0u;
        uint32_t subtrahend = (uint32_t)sx16(imm);
        c->gpr[rt] = subtrahend - minuend;
        if (subtrahend >= minuend) c->xer |= 0x20000000u;
        else c->xer &= ~0x20000000u;
        break;
    }
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
    case 27: c->gpr[ra] = c->gpr[rt] ^ (imm << 16); break;
    case 28:
        c->gpr[ra] = c->gpr[rt] & imm;
        set_cr0(c, c->gpr[ra]);
        break;
    case 29:
        c->gpr[ra] = c->gpr[rt] & (imm << 16);
        set_cr0(c, c->gpr[ra]);
        break;
    case 33: { /* LWZU: load word and update RA */
        uint32_t a;
        if (ra == 0u || ra == rt) return CUBEX_CPU_UNSUPPORTED;
        a = c->gpr[ra] + (uint32_t)sx16(imm);
        if (cubex_bus_read32be(m, a, &c->gpr[rt])) return CUBEX_CPU_MEMORY_FAULT;
        c->gpr[ra] = a;
        break;
    }
    case 35: { /* LBZU: load byte and update RA */
        uint32_t a, p;
        uint8_t v;
        if (ra == 0u || ra == rt) return CUBEX_CPU_UNSUPPORTED;
        a = c->gpr[ra] + (uint32_t)sx16(imm);
        if (cubex_bus_translate_main_ram(a, &p) || cubex_memory_read8(m, p, &v))
            return CUBEX_CPU_MEMORY_FAULT;
        c->gpr[rt] = v;
        c->gpr[ra] = a;
        break;
    }
    case 37: { /* STWU: store word and update RA */
        uint32_t a;
        if (ra == 0u) return CUBEX_CPU_UNSUPPORTED;
        a = c->gpr[ra] + (uint32_t)sx16(imm);
        if (cubex_bus_write32be(m, a, c->gpr[rt])) return CUBEX_CPU_MEMORY_FAULT;
        c->gpr[ra] = a;
        break;
    }
    case 39: { /* STBU: store byte and update RA */
        uint32_t a, p;
        if (ra == 0u) return CUBEX_CPU_UNSUPPORTED;
        a = c->gpr[ra] + (uint32_t)sx16(imm);
        if (cubex_bus_translate_main_ram(a, &p) || cubex_memory_write8(m, p, (uint8_t)c->gpr[rt]))
            return CUBEX_CPU_MEMORY_FAULT;
        c->gpr[ra] = a;
        break;
    }
    case 41: { /* LHZU: load halfword and update RA */
        uint32_t a, p;
        uint16_t v;
        if (ra == 0u || ra == rt) return CUBEX_CPU_UNSUPPORTED;
        a = c->gpr[ra] + (uint32_t)sx16(imm);
        if (cubex_bus_translate_main_ram(a, &p) || cubex_memory_read16be(m, p, &v))
            return CUBEX_CPU_MEMORY_FAULT;
        c->gpr[rt] = v;
        c->gpr[ra] = a;
        break;
    }
    case 43: { /* LHAU: load algebraic halfword and update RA */
        uint32_t a, p;
        uint16_t v;
        if (ra == 0u || ra == rt) return CUBEX_CPU_UNSUPPORTED;
        a = c->gpr[ra] + (uint32_t)sx16(imm);
        if (cubex_bus_translate_main_ram(a, &p) || cubex_memory_read16be(m, p, &v))
            return CUBEX_CPU_MEMORY_FAULT;
        c->gpr[rt] = (v & 0x8000u) ? (uint32_t)(int32_t)(int16_t)v : (uint32_t)v;
        c->gpr[ra] = a;
        break;
    }
    case 45: { /* STHU: store halfword and update RA */
        uint32_t a, p;
        if (ra == 0u) return CUBEX_CPU_UNSUPPORTED;
        a = c->gpr[ra] + (uint32_t)sx16(imm);
        if (cubex_bus_translate_main_ram(a, &p) || cubex_memory_write16be(m, p, (uint16_t)c->gpr[rt]))
            return CUBEX_CPU_MEMORY_FAULT;
        c->gpr[ra] = a;
        break;
    }
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
    case 20:
    case 21:
    case 23: {
        uint32_t sh = (op == 23u) ? (c->gpr[rb] & 31u) : ((ins >> 11) & 31u);
        uint32_t mb = (ins >> 6) & 31u;
        uint32_t me = (ins >> 1) & 31u;
        uint32_t mask = ppc_mask_mb_me(mb, me);
        uint32_t source = c->gpr[rt];
        uint32_t rotated = (source << sh) | (source >> ((32u - sh) & 31u));
        if (op == 20u)
            c->gpr[ra] = (c->gpr[ra] & ~mask) | (rotated & mask);
        else
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
            uint32_t bt = (ins >> 21) & 31u;
            uint32_t ba = (ins >> 16) & 31u;
            uint32_t bb = (ins >> 11) & 31u;
            uint32_t a = (c->cr >> (31u - ba)) & 1u;
            uint32_t b = (c->cr >> (31u - bb)) & 1u;
            uint32_t result;
            switch (xo) {
            case 33:  result = !(a | b); break;       /* CRNOR */
            case 129: result = a & !b; break;         /* CRANDC */
            case 193: result = a ^ b; break;          /* CRXOR */
            case 225: result = !(a & b); break;       /* CRNAND */
            case 257: result = a & b; break;          /* CRAND */
            case 289: result = !(a ^ b); break;       /* CREQV */
            case 417: result = a | !b; break;         /* CRORC */
            case 449: result = a | b; break;          /* CROR */
            default: return CUBEX_CPU_UNSUPPORTED;
            }
            c->cr = (c->cr & ~(1u << (31u - bt))) | (result << (31u - bt));
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
        case 512: { /* MCRXR: move XER SO/OV/CA to CR field and clear them */
            uint32_t bf = (ins >> 23) & 7u;
            uint32_t field = ((c->xer >> 31) & 1u) << 3;
            field |= ((c->xer >> 30) & 1u) << 2;
            field |= ((c->xer >> 29) & 1u) << 1;
            set_cr_field(c, bf, field);
            c->xer &= ~0xe0000000u;
            break;
        }
        case 19: /* MFCR: move complete condition register to GPR */
            c->gpr[rt] = c->cr;
            break;
        case 144: { /* MTCRF: move selected CR fields from GPR */
            uint32_t fxm = (ins >> 12) & 0xffu;
            uint32_t source = c->gpr[rt];
            uint32_t field;
            for (field = 0u; field < 8u; ++field) {
                if (fxm & (1u << (7u - field))) {
                    uint32_t shift = 28u - field * 4u;
                    uint32_t mask = 0xfu << shift;
                    c->cr = (c->cr & ~mask) | (source & mask);
                }
            }
            break;
        }
        case 26: { /* CNTLZW: count leading zero bits */
            uint32_t value = c->gpr[rt], count = 0u;
            while (count < 32u && (value & (0x80000000u >> count)) == 0u) ++count;
            c->gpr[ra] = count;
            if (ins & 1u) set_cr0(c, count);
            break;
        }
        case 60: /* ANDC */
            c->gpr[ra] = c->gpr[rt] & ~c->gpr[rb];
            if (ins & 1u) set_cr0(c, c->gpr[ra]);
            break;
        case 104: /* NEG: negate source register modulo 2^32 */
            c->gpr[ra] = 0u - c->gpr[rt];
            if (ins & 1u) set_cr0(c, c->gpr[ra]);
            break;
        case 124: /* NOR */
            c->gpr[ra] = ~(c->gpr[rt] | c->gpr[rb]);
            if (ins & 1u) set_cr0(c, c->gpr[ra]);
            break;
        case 284: /* EQV */
            c->gpr[ra] = ~(c->gpr[rt] ^ c->gpr[rb]);
            if (ins & 1u) set_cr0(c, c->gpr[ra]);
            break;
        case 476: /* NAND */
            c->gpr[ra] = ~(c->gpr[rt] & c->gpr[rb]);
            if (ins & 1u) set_cr0(c, c->gpr[ra]);
            break;
        case 922: /* EXTSH */
            c->gpr[ra] = (uint32_t)(int32_t)(int16_t)c->gpr[rt];
            if (ins & 1u) set_cr0(c, c->gpr[ra]);
            break;
        case 954: /* EXTSB */
            c->gpr[ra] = (uint32_t)(int32_t)(int8_t)c->gpr[rt];
            if (ins & 1u) set_cr0(c, c->gpr[ra]);
            break;
        case 24: /* SLW: shift left word */
            c->gpr[ra] = (c->gpr[rb] & 0x20u) ? 0u :
                         (c->gpr[rt] << (c->gpr[rb] & 31u));
            if (ins & 1u) set_cr0(c, c->gpr[ra]);
            break;
        case 536: /* SRW: shift right word */
            c->gpr[ra] = (c->gpr[rb] & 0x20u) ? 0u :
                         (c->gpr[rt] >> (c->gpr[rb] & 31u));
            if (ins & 1u) set_cr0(c, c->gpr[ra]);
            break;
        case 824: { /* SRAWI: arithmetic right shift immediate */
            uint32_t sh = (ins >> 11) & 31u;
            uint32_t source = c->gpr[rt];
            uint32_t result = source;
            int discarded_one = 0;
            if (sh != 0u) {
                uint32_t discarded_mask = (1u << sh) - 1u;
                discarded_one = ((source & 0x80000000u) != 0u) &&
                                ((source & discarded_mask) != 0u);
                result = source >> sh;
                if (source & 0x80000000u)
                    result |= (~0u << (32u - sh));
            }
            c->gpr[ra] = result;
            if (discarded_one) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            if (ins & 1u) set_cr0(c, result);
            break;
        }
        case 792: { /* SRAW: arithmetic right shift word by register */
            uint32_t source = c->gpr[rt];
            uint32_t shift = c->gpr[rb] & 0x3fu;
            uint32_t result;
            int negative = (source & 0x80000000u) != 0u;
            int discarded_one = 0;
            if (shift == 0u) {
                result = source;
            } else if (shift >= 32u) {
                result = negative ? 0xffffffffu : 0u;
                discarded_one = negative && ((source & 0x7fffffffu) != 0u);
            } else {
                uint32_t discarded_mask = (1u << shift) - 1u;
                discarded_one = negative && ((source & discarded_mask) != 0u);
                result = source >> shift;
                if (negative) result |= (~0u << (32u - shift));
            }
            c->gpr[ra] = result;
            if (discarded_one) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            if (ins & 1u) set_cr0(c, result);
            break;
        }
        case 1014: { /* DCBZ: clear the addressed 32-byte cache block */
            uint32_t effective = (ra ? c->gpr[ra] : 0u) + c->gpr[rb];
            uint32_t physical;
            uint32_t i;
            if (cubex_bus_translate_main_ram(effective & ~31u, &physical) ||
                physical > mem->main_ram_size ||
                32u > mem->main_ram_size - physical)
                return CUBEX_CPU_MEMORY_FAULT;
            for (i = 0u; i < 32u; ++i) {
                if (cubex_memory_write8(m, physical + i, 0u))
                    return CUBEX_CPU_MEMORY_FAULT;
            }
            break;
        }
        case 339: {
            uint32_t spr = ((ins >> 16) & 31u) | ((ins >> 6) & 0x3e0u);
            if (spr == 8u) c->gpr[rt] = c->lr;
            else if (spr == 9u) c->gpr[rt] = c->ctr;
            else if (spr == 1u) c->gpr[rt] = c->xer;
            else return CUBEX_CPU_UNSUPPORTED;
            break;
        }
        case 467: {
            uint32_t spr = ((ins >> 16) & 31u) | ((ins >> 6) & 0x3e0u);
            if (spr == 8u) c->lr = c->gpr[rt];
            else if (spr == 9u) c->ctr = c->gpr[rt];
            else if (spr == 1u) c->xer = c->gpr[rt];
            else return CUBEX_CPU_UNSUPPORTED;
            break;
        }
        case 459: { /* DIVWU: unsigned word division; zero divisor has deterministic placeholder */
            uint32_t dividend = c->gpr[ra], divisor = c->gpr[rb];
            c->gpr[rt] = divisor ? dividend / divisor : 0u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 616: { /* NEGO: negate with signed overflow enabled */
            uint32_t a = c->gpr[rt];
            uint32_t result = 0u - a;
            uint32_t overflow = (a == 0x80000000u);
            c->gpr[ra] = result;
            c->xer = (c->xer & ~0x40000000u) | (overflow << 30);
            if (overflow) c->xer |= 0x80000000u; /* SO is sticky */
            if (ins & 1u) set_cr0(c, result);
            break;
        }
        case 491: { /* DIVW: signed word division */
            int32_t dividend = (int32_t)c->gpr[ra];
            int32_t divisor = (int32_t)c->gpr[rb];
            int overflow = (divisor == 0) ||
                           (dividend == INT32_MIN && divisor == -1);
            int32_t quotient = overflow ? 0 : (int32_t)((int64_t)dividend / (int64_t)divisor);
            c->gpr[rt] = (uint32_t)quotient;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 1003: { /* DIVWO: signed word division with overflow enabled */
            int32_t dividend = (int32_t)c->gpr[ra];
            int32_t divisor = (int32_t)c->gpr[rb];
            int overflow = (divisor == 0) ||
                           (dividend == INT32_MIN && divisor == -1);
            int32_t quotient = overflow ? 0 : (int32_t)((int64_t)dividend / (int64_t)divisor);
            c->gpr[rt] = (uint32_t)quotient;
            c->xer = (c->xer & ~0x40000000u) | ((uint32_t)overflow << 30);
            if (overflow) c->xer |= 0x80000000u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 971: { /* DIVWUO: unsigned word division with overflow enabled */
            uint32_t dividend = c->gpr[ra], divisor = c->gpr[rb];
            int overflow = divisor == 0u;
            c->gpr[rt] = overflow ? 0u : dividend / divisor;
            c->xer = (c->xer & ~0x40000000u) | ((uint32_t)overflow << 30);
            if (overflow) c->xer |= 0x80000000u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 714: { /* ADDZEO: RA + XER.CA, signed overflow enabled */
            uint32_t a = c->gpr[ra];
            uint64_t sum = (uint64_t)a + ((c->xer >> 29) & 1u);
            uint32_t result = (uint32_t)sum;
            uint32_t overflow = ((~(a ^ 0u) & (a ^ result)) >> 31) & 1u;
            c->gpr[rt] = result;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            c->xer = (c->xer & ~0x40000000u) | (overflow << 30);
            if (overflow) c->xer |= 0x80000000u; /* SO is sticky */
            if (ins & 1u) set_cr0(c, result);
            break;
        }
        case 747: { /* MULLWO: low word, with signed overflow enabled */
            int64_t product = (int64_t)(int32_t)c->gpr[ra] *
                              (int64_t)(int32_t)c->gpr[rb];
            uint32_t result = (uint32_t)(uint64_t)product;
            int overflow = product > INT32_MAX || product < INT32_MIN;
            c->gpr[rt] = result;
            c->xer = (c->xer & ~0x40000000u) |
                     ((uint32_t)overflow << 30);
            if (overflow) c->xer |= 0x80000000u;
            if (ins & 1u) set_cr0(c, result);
            break;
        }
        case 75: { /* MULHW: high word of signed 32x32 product */
            int64_t product = (int64_t)(int32_t)c->gpr[ra] *
                              (int64_t)(int32_t)c->gpr[rb];
            c->gpr[rt] = (uint32_t)((uint64_t)product >> 32);
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 11: { /* MULHWU: high word of unsigned 32x32 product */
            uint64_t product = (uint64_t)c->gpr[ra] * (uint64_t)c->gpr[rb];
            c->gpr[rt] = (uint32_t)(product >> 32);
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 10: { /* ADDC: add with carry-out */
            uint64_t sum = (uint64_t)c->gpr[ra] + (uint64_t)c->gpr[rb];
            c->gpr[rt] = (uint32_t)sum;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 520: { /* SUBFCO: RB - RA, carry and signed overflow */
            uint32_t a = c->gpr[rb], b = c->gpr[ra];
            uint64_t sum = (uint64_t)a + (uint64_t)(~b) + 1u;
            uint32_t result = (uint32_t)sum;
            int64_t signed_diff = (int64_t)(int32_t)a - (int64_t)(int32_t)b;
            uint32_t overflow = (signed_diff > INT32_MAX || signed_diff < INT32_MIN);
            c->gpr[rt] = result;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            c->xer = (c->xer & ~0x40000000u) | (overflow << 30);
            if (overflow) c->xer |= 0x80000000u;
            if (ins & 1u) set_cr0(c, result);
            break;
        }
        case 8: { /* SUBFC: RB - RA; CA is set when no borrow occurs */
            uint64_t sum = (uint64_t)c->gpr[rb] + (uint64_t)(~c->gpr[ra]) + 1u;
            c->gpr[rt] = (uint32_t)sum;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 650: { /* ADDEO: RA + RB + CA, signed overflow enabled */
            uint32_t a = c->gpr[ra], b = c->gpr[rb];
            uint64_t sum = (uint64_t)a + (uint64_t)b + ((c->xer >> 29) & 1u);
            uint32_t result = (uint32_t)sum;
            int64_t signed_sum = (int64_t)(int32_t)a + (int64_t)(int32_t)b +
                                 (int64_t)((c->xer >> 29) & 1u);
            uint32_t overflow = (signed_sum > INT32_MAX || signed_sum < INT32_MIN);
            c->gpr[rt] = result;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            c->xer = (c->xer & ~0x40000000u) | (overflow << 30);
            if (overflow) c->xer |= 0x80000000u;
            if (ins & 1u) set_cr0(c, result);
            break;
        }
        case 138: { /* ADDE: RA + RB + XER.CA */
            uint64_t sum = (uint64_t)c->gpr[ra] + (uint64_t)c->gpr[rb] +
                           ((c->xer >> 29) & 1u);
            c->gpr[rt] = (uint32_t)sum;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 648: { /* SUBFEO: RB - RA - (1 - CA), overflow enabled */
            uint32_t a = c->gpr[rb], b = c->gpr[ra];
            uint32_t ca = (c->xer >> 29) & 1u;
            uint64_t sum = (uint64_t)a + (uint64_t)(~b) + ca;
            uint32_t result = (uint32_t)sum;
            int64_t signed_diff = (int64_t)(int32_t)a - (int64_t)(int32_t)b -
                                  (int64_t)(1u - ca);
            uint32_t overflow = (signed_diff > INT32_MAX || signed_diff < INT32_MIN);
            c->gpr[rt] = result;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            c->xer = (c->xer & ~0x40000000u) | (overflow << 30);
            if (overflow) c->xer |= 0x80000000u;
            if (ins & 1u) set_cr0(c, result);
            break;
        }
        case 136: { /* SUBFE: RB - RA - (1 - XER.CA) */
            uint64_t sum = (uint64_t)c->gpr[rb] + (uint64_t)(~c->gpr[ra]) +
                           ((c->xer >> 29) & 1u);
            c->gpr[rt] = (uint32_t)sum;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 234: { /* ADDME: RA + 0xffffffff + XER.CA */
            uint64_t sum = (uint64_t)c->gpr[ra] + 0xffffffffull +
                           ((c->xer >> 29) & 1u);
            c->gpr[rt] = (uint32_t)sum;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 202: { /* ADDZE: RA + XER.CA */
            uint64_t sum = (uint64_t)c->gpr[ra] + ((c->xer >> 29) & 1u);
            c->gpr[rt] = (uint32_t)sum;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 232: { /* SUBFME: ~RA + 0xffffffff + XER.CA */
            uint64_t sum = (uint64_t)(~c->gpr[ra]) + 0xffffffffull +
                           ((c->xer >> 29) & 1u);
            c->gpr[rt] = (uint32_t)sum;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 200: { /* SUBFZE: 0 - RA - (1 - XER.CA) */
            uint64_t sum = (uint64_t)(~c->gpr[ra]) +
                           ((c->xer >> 29) & 1u);
            c->gpr[rt] = (uint32_t)sum;
            if (sum >> 32) c->xer |= 0x20000000u;
            else c->xer &= ~0x20000000u;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 235: { /* MULLW: low 32 bits of signed word product */
            int64_t product = (int64_t)(int32_t)c->gpr[ra] *
                              (int64_t)(int32_t)c->gpr[rb];
            c->gpr[rt] = (uint32_t)(uint64_t)product;
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        }
        case 266: /* ADD */ 
            c->gpr[rt] = c->gpr[ra] + c->gpr[rb];
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        case 778: { /* ADDO: signed overflow enabled */
            uint32_t a = c->gpr[ra], b = c->gpr[rb], result = a + b;
            uint32_t overflow = ((~(a ^ b) & (a ^ result)) >> 31) & 1u;
            c->gpr[rt] = result;
            c->xer = (c->xer & ~0x40000000u) | (overflow << 30);
            if (overflow) c->xer |= 0x80000000u; /* SO is sticky */
            if (ins & 1u) set_cr0(c, result);
            break;
        }
        case 40: /* SUBF: RB - RA */
            c->gpr[rt] = c->gpr[rb] - c->gpr[ra];
            if (ins & 1u) set_cr0(c, c->gpr[rt]);
            break;
        case 552: { /* SUBFO: signed overflow enabled */
            uint32_t a = c->gpr[rb], b = c->gpr[ra], result = a - b;
            uint32_t overflow = (((a ^ b) & (a ^ result)) >> 31) & 1u;
            c->gpr[rt] = result;
            c->xer = (c->xer & ~0x40000000u) | (overflow << 30);
            if (overflow) c->xer |= 0x80000000u; /* SO is sticky */
            if (ins & 1u) set_cr0(c, result);
            break;
        }
        case 28: c->gpr[ra] = c->gpr[rt] & c->gpr[rb]; if (ins & 1u) set_cr0(c, c->gpr[ra]); break;
        case 412: /* ORC */
            c->gpr[ra] = c->gpr[rt] | ~c->gpr[rb];
            if (ins & 1u) set_cr0(c, c->gpr[ra]);
            break;
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
        if (ins & 1u) c->lr = c->pc + 4u;
        break;
    }
    default: return CUBEX_CPU_UNSUPPORTED;
    }
    c->pc = next;
    c->retired_instructions++;
    return CUBEX_CPU_OK;
}

cubex_cpu_result_t cubex_cpu_run(cubex_cpu_state_t *c, cubex_memory_t *m, uint64_t instruction_budget) {
    uint64_t i;
    if (!c || !m) return CUBEX_CPU_BAD_ARGUMENT;
    for (i = 0; i < instruction_budget; ++i) {
        cubex_cpu_result_t result = cubex_cpu_step(c, m);
        if (result != CUBEX_CPU_OK) return result;
    }
    return CUBEX_CPU_OK;
}
