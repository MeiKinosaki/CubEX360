#include "cubex360/timer.h"

int cubex_timer_init(cubex_timer_t *timer, uint32_t period, bool periodic) {
    if (!timer || period == 0u) return -1;
    timer->period = period;
    timer->remaining = period;
    timer->enabled = false;
    timer->periodic = periodic;
    timer->interrupt_pending = false;
    return 0;
}

int cubex_timer_enable(cubex_timer_t *timer, bool enabled) {
    if (!timer || timer->period == 0u) return -1;
    if (enabled && !timer->enabled) timer->remaining = timer->period;
    timer->enabled = enabled;
    return 0;
}

int cubex_timer_advance(cubex_timer_t *timer, uint32_t cycles) {
    uint32_t after_expiry;
    if (!timer || timer->period == 0u || timer->remaining == 0u) return -1;
    if (!timer->enabled || cycles == 0u) return 0;
    if (cycles < timer->remaining) {
        timer->remaining -= cycles;
        return 0;
    }

    after_expiry = cycles - timer->remaining;
    timer->interrupt_pending = true;
    if (!timer->periodic) {
        timer->remaining = 0u;
        timer->enabled = false;
        return 0;
    }

    /* Reduce by period before adding, avoiding arithmetic overflow. */
    after_expiry %= timer->period;
    timer->remaining = after_expiry == 0u ? timer->period
                                          : timer->period - after_expiry;
    return 0;
}

void cubex_timer_clear_interrupt(cubex_timer_t *timer) {
    if (timer) timer->interrupt_pending = false;
}
