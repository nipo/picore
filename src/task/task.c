#include <stdlib.h>
#include <hardware/sync.h>
#include <pr/task.h>
#include <pico/time.h>

#define dprintf(...) do{}while(0)

void pr_task_init(struct pr_task *task,
                struct pr_task_queue *queue,
                pr_task_handler_t *handler)
{
    task->handler = handler;
    task->queue = queue;
    task->next = NULL;
    task->flags = 0;
    task->alarm = -1;
}

int pr_task_exec(struct pr_task *task)
{
    if (!task || !task->handler) {
        return -1;
    }

    struct pr_task_queue *queue = task->queue;

    if (!queue) {
        return -1;
    }

    uint32_t irq = spin_lock_blocking(queue->lock);

    bool is_pending = (task->flags & PR_TASK_FLAG_PENDING);
        
    if (!is_pending) {
        task->flags |= PR_TASK_FLAG_PENDING;
        task->next = NULL;
        if (queue->last)
            queue->last->next = task;
        queue->last = task;
        if (!queue->first)
            queue->first = task;
    }

    spin_unlock(queue->lock, irq);

    return 0;
}

void pr_task_queue_init(struct pr_task_queue *queue)
{
    alarm_pool_init_default();

    queue->first = NULL;
    queue->last = NULL;
    queue->lock_no = spin_lock_claim_unused(1);
    queue->lock = spin_lock_instance(queue->lock_no);
}

void pr_task_queue_run_until_empty(struct pr_task_queue *queue)
{
    struct pr_task *task;
    bool empty = false;

    while (!empty) {
        uint32_t irq = spin_lock_blocking(queue->lock);

        empty = true;
        task = queue->first;
        if (task) {
            queue->first = task->next;
            empty = !!queue->first;
            if (queue->last == task)
                queue->last = NULL;
            task->next = NULL;
            task->flags &= ~PR_TASK_FLAG_PENDING;
        }

        spin_unlock(queue->lock, irq);

        if (!task)
            break;

        dprintf("pt %p\n", task->handler);
        task->handler(task);
    }
}

static int64_t ptask_alarm(alarm_id_t id, void *user_data)
{
    struct pr_task *t = user_data;
    t->alarm = -1;
    pr_task_exec(t);

    return 0;
}

void pr_task_exec_in_us(struct pr_task *t, uint64_t us)
{
    if (t->alarm >= 0)
        cancel_alarm(t->alarm);

    t->alarm = add_alarm_in_us(us, ptask_alarm, t, 1);
}

void pr_task_exec_in_ms(struct pr_task *t, uint64_t ms)
{
    if (t->alarm >= 0)
        cancel_alarm(t->alarm);

    t->alarm = add_alarm_in_ms(ms, ptask_alarm, t, 1);
}

bool pr_task_queue_is_empty(struct pr_task_queue *queue)
{
    bool empty;

    uint32_t irq = spin_lock_blocking(queue->lock);
    empty = !queue->first;
    spin_unlock(queue->lock, irq);

    return empty;
}

