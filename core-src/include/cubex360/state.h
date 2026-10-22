#ifndef CUBEX360_STATE_H
#define CUBEX360_STATE_H
#include "cubex360/cpu.h"
/* Versioned host-side snapshot of CPU state + the current 24 MiB main RAM.
 * Returns 0 on success; invalid/truncated/corrupt/incompatible files fail closed.
 * This does not yet snapshot GPU, DSP, timers, disc or other devices. */
int cubex_state_save(const char *path, const cubex_cpu_state_t *cpu, const cubex_memory_t *mem);
int cubex_state_load(const char *path, cubex_cpu_state_t *cpu, cubex_memory_t *mem);
#endif
