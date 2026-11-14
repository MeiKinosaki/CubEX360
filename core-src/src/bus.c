#include "cubex360/bus.h"

static int mmio_read_word(const cubex_memory_t *m, uint32_t address,
                          uint32_t *value) {
    if (!m || !m->mmio_read32 || !value) return -1;
    return m->mmio_read32(m->bus_context, address & ~3u, value);
}

static int mmio_write_word(cubex_memory_t *m, uint32_t address,
                           uint32_t value) {
    if (!m || !m->mmio_write32) return -1;
    return m->mmio_write32(m->bus_context, address & ~3u, value);
}

int cubex_bus_translate_main_ram(uint32_t a, uint32_t *o) {
    if (!o) return -1;
    if (a < CUBEX360_MAIN_RAM_SIZE) { *o = a; return 0; }
    if (a >= CUBEX360_RAM_CACHED_BASE &&
        a - CUBEX360_RAM_CACHED_BASE < CUBEX360_MAIN_RAM_SIZE) {
        *o = a - CUBEX360_RAM_CACHED_BASE; return 0;
    }
    if (a >= CUBEX360_RAM_UNCACHED_BASE &&
        a - CUBEX360_RAM_UNCACHED_BASE < CUBEX360_MAIN_RAM_SIZE) {
        *o = a - CUBEX360_RAM_UNCACHED_BASE; return 0;
    }
    return -1;
}

int cubex_bus_read32be(const cubex_memory_t *m, uint32_t a, uint32_t *o) {
    uint32_t p;
    if (!o || !m) return -1;
    if (cubex_bus_translate_main_ram(a, &p) == 0)
        return cubex_memory_read32be(m, p, o);
    return mmio_read_word(m, a, o);
}

int cubex_bus_read8(const cubex_memory_t *m, uint32_t a, uint8_t *o) {
    uint32_t p, word;
    unsigned shift;
    if (!o || !m) return -1;
    if (cubex_bus_translate_main_ram(a, &p) == 0)
        return cubex_memory_read8(m, p, o);
    if (mmio_read_word(m, a, &word) != 0) return -1;
    shift = (3u - (a & 3u)) * 8u;
    *o = (uint8_t)(word >> shift);
    return 0;
}

int cubex_bus_read16be(const cubex_memory_t *m, uint32_t a, uint16_t *o) {
    uint32_t p, word;
    unsigned shift;
    if (!o || !m) return -1;
    if (cubex_bus_translate_main_ram(a, &p) == 0)
        return cubex_memory_read16be(m, p, o);
    if ((a & 3u) == 3u || mmio_read_word(m, a, &word) != 0) return -1;
    shift = (2u - (a & 2u)) * 8u;
    *o = (uint16_t)(word >> shift);
    return 0;
}

int cubex_bus_write32be(cubex_memory_t *m, uint32_t a, uint32_t v) {
    uint32_t p;
    if (!m) return -1;
    if (cubex_bus_translate_main_ram(a, &p) == 0)
        return cubex_memory_write32be(m, p, v);
    return mmio_write_word(m, a, v);
}

int cubex_bus_write8(cubex_memory_t *m, uint32_t a, uint8_t v) {
    uint32_t p, word;
    unsigned shift;
    if (!m) return -1;
    if (cubex_bus_translate_main_ram(a, &p) == 0)
        return cubex_memory_write8(m, p, v);
    if (mmio_read_word(m, a, &word) != 0) return -1;
    shift = (3u - (a & 3u)) * 8u;
    word = (word & ~(0xffu << shift)) | ((uint32_t)v << shift);
    return mmio_write_word(m, a, word);
}

int cubex_bus_write16be(cubex_memory_t *m, uint32_t a, uint16_t v) {
    uint32_t p, word;
    unsigned shift;
    if (!m) return -1;
    if (cubex_bus_translate_main_ram(a, &p) == 0)
        return cubex_memory_write16be(m, p, v);
    if ((a & 3u) == 3u || mmio_read_word(m, a, &word) != 0) return -1;
    shift = (2u - (a & 2u)) * 8u;
    word = (word & ~(0xffffu << shift)) | ((uint32_t)v << shift);
    return mmio_write_word(m, a, word);
}

int cubex_bus_copy_from(const cubex_memory_t *m, uint32_t a,
                        void *dst, size_t n) {
    uint32_t p;
    if (n == 0u) return m && m->main_ram ? 0 : -1;
    return cubex_bus_translate_main_ram(a, &p) ? -1 :
           cubex_memory_copy_from(m, p, dst, n);
}

int cubex_bus_copy_to(cubex_memory_t *m, uint32_t a,
                      const void *src, size_t n) {
    uint32_t p;
    if (n == 0u) return m && m->main_ram ? 0 : -1;
    return cubex_bus_translate_main_ram(a, &p) ? -1 :
           cubex_memory_copy_to(m, p, src, n);
}
