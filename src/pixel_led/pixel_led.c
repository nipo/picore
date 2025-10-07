#include <pico/stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pr/task.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <pr/pixel_led.h>
#include "pixel_led.pio.h"

#define dprintf(...) do{}while(0)

static struct pr_pixel_led_strip *g_instance[2][4] = {};

static
void on_pixel_led_irq(struct pr_pixel_led_strip *strip)
{
    bool done = false;

    while (!pio_sm_is_tx_fifo_full(strip->pio, strip->sm)) {
        if (strip->offset < strip->pixel_count) {
            uint32_t val = strip->pixel_data[strip->offset++] << 8;
            if (strip->inv)
                val = ~val;

            pio_sm_put(strip->pio, strip->sm, val);
        } else {
            pio_sm_put(strip->pio, strip->sm, 0);

            done = true;
            break;
        }
    }

    if (done) {
        pr_task_exec_in_us(&strip->done_task, 50);

        pio_set_irq0_source_enabled(
            strip->pio,
            pio_get_tx_fifo_not_full_interrupt_source(strip->sm),
            false);
    } else {
        pio_set_irq0_source_enabled(
            strip->pio,
            pio_get_tx_fifo_not_full_interrupt_source(strip->sm),
            true);
    }
}

static
void on_pio_irq(uint8_t pio_index, PIO pio)
{
    for (uint8_t sm = 0; sm < 4; ++sm) {
        struct pr_pixel_led_strip *strip = g_instance[pio_index][sm];

        pio_interrupt_clear(pio, pio_get_tx_fifo_not_full_interrupt_source(sm));
        if (strip) {
            on_pixel_led_irq(strip);
        }
    }
}

static
void on_pio0_irq0(void)
{
    on_pio_irq(0, pio0);
}

static
void on_pio1_irq0(void)
{
    on_pio_irq(1, pio1);
}

static
void on_refresh(struct pr_task *task)
{
    struct pr_pixel_led_strip *strip = pr_pixel_led_strip_from_refresh_task(task);

    dprintf("On refresh %d %d %d\n", pio_get_index(strip->pio), strip->sm, strip->refreshing);
    
    if (strip->refreshing)
        return;
    strip->refreshing = true;

    for (size_t i = 0; i < strip->pixel_count; ++i) {
        if ((i & 0x7) == 0)
            dprintf("\n%03x:", i);
        dprintf(" %06x", strip->pixel_data[i]);
    }
    dprintf("\n");

    strip->dirty = false;
    strip->offset = 0;
    pio_set_irq0_source_enabled(
        strip->pio,
        pio_get_tx_fifo_not_full_interrupt_source(strip->sm),
        true);
}

static
void on_done(struct pr_task *task)
{
    struct pr_pixel_led_strip *strip = pr_pixel_led_strip_from_done_task(task);

    dprintf("On done %d %d\n", pio_get_index(strip->pio), strip->sm);

    strip->refreshing = false;

    if (strip->dirty)
        pr_task_exec(&strip->refresh_task);
}

void pr_pixel_led_strip_init(struct pr_pixel_led_strip *strip,
                             struct pr_task_queue *queue,
                             uint8_t pio_index, uint8_t sm,
                             size_t led_count, uint pin_data,
                             bool inv)
{
    memset(strip, 0, sizeof(*strip));

    hard_assert(pio_index < 2);
    hard_assert(sm < 4);
    
    strip->pio = pio_index ? pio1 : pio0;
    strip->sm = sm;

    pr_task_init(&strip->refresh_task, queue, on_refresh);
    pr_task_init(&strip->done_task, queue, on_done);

    strip->pixel_data = malloc(4 * led_count);
    hard_assert(strip->pixel_data);

    memset(strip->pixel_data, 0, 4 * led_count);

    strip->pixel_count = led_count;
    strip->offset = 0;
    strip->inv = inv;

    g_instance[pio_index][strip->sm] = strip;

    if (sm == 0) {
        pixel_led_pio_init(strip->pio);
    }

    
    pio_set_irq0_source_enabled(
        strip->pio,
        pio_get_tx_fifo_not_full_interrupt_source(strip->sm),
        false);

    pixel_led_sm_init(strip->pio, strip->sm, pin_data, inv);
    pio_sm_restart(strip->pio, sm);
    pio_sm_set_enabled(strip->pio, sm, true);

    pr_task_exec(&strip->refresh_task);

    if (sm == 0) {
        int irq = pio_index ? PIO1_IRQ_0 : PIO0_IRQ_0;

        /* irq_add_shared_handler(irq, */
        /*                        pio_index ? on_pio1_irq0 : on_pio0_irq0, */
        /*                        PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY); */
        irq_set_exclusive_handler(irq,
                                  pio_index ? on_pio1_irq0 : on_pio0_irq0);
        irq_set_enabled(irq, 1);
    }
}

void pr_pixel_led_refresh(struct pr_pixel_led_strip *strip)
{
    pr_task_exec(&strip->refresh_task);
}
