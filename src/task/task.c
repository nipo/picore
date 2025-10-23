#include <stdlib.h>
#include <hardware/sync.h>
#include <pr/task.h>
#include <pico/time.h>

#define dprintf(...) do{}while(0)
static
void queue_reschedule(struct pr_task_queue *queue);

void pr_task_init(struct pr_task *task,
                struct pr_task_queue *queue,
                pr_task_handler_t *handler)
{
    task->handler = handler;
    task->queue = queue;
    task->next = NULL;
    task->flags = 0;
    task->exec_at = 0;
}

static
void pr_task_wait_queue_remove(struct pr_task_queue *queue, struct pr_task *task)
{
    assert(queue->waiting);
    
    struct pr_task **cur = &queue->waiting;
    while (*cur) {
        if (*cur == task) {
            *cur = task->next;
            task->next = NULL;
            task->flags &= ~PR_TASK_FLAG_WAITING;
            task->exec_at = 0;
            return;
        }
        cur = &(*cur)->next;
    }
}

static
struct pr_task *pr_task_wait_queue_pop(struct pr_task_queue *queue)
{
    struct pr_task *task = queue->waiting;

    if (!task)
        return NULL;

    queue->waiting = task->next;
    task->next = NULL;
    task->flags &= ~PR_TASK_FLAG_WAITING;
    task->exec_at = 0;

    return task;
}

static
void pr_task_wait_queue_insert(struct pr_task_queue *queue,
                               struct pr_task *task,
                               absolute_time_t at)
{
    struct pr_task **cur = &queue->waiting;
    while (*cur && (*cur)->exec_at < at) {
        cur = &(*cur)->next;
    }

    task->exec_at = at;
    task->flags |= PR_TASK_FLAG_WAITING;
    task->next = *cur;
    *cur = task;
}

static
struct pr_task *pr_task_pending_queue_pop(struct pr_task_queue *queue)
{
    struct pr_task *task = queue->first;

    if (!task)
        return NULL;

    queue->first = task->next;
    if (queue->last == task)
        queue->last = NULL;
    task->next = NULL;
    task->flags &= ~PR_TASK_FLAG_PENDING;

    return task;
}

static
void pr_task_pending_queue_insert(struct pr_task_queue *queue,
                                  struct pr_task *task)
{
    task->flags |= PR_TASK_FLAG_PENDING;
    task->next = NULL;
    if (queue->last)
        queue->last->next = task;
    queue->last = task;
    if (!queue->first)
        queue->first = task;
}

static
bool is_waiting(struct pr_task *task)
{
    return !!(task->flags & PR_TASK_FLAG_WAITING);
}

static
bool is_pending(struct pr_task *task)
{
    return !!(task->flags & PR_TASK_FLAG_PENDING);
}

int pr_task_exec(struct pr_task *task)
{
    if (!task || !task->handler)
        return -1;

    struct pr_task_queue *queue = task->queue;

    if (!queue)
        return -1;

    bool reschedule = false;
    uint32_t irq = spin_lock_blocking(queue->lock);

    if (!is_pending(task)) {
        if (is_waiting(task)) {
            pr_task_wait_queue_remove(queue, task);
            reschedule = true;
        }

        pr_task_pending_queue_insert(queue, task);
    }
    
    spin_unlock(queue->lock, irq);

    if (reschedule)
        queue_reschedule(queue);    
    return 0;
}

void pr_task_queue_init(struct pr_task_queue *queue)
{
    alarm_pool_init_default();

    queue->first = NULL;
    queue->last = NULL;
    queue->waiting = NULL;
    queue->lock_no = spin_lock_claim_unused(1);
    queue->lock = spin_lock_instance(queue->lock_no);
}

void pr_task_queue_run_until_empty(struct pr_task_queue *queue)
{
    for (;;) {
        uint32_t irq = spin_lock_blocking(queue->lock);
        struct pr_task *task = pr_task_pending_queue_pop(queue);
        spin_unlock(queue->lock, irq);

        if (!task)
            break;

        dprintf("pt %p\n", task->handler);
        task->handler(task);
    }
}

static
void queue_waiting_process(struct pr_task_queue *queue)
{
    absolute_time_t now = get_absolute_time();

    uint32_t irq = spin_lock_blocking(queue->lock);
    while (queue->waiting && queue->waiting->exec_at <= now)
        pr_task_pending_queue_insert(queue, pr_task_wait_queue_pop(queue));
    spin_unlock(queue->lock, irq);

    queue_reschedule(queue);
}

static
int64_t ptask_alarm(alarm_id_t id, void *user_data)
{
    struct pr_task_queue *queue = user_data;
    queue->alarm = -1;
    queue_waiting_process(queue);

    return 0;
}

static
void queue_reschedule(struct pr_task_queue *queue)
{
    if (queue->alarm >= 0) {
        cancel_alarm(queue->alarm);
        queue->alarm = -1;
    }

    if (!queue->waiting)
        return;
    
    uint32_t irq = spin_lock_blocking(queue->lock);
    queue->alarm = add_alarm_at(queue->waiting->exec_at, ptask_alarm, queue, true);
    spin_unlock(queue->lock, irq);
}

void pr_task_exec_in_us(struct pr_task *task, uint64_t us)
{
    struct pr_task_queue *queue = task->queue;

    if (is_pending(task))
        return;
    
    if (is_waiting(task))
        pr_task_wait_queue_remove(queue, task);

    pr_task_wait_queue_insert(queue, task, make_timeout_time_us(us));

    queue_reschedule(queue);
}

void pr_task_exec_in_ms(struct pr_task *task, uint64_t ms)
{
    struct pr_task_queue *queue = task->queue;

    if (is_pending(task))
        return;

    if (is_waiting(task))
        pr_task_wait_queue_remove(queue, task);

    pr_task_wait_queue_insert(queue, task, make_timeout_time_ms(ms));

    queue_reschedule(queue);
}

bool pr_task_queue_is_empty(struct pr_task_queue *queue)
{
    bool empty;

    uint32_t irq = spin_lock_blocking(queue->lock);
    empty = !queue->first;
    spin_unlock(queue->lock, irq);

    return empty;
}

