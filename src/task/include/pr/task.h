#ifndef PR_TASK_H_
#define PR_TASK_H_

#include <stdint.h>
#include <hardware/sync.h>
#include <pico/time.h>
#include <pr/struct_compose.h>

struct pr_task;
struct pr_task_queue;

typedef void pr_task_handler_t(struct pr_task *);

#define PR_TASK_STRUCT_COMPOSE(struct_name, field)                        \
    PR_STRUCT_COMPOSE(struct_name, pr_task, field)

#define PR_TASK_FLAG_PENDING 1

struct pr_task
{
    struct pr_task *next;
    uint32_t flags;
    alarm_id_t alarm;
    struct pr_task_queue *queue;
    pr_task_handler_t *handler;
};

struct pr_task_queue
{
    struct pr_task *first;
    struct pr_task *last;
    spin_lock_t *lock;
    int lock_no;
};

void pr_task_init(struct pr_task *task,
                     struct pr_task_queue *queue,
                     pr_task_handler_t *handler);

int pr_task_exec(struct pr_task *t);

void pr_task_exec_in_us(struct pr_task *t, uint64_t us);

void pr_task_exec_in_ms(struct pr_task *t, uint64_t ms);

int pr_task_cancel(struct pr_task *t);

void pr_task_queue_init(struct pr_task_queue *q);

void pr_task_queue_run_until_empty(struct pr_task_queue *q);

#endif
