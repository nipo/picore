#ifndef PR_BUTTON_H
#define PR_BUTTON_H

#include <pico/stdlib.h>
#include <pr/task.h>

struct pr_button
{
    struct pr_task worker;
    struct pr_task *wake;
    uint8_t io;
    bool active_high, last_value;
    uint32_t mask;
};

PR_TASK_STRUCT_COMPOSE(pr_button, worker);

void pr_button_init(struct pr_button *button,
                    struct pr_task_queue *queue,
                    struct pr_task *wake,
                    uint pin, bool active_high);

#endif
