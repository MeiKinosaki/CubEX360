#ifndef CUBEX360_TIMER_H
#define CUBEX360_TIMER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Deterministic timer primitive for device prototypes; not a GC hardware timer map. */
typedef struct cubex_timer {
    uint32_t period;
    uint32_t remaining;
    bool enabled;
    bool periodic;
    bool interrupt_pending;
} cubex_timer_t;

int cubex_timer_init(cubex_timer_t *timer, uint32_t period, bool periodic);
int cubex_timer_enable(cubex_timer_t *timer, bool enabled);
int cubex_timer_advance(cubex_timer_t *timer, uint32_t cycles);
void cubex_timer_clear_interrupt(cubex_timer_t *timer);

#ifdef __cplusplus
}
#endif
#endif
