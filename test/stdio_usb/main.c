#include <pico/stdlib.h>
#include <hardware/clocks.h>
#include <string.h>
#include <tusb.h>
#include <pr/tiny_usb.h>
#include <pr/cdc_interface.h>
#include <pr/fifo.h>
#include <pr/task.h>
#include <pr/stdio_fifo.h>

struct app
{
    struct pr_task_queue task_queue;
    struct pr_tiny_usb usb;
    struct pr_cdc_interface cdc;
    struct pr_fifo cdc_out, cdc_in;
    struct pr_task cdc_event;
    struct pr_task ticker;
    uint32_t counter;
};

PR_TASK_STRUCT_COMPOSE(app, cdc_event);
PR_TASK_STRUCT_COMPOSE(app, ticker);

static
void on_cdc_event(struct pr_task *cdc_event)
{
    struct app *app = app_from_cdc_event(cdc_event);

    app->cdc.events = 0;
}

static
void on_tick(struct pr_task *task)
{
    struct app *app = app_from_ticker(task);

    pr_task_exec_in_ms(&app->ticker, 1000);

    printf("Hello %d\n", app->counter);

    app->counter++;
}

struct app app[1];

int main(void)
{
    set_sys_clock_khz(125000, false);

    stdio_init_all();

    pr_task_queue_init(&app->task_queue);
    pr_task_init(&app->cdc_event, &app->task_queue, on_cdc_event);
    pr_task_init(&app->ticker, &app->task_queue, on_tick);
    pr_fifo_init(&app->cdc_out, 8192);
    pr_fifo_init(&app->cdc_in, 8192);
    pr_tiny_usb_init(&app->usb, &app->task_queue);
    pr_cdc_interface_init(&app->cdc, &app->task_queue, 0,
                          &app->cdc_in, &app->cdc_out, &app->cdc_event);

    pr_stdio_fifo_driver_bind(&app->cdc_in, &app->cdc_out);

    pr_task_exec(&app->ticker);

    for (;;) {
        pr_task_queue_run_until_empty(&app->task_queue);
    }
    
    return 0;
}
