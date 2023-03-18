#ifndef PR_TINY_USB_H_
#define PR_TINY_USB_H_

#include <stdint.h>
#include <stdlib.h>
#include <pr/task.h>

struct pr_tiny_usb
{
    struct pr_task runner;
};

PR_TASK_STRUCT_COMPOSE(pr_tiny_usb, runner)

void pr_tiny_usb_init(struct pr_tiny_usb *tiny_usb,
                      struct pr_task_queue *queue);

uint16_t* pr_usb_serial_number(void);

#define PR_TINY_USB_IMPL                                                \
    static                                                              \
    void tiny_usb_runner(struct pr_task *runner)                        \
    {                                                                   \
        struct pr_tiny_usb *context = pr_tiny_usb_from_runner(runner);  \
                                                                        \
        tud_task();                                                     \
                                                                        \
        pr_task_exec(&context->runner);                                 \
    }                                                                   \
                                                                        \
    void pr_tiny_usb_init(struct pr_tiny_usb *context,                  \
                          struct pr_task_queue *queue)                  \
    {                                                                   \
        tusb_init();                                                    \
                                                                        \
        pr_task_init(&context->runner, queue, tiny_usb_runner);         \
        pr_task_exec(&context->runner);                                 \
    }                                                                   \

#endif
