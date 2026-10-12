#include "cubex360/cpu.h"
#include <stdio.h>
int main(void){cubex_memory_t mem;cubex_cpu_state_t cpu;if(cubex_memory_init(&mem))return 1;if(cubex_bus_write32be(&mem,CUBEX360_RAM_CACHED_BASE,0x3860002au)){cubex_memory_destroy(&mem);return 2;}cubex_cpu_reset(&cpu,CUBEX360_RAM_CACHED_BASE);if(cubex_cpu_step(&cpu,&mem)!=CUBEX_CPU_OK||cpu.gpr[3]!=42u){cubex_memory_destroy(&mem);return 3;}printf("CubEX360 core smoke: r3=%u pc=%08x\n",(unsigned)cpu.gpr[3],(unsigned)cpu.pc);cubex_memory_destroy(&mem);return 0;}
