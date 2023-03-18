#ifndef PR_FIFO_H_
#define PR_FIFO_H_

#include <stdint.h>
#include <stdlib.h>
#include <hardware/sync.h>
#include <pr/task.h>

struct pr_fifo
{
    struct pr_task *consumer, *producer;
    uint8_t *buffer;
    uint32_t size;
    uint32_t wptr, rptr, free, used;
    spin_lock_t *lock;
    int lock_no;
};

void pr_fifo_init(struct pr_fifo *fifo,
                  size_t size);

void pr_fifo_clear(struct pr_fifo *fifo);

bool pr_fifo_is_empty(struct pr_fifo *fifo);
bool pr_fifo_is_full(struct pr_fifo *fifo);

size_t pr_fifo_available(struct pr_fifo *fifo);
size_t pr_fifo_free(struct pr_fifo *fifo);

void pr_fifo_consumer_set(struct pr_fifo *fifo,
                          struct pr_task *task);

void pr_fifo_producer_set(struct pr_fifo *fifo,
                          struct pr_task *task);

size_t pr_fifo_write(struct pr_fifo *fifo,
                     const uint8_t *data,
                     size_t size);

size_t pr_fifo_read(struct pr_fifo *fifo,
                    uint8_t *data,
                    size_t size);

size_t pr_fifo_peek(struct pr_fifo *fifo,
                    uint8_t *data,
                    size_t size);

size_t pr_fifo_read_prepare(struct pr_fifo *fifo,
                            size_t size,
                            const uint8_t **data);

void pr_fifo_read_done(struct pr_fifo *fifo,
                       size_t size);

size_t pr_fifo_write_prepare(struct pr_fifo *fifo,
                             size_t size,
                             uint8_t **data);

void pr_fifo_write_done(struct pr_fifo *fifo,
                        size_t size);

#endif
