#include "cubex360/memory.h"
#include <stdlib.h>
#include <string.h>
int cubex_memory_init(cubex_memory_t *m){if(!m)return -1;m->main_ram=(uint8_t*)calloc(CUBEX360_MAIN_RAM_SIZE,1);if(!m->main_ram){m->main_ram_size=0;return -2;}m->main_ram_size=CUBEX360_MAIN_RAM_SIZE;return 0;}
void cubex_memory_destroy(cubex_memory_t *m){if(!m)return;free(m->main_ram);m->main_ram=0;m->main_ram_size=0;}
static int valid(const cubex_memory_t*m,uint32_t a,size_t n){return m&&m->main_ram&&n<=m->main_ram_size&&(size_t)a<=m->main_ram_size-n;}
int cubex_memory_read8(const cubex_memory_t*m,uint32_t a,uint8_t*o){if(!o||!valid(m,a,1))return -1;*o=m->main_ram[a];return 0;}
int cubex_memory_read16be(const cubex_memory_t*m,uint32_t a,uint16_t*o){if(!o||(a&1)||!valid(m,a,2))return -1;*o=(uint16_t)(((uint16_t)m->main_ram[a]<<8)|m->main_ram[a+1]);return 0;}
int cubex_memory_read32be(const cubex_memory_t*m,uint32_t a,uint32_t*o){if(!o||(a&3)||!valid(m,a,4))return -1;*o=((uint32_t)m->main_ram[a]<<24)|((uint32_t)m->main_ram[a+1]<<16)|((uint32_t)m->main_ram[a+2]<<8)|m->main_ram[a+3];return 0;}
int cubex_memory_write8(cubex_memory_t*m,uint32_t a,uint8_t v){if(!valid(m,a,1))return -1;m->main_ram[a]=v;return 0;}
int cubex_memory_write16be(cubex_memory_t*m,uint32_t a,uint16_t v){if((a&1)||!valid(m,a,2))return -1;m->main_ram[a]=(uint8_t)(v>>8);m->main_ram[a+1]=(uint8_t)v;return 0;}
int cubex_memory_write32be(cubex_memory_t*m,uint32_t a,uint32_t v){if((a&3)||!valid(m,a,4))return -1;m->main_ram[a]=(uint8_t)(v>>24);m->main_ram[a+1]=(uint8_t)(v>>16);m->main_ram[a+2]=(uint8_t)(v>>8);m->main_ram[a+3]=(uint8_t)v;return 0;}

int cubex_memory_copy_from(const cubex_memory_t *m, uint32_t a, void *destination, size_t size) {
    if (size == 0u) return 0;
    if (!destination || !valid(m, a, size)) return -1;
    memmove(destination, m->main_ram + a, size);
    return 0;
}
int cubex_memory_copy_to(cubex_memory_t *m, uint32_t a, const void *source, size_t size) {
    if (size == 0u) return 0;
    if (!source || !valid(m, a, size)) return -1;
    memmove(m->main_ram + a, source, size);
    return 0;
}
