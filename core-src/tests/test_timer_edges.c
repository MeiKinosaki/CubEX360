#include "cubex360/timer.h"
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

int main(void) {
    cubex_timer_t t;

    /* One-shot: no event before deadline, exactly one at deadline, then stopped. */
    assert(cubex_timer_init(&t, 7u, false) == 0);
    assert(cubex_timer_enable(&t, true) == 0);
    assert(cubex_timer_advance(&t, 0u) == 0);
    assert(t.remaining == 7u && !t.interrupt_pending);
    assert(cubex_timer_advance(&t, 6u) == 0);
    assert(t.remaining == 1u && !t.interrupt_pending);
    assert(cubex_timer_advance(&t, 1u) == 0);
    assert(t.remaining == 0u && t.interrupt_pending && !t.enabled);
    /* Advancing a disabled one-shot must not create another event. */
    assert(cubex_timer_advance(&t, UINT32_MAX) == 0);
    assert(t.remaining == 0u && t.interrupt_pending && !t.enabled);

    /* Re-enabling restarts the configured period and clears no pending IRQ. */
    assert(cubex_timer_enable(&t, true) == 0);
    assert(t.remaining == 7u && t.interrupt_pending && t.enabled);
    cubex_timer_clear_interrupt(&t);
    assert(!t.interrupt_pending);

    /* Periodic timer: exact multiples and a large cycle batch. */
    assert(cubex_timer_init(&t, 5u, true) == 0);
    assert(cubex_timer_enable(&t, true) == 0);
    assert(cubex_timer_advance(&t, 5u) == 0);
    assert(t.interrupt_pending && t.enabled && t.remaining == 5u);
    cubex_timer_clear_interrupt(&t);
    assert(cubex_timer_advance(&t, 15u) == 0);
    assert(t.interrupt_pending && t.enabled && t.remaining == 5u);
    cubex_timer_clear_interrupt(&t);
    assert(cubex_timer_advance(&t, 17u) == 0);
    assert(t.interrupt_pending && t.enabled && t.remaining == 3u);

    /* UINT32_MAX must not overflow the periodic remainder calculation. */
    assert(cubex_timer_init(&t, 11u, true) == 0);
    assert(cubex_timer_enable(&t, true) == 0);
    assert(cubex_timer_advance(&t, UINT32_MAX) == 0);
    assert(t.interrupt_pending && t.enabled);
    assert(t.remaining >= 1u && t.remaining <= 11u);

    /* Pause preserves remaining time; resume follows documented primitive behavior. */
    assert(cubex_timer_init(&t, 9u, true) == 0);
    assert(cubex_timer_enable(&t, true) == 0);
    assert(cubex_timer_advance(&t, 4u) == 0);
    assert(t.remaining == 5u);
    assert(cubex_timer_enable(&t, false) == 0);
    assert(cubex_timer_advance(&t, 100u) == 0);
    assert(t.remaining == 5u);
    assert(cubex_timer_enable(&t, true) == 0);
    assert(t.remaining == 9u);

    return 0;
}
