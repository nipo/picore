#ifndef PR_PIXEL_LED_H
#define PR_PIXEL_LED_H

#include <pico/stdlib.h>
#include <pr/task.h>
#include <hardware/pio.h>

struct pr_pixel_led_strip
{
    PIO pio;
    uint8_t sm;
    struct pr_task_queue *task_queue;
    struct pr_task refresh_task;
    struct pr_task done_task;
    uint32_t *pixel_data;
    size_t pixel_count;
    size_t offset;
    bool refreshing, dirty, inv;
};

PR_TASK_STRUCT_COMPOSE(pr_pixel_led_strip, refresh_task);
PR_TASK_STRUCT_COMPOSE(pr_pixel_led_strip, done_task);

void pr_pixel_led_strip_init(struct pr_pixel_led_strip *strip,
                             struct pr_task_queue *queue,
                             uint8_t pio_index, uint8_t sm_index,
                             size_t led_count, uint pin_data, bool inv);

void pr_pixel_led_refresh(struct pr_pixel_led_strip *strip);

static inline
void pr_pixel_led_set_norefresh(struct pr_pixel_led_strip *strip,
                                size_t offset, uint32_t value)
{
    assert(offset < strip->pixel_count);
    strip->dirty = true;
    strip->pixel_data[offset] = value;
}

static inline
void pr_pixel_led_set(struct pr_pixel_led_strip *strip,
                      size_t offset, uint32_t value)
{
    pr_pixel_led_set_norefresh(strip, offset, value);
    pr_pixel_led_refresh(strip);
}

static inline
void pr_pixel_led_set_many(struct pr_pixel_led_strip *strip,
                           size_t offset, const uint32_t *value, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        pr_pixel_led_set_norefresh(strip, offset+i, value[i]);
    pr_pixel_led_refresh(strip);
}

#endif
