#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pico/malloc.h>
#include <hardware/sync.h>
#include <pr/task.h>
#include <pr/queue.h>

static spin_lock_t *lock = NULL;
static int lock_no = -1;

static
void lock_init(void)
{
    if (lock)
        return;

    lock_no = spin_lock_claim_unused(1);
    lock = spin_lock_instance(lock_no);
}

void pr_queue_init(struct pr_queue *queue,
                   size_t size)
{
    lock_init();
    
    queue->entry = malloc(size * sizeof(void*));
    assert(queue->entry);
    queue->size = size;
    queue->producer = NULL;
    queue->consumer = NULL;
    queue->wptr = 0;
    queue->rptr = 0;
    queue->free = size;
    queue->used = 0;
}

bool pr_queue_is_empty(struct pr_queue *queue)
{
    uint32_t irq;
    bool ret;

    irq = spin_lock_blocking(lock);
    ret = queue->used == 0;
    spin_unlock(lock, irq);

    return ret;
}

bool pr_queue_is_full(struct pr_queue *queue)
{
    uint32_t irq;
    bool ret;

    irq = spin_lock_blocking(lock);
    ret = queue->free == 0;
    spin_unlock(lock, irq);

    return ret;
}

size_t pr_queue_available(struct pr_queue *queue)
{
    uint32_t ret, irq;

    irq = spin_lock_blocking(lock);
    ret = queue->used;
    spin_unlock(lock, irq);

    return ret;
}

size_t pr_queue_free(struct pr_queue *queue)
{
    uint32_t ret, irq;

    irq = spin_lock_blocking(lock);
    ret = queue->free;
    spin_unlock(lock, irq);

    return ret;
}

void pr_queue_consumer_set(struct pr_queue *queue,
                            struct pr_task *task)
{
    queue->consumer = task;
}

void pr_queue_producer_set(struct pr_queue *queue,
                            struct pr_task *task)
{
    queue->producer = task;
}

pr_error_t pr_queue_push(struct pr_queue *queue,
                         void *entry)
{
    uint32_t ret, irq;

    irq = spin_lock_blocking(lock);

    if (queue->free) {
        queue->free--;
        queue->used++;
        queue->entry[queue->wptr++] = entry;
        if (queue->wptr >= queue->size)
            queue->wptr = 0;
        ret = 0;
    } else {
        ret = PR_ERR_MEMORY;
    }
    
    spin_unlock(lock, irq);

    if (!ret)
        pr_task_exec(queue->consumer);
    
    return ret;
}

pr_error_t pr_queue_pop(struct pr_queue *queue,
                        void **entry)
{
    uint32_t ret, irq;

    irq = spin_lock_blocking(lock);

    if (queue->used) {
        queue->free++;
        queue->used--;
        *entry = queue->entry[queue->rptr++];
        if (queue->rptr >= queue->size)
            queue->rptr = 0;
        ret = 0;
    } else {
        ret = PR_ERR_MEMORY;
    }
    
    spin_unlock(lock, irq);

    if (!ret)
        pr_task_exec(queue->producer);

    return ret;
}

