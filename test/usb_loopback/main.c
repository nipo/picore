#include <hardware/irq.h>
#include <hardware/structs/sio.h>
#include <pico/stdlib.h>
#include <string.h>
#include <tusb.h>
#include <pr/tiny_usb.h>
#include <pr/cdc_interface.h>
#include <pr/fifo.h>
#include <pr/task.h>

struct app_context
{
    struct pr_task_queue queue;
    struct pr_fifo fifo;
    struct pr_tiny_usb usb;
    struct pr_cdc_interface cdc;
    struct pr_task cdc_event;
};

PR_TASK_STRUCT_COMPOSE(app_context, cdc_event);

static
void on_cdc_event(struct pr_task *cdc_event)
{
    struct app_context *app = app_context_from_cdc_event(cdc_event);

#if defined(PICO_DEFAULT_LED_PIN)
    if (app->cdc.events & PR_CDC_INTERFACE_EVENT_TX) {
        gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
    }
#endif

    app->cdc.events = 0;
}

int main(void)
{
    struct app_context app[1];

    set_sys_clock_khz(125000, false);

#if defined(PICO_DEFAULT_LED_PIN)
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, 1);
#endif

    pr_task_queue_init(&app->queue);
    pr_task_init(&app->cdc_event, &app->queue, on_cdc_event);
    pr_fifo_init(&app->fifo, 8192);
    pr_tiny_usb_init(&app->usb, &app->queue);
    pr_cdc_interface_init(&app->cdc, &app->queue, 0, &app->fifo, &app->fifo, &app->cdc_event);

    for (;;) {
        pr_task_queue_run_until_empty(&app->queue);
    }
    
    return 0;
}
