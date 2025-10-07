#include <pico/stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pr/task.h>
#include <hardware/gpio.h>
#include <hardware/clocks.h>
#include <pr/button.h>

#define dprintf(...) do{}while(0)

static
void on_work(struct pr_task *task)
{
    struct pr_button *button = pr_button_from_worker(task);

    bool value = gpio_get(button->io);
    if (value != button->last_value) {
        button->last_value = value;
    
        if (value == button->active_high)
            pr_task_exec(button->wake);
    }

    pr_task_exec(&button->worker);
}

void pr_button_init(struct pr_button *button,
                    struct pr_task_queue *queue,
                    struct pr_task *wake,
                    uint pin, bool active_high)
{
    memset(button, 0, sizeof(*button));

    pr_task_init(&button->worker, queue, on_work);
    button->io = pin;
    button->last_value = gpio_get(button->io);
    button->active_high = active_high;
    button->mask = 0;
    button->wake = wake;

    pr_task_exec(&button->worker);
}
