#ifndef CUBEX360_BUS_H
#define CUBEX360_BUS_H
#include "cubex360/memory.h"
#define CUBEX360_RAM_CACHED_BASE 0x80000000u
#define CUBEX360_RAM_UNCACHED_BASE 0xC0000000u
int cubex_bus_translate_main_ram(uint32_t address,uint32_t *offset);
int cubex_bus_read8(const cubex_memory_t *mem,uint32_t address,uint8_t *out);
int cubex_bus_read16be(const cubex_memory_t *mem,uint32_t address,uint16_t *out);
int cubex_bus_read32be(const cubex_memory_t *mem,uint32_t address,uint32_t *out);
int cubex_bus_write8(cubex_memory_t *mem,uint32_t address,uint8_t value);
int cubex_bus_write16be(cubex_memory_t *mem,uint32_t address,uint16_t value);
int cubex_bus_write32be(cubex_memory_t *mem,uint32_t address,uint32_t value);
#endif
