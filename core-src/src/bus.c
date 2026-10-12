#include "cubex360/bus.h"
int cubex_bus_translate_main_ram(uint32_t a,uint32_t*o){if(!o)return -1;if(a<CUBEX360_MAIN_RAM_SIZE){*o=a;return 0;}if(a>=CUBEX360_RAM_CACHED_BASE&&a-CUBEX360_RAM_CACHED_BASE<CUBEX360_MAIN_RAM_SIZE){*o=a-CUBEX360_RAM_CACHED_BASE;return 0;}if(a>=CUBEX360_RAM_UNCACHED_BASE&&a-CUBEX360_RAM_UNCACHED_BASE<CUBEX360_MAIN_RAM_SIZE){*o=a-CUBEX360_RAM_UNCACHED_BASE;return 0;}return -1;}
int cubex_bus_read32be(const cubex_memory_t*m,uint32_t a,uint32_t*o){uint32_t p;return cubex_bus_translate_main_ram(a,&p)?-1:cubex_memory_read32be(m,p,o);}
int cubex_bus_write32be(cubex_memory_t*m,uint32_t a,uint32_t v){uint32_t p;return cubex_bus_translate_main_ram(a,&p)?-1:cubex_memory_write32be(m,p,v);}
