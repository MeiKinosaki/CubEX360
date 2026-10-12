#ifndef CUBEX360_MEMORY_H
#define CUBEX360_MEMORY_H
#include <stddef.h>
#include <stdint.h>
#define CUBEX360_MAIN_RAM_SIZE (24u * 1024u * 1024u)
typedef struct cubex_memory { uint8_t *main_ram; size_t main_ram_size; } cubex_memory_t;
int cubex_memory_init(cubex_memory_t *mem);
void cubex_memory_destroy(cubex_memory_t *mem);
int cubex_memory_read8(const cubex_memory_t *mem,uint32_t address,uint8_t *out);
int cubex_memory_read16be(const cubex_memory_t *mem,uint32_t address,uint16_t *out);
int cubex_memory_read32be(const cubex_memory_t *mem,uint32_t address,uint32_t *out);
int cubex_memory_write8(cubex_memory_t *mem,uint32_t address,uint8_t value);
int cubex_memory_write16be(cubex_memory_t *mem,uint32_t address,uint16_t value);
int cubex_memory_write32be(cubex_memory_t *mem,uint32_t address,uint32_t value);
#endif
