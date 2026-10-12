#ifndef CUBEX360_BUS_H
#define CUBEX360_BUS_H
#include "cubex360/memory.h"
#define CUBEX360_RAM_CACHED_BASE 0x80000000u
#define CUBEX360_RAM_UNCACHED_BASE 0xC0000000u
int cubex_bus_translate_main_ram(uint32_t address,uint32_t *offset);
int cubex_bus_read32be(const cubex_memory_t *mem,uint32_t address,uint32_t *out);
int cubex_bus_write32be(cubex_memory_t *mem,uint32_t address,uint32_t value);
#endif
