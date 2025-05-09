#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pico/malloc.h>
#include <hardware/sync.h>
#include <pr/task.h>
#include <pr/fifo.h>

static int lock_no = -1;
spin_lock_t *lock;

void pr_fifo_init(struct pr_fifo *fifo,
               size_t size)
{
    fifo->buffer = malloc(size);
    assert(fifo->buffer);
    fifo->size = size;
    fifo->producer = NULL;
    fifo->consumer = NULL;
    fifo->wptr = 0;
    fifo->rptr = 0;
    fifo->free = size;
    fifo->used = 0;

    if (lock_no == -1) {
        lock_no = spin_lock_claim_unused(1);
        lock = spin_lock_instance(lock_no);
    }
}

void pr_fifo_clear(struct pr_fifo *fifo)
{
    uint32_t irq;

    irq = spin_lock_blocking(lock);
    fifo->used = 0;
    fifo->free = fifo->size;
    fifo->rptr = 0;
    fifo->wptr = 0;
    spin_unlock(lock, irq);
}

bool pr_fifo_is_empty(struct pr_fifo *fifo)
{
    uint32_t irq;
    bool ret;

    irq = spin_lock_blocking(lock);
    ret = fifo->used == 0;
    spin_unlock(lock, irq);

    return ret;
}

bool pr_fifo_is_full(struct pr_fifo *fifo)
{
    uint32_t irq;
    bool ret;

    irq = spin_lock_blocking(lock);
    ret = fifo->free == 0;
    spin_unlock(lock, irq);

    return ret;
}

size_t pr_fifo_available(struct pr_fifo *fifo)
{
    uint32_t ret, irq;

    irq = spin_lock_blocking(lock);
    ret = fifo->used;
    spin_unlock(lock, irq);

    return ret;
}

size_t pr_fifo_free(struct pr_fifo *fifo)
{
    uint32_t ret, irq;

    irq = spin_lock_blocking(lock);
    ret = fifo->free;
    spin_unlock(lock, irq);

    return ret;
}

void pr_fifo_consumer_set(struct pr_fifo *fifo,
                            struct pr_task *task)
{
    fifo->consumer = task;
}

void pr_fifo_producer_set(struct pr_fifo *fifo,
                            struct pr_task *task)
{
    fifo->producer = task;
}

size_t pr_fifo_read_prepare(struct pr_fifo *fifo,
                              size_t size,
                              const uint8_t **data)
{
    size_t rptr = 0;
    uint32_t irq;

    irq = spin_lock_blocking(lock);
    if (fifo->used < size)
        size = fifo->used;

    rptr = fifo->rptr;
    if (fifo->size - rptr < size)
        size = fifo->size - rptr;

    if (size) {
        fifo->used -= size;
        fifo->rptr += size;
        if (fifo->rptr == fifo->size)
            fifo->rptr = 0;
    }
    spin_unlock(lock, irq);
    
    *data = fifo->buffer + rptr;

    return size;
}

void pr_fifo_read_done(struct pr_fifo *fifo,
                         size_t size)
{
    struct pr_task *on_copy = NULL;
    uint32_t irq;

    if (!size)
        return;
    
    irq = spin_lock_blocking(lock);
    fifo->free += size;
    on_copy = fifo->producer;
    spin_unlock(lock, irq);

    pr_task_exec(on_copy);
}

size_t pr_fifo_write_prepare(struct pr_fifo *fifo,
                              size_t size,
                              uint8_t **data)
{
    size_t wptr = 0;
    uint32_t irq;

    irq = spin_lock_blocking(lock);
    if (fifo->free < size)
        size = fifo->free;

    wptr = fifo->wptr;
    if (fifo->size - wptr < size)
        size = fifo->size - wptr;

    if (size) {
        fifo->free -= size;
        fifo->wptr += size;
        if (fifo->wptr == fifo->size)
            fifo->wptr = 0;
    }
    spin_unlock(lock, irq);
    
    *data = fifo->buffer + wptr;

    return size;
}

void pr_fifo_write_done(struct pr_fifo *fifo,
                         size_t size)
{
    struct pr_task *on_copy = NULL;
    uint32_t irq;

    if (!size)
        return;
    
    irq = spin_lock_blocking(lock);
    fifo->used += size;
    on_copy = fifo->consumer;
    spin_unlock(lock, irq);

    pr_task_exec(on_copy);
}

size_t pr_fifo_write(struct pr_fifo *fifo,
                       const uint8_t *data,
                       size_t size)
{
    size_t done = 0, transferred;

    if (!size)
        return 0;
    
    do {
        uint8_t *dst;

        transferred = pr_fifo_write_prepare(fifo, size - done, &dst);

        if (transferred) {
            memcpy(dst, data + done, transferred);
            pr_fifo_write_done(fifo, transferred);
        }

        done += transferred;
    } while (transferred);

    
    
    return done;
}

size_t pr_fifo_read(struct pr_fifo *fifo,
                 uint8_t *data,
                 size_t size)
{
    size_t done = 0, transferred;

    if (!size)
        return 0;
    
    do {
        size_t transferred;
        const uint8_t *src;

        transferred = pr_fifo_read_prepare(fifo, size - done, &src);
        if (transferred) {
            memcpy(data + done, src, transferred);
            pr_fifo_read_done(fifo, transferred);
        }

        done += transferred;
    } while (transferred);

    return done;
}

size_t pr_fifo_peek(struct pr_fifo *fifo,
                      uint8_t *data,
                      size_t size)
{
    size_t rptr = 0;
    uint32_t irq;
    size_t s1, s2;

    irq = spin_lock_blocking(lock);
    if (fifo->used < size)
        size = fifo->used;

    s1 = size;
    s2 = 0;
    rptr = fifo->rptr;
    if (fifo->size - rptr < size) {
        s1 = fifo->size - rptr;
        s2 = size - s1;
    }

    if (s1)
        memcpy(data, fifo->buffer + rptr, s1);
    if (s2)
        memcpy(data + s1, fifo->buffer, s2);
    spin_unlock(lock, irq);

    return size;
}
