#ifndef CUBEX360_CPU_H
#define CUBEX360_CPU_H
#include "cubex360/bus.h"
typedef struct cubex_cpu_state { uint32_t gpr[32]; uint32_t pc,lr,ctr,cr,xer,msr; uint64_t retired_instructions; } cubex_cpu_state_t;
typedef enum cubex_cpu_result { CUBEX_CPU_OK=0,CUBEX_CPU_UNSUPPORTED=1,CUBEX_CPU_MEMORY_FAULT=2,CUBEX_CPU_BAD_ARGUMENT=3 } cubex_cpu_result_t;
void cubex_cpu_reset(cubex_cpu_state_t *cpu,uint32_t entry_pc);
cubex_cpu_result_t cubex_cpu_step(cubex_cpu_state_t *cpu,cubex_memory_t *mem);
#endif
