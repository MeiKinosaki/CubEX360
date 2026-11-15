#ifndef CUBEX360_CPU_H
#define CUBEX360_CPU_H
#include "cubex360/bus.h"
typedef struct cubex_cpu_state { uint32_t gpr[32], sr[16], ibat[8], dbat[8], sdr1; uint64_t fpr[32]; uint32_t fpscr; uint32_t pc,lr,ctr,cr,xer,msr,srr0,srr1,dec; uint32_t dec_pending,dar,dsisr; uint64_t retired_instructions; } cubex_cpu_state_t;
typedef enum cubex_cpu_result { CUBEX_CPU_OK=0,CUBEX_CPU_UNSUPPORTED=1,CUBEX_CPU_MEMORY_FAULT=2,CUBEX_CPU_BAD_ARGUMENT=3 } cubex_cpu_result_t;
void cubex_cpu_reset(cubex_cpu_state_t *cpu,uint32_t entry_pc);
int cubex_cpu_translate_bat(const cubex_cpu_state_t *cpu,uint32_t effective_address,int is_instruction,int user_mode,uint32_t *physical_address);
int cubex_cpu_translate_hash(const cubex_cpu_state_t *cpu,const cubex_memory_t *mem,uint32_t effective_address,int is_instruction,int is_store,uint32_t *physical_address);
int cubex_cpu_take_external_interrupt(cubex_cpu_state_t *cpu);
int cubex_cpu_take_decrementer_interrupt(cubex_cpu_state_t *cpu);
cubex_cpu_result_t cubex_cpu_step(cubex_cpu_state_t *cpu,cubex_memory_t *mem);
cubex_cpu_result_t cubex_cpu_run(cubex_cpu_state_t *cpu,cubex_memory_t *mem,uint64_t instruction_budget);
#endif
