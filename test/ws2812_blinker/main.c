#include <stdio.h>
#include <stdlib.h>
#include <pr/pixel_led.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/clocks.h>
#include <string.h>
#include <pr/pixel_led.h>

#define LED_PIN 1
#define LED_COUNT 6

struct app
{
    struct pr_task_queue task_queue;
    struct pr_pixel_led_strip led_strip;
    struct pr_task ticker;
    uint8_t last_color;
};

PR_TASK_STRUCT_COMPOSE(app, ticker);

static
void on_tick(struct pr_task *task)
{
    struct app *app = app_from_ticker(task);

    pr_task_exec_in_ms(&app->ticker, 200);

    for (size_t i = 0; i < LED_COUNT; ++i)
        pr_pixel_led_set_norefresh(&app->led_strip, i,
                                   (((app->last_color + i) & 1 ? 0x40 : 0) << 16)
                                   | (((app->last_color + i) & 2 ? 0x40 : 0) << 8)
                                   | ((app->last_color + i) & 4 ? 0x40 : 0)
            );
    pr_pixel_led_refresh(&app->led_strip);
    app->last_color++;
}

struct app app[1];

int main(void)
{
    memset(app, 0, sizeof(*app));

    set_sys_clock_khz(125000, false);
    stdio_init_all();

    pr_task_queue_init(&app->task_queue);
    pr_task_init(&app->ticker, &app->task_queue, on_tick);
    pr_pixel_led_strip_init(&app->led_strip, &app->task_queue,
                            1, 0,
                            LED_COUNT, LED_PIN, false);

    pr_task_exec(&app->ticker);
    
    for (;;) {
        pr_task_queue_run_until_empty(&app->task_queue);
    }
    
    return 0;
}
