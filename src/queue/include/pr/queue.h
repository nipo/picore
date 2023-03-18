#ifndef PR_QUEUE_H_
#define PR_QUEUE_H_

#include <stdint.h>
#include <stdlib.h>
#include <hardware/sync.h>
#include <pr/task.h>
#include <pr/error.h>

struct pr_queue
{
    struct pr_task *consumer, *producer;
    void **entry;
    uint32_t size;
    uint32_t wptr, rptr, free, used;
    spin_lock_t *lock;
    int lock_no;
};

void pr_queue_init(struct pr_queue *queue,
                   size_t size);

bool pr_queue_is_empty(struct pr_queue *queue);
bool pr_queue_is_full(struct pr_queue *queue);

size_t pr_queue_available(struct pr_queue *queue);
size_t pr_queue_free(struct pr_queue *queue);

void pr_queue_consumer_set(struct pr_queue *queue,
                           struct pr_task *task);

void pr_queue_producer_set(struct pr_queue *queue,
                           struct pr_task *task);

pr_error_t pr_queue_prsh(struct pr_queue *queue,
                         void *entry);

pr_error_t pr_queue_pop(struct pr_queue *queue,
                        void **entry);

#endif
