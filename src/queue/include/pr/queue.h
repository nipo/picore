/**
 * @file queue.h
 * @defgroup pr_queue Pointer Queue
 * @brief Task-integrated ring buffer for passing structure pointers
 *
 * The queue library provides circular buffers that exchange pointers to
 * arbitrary structures instead of byte streams. Like FIFOs, queues support
 * automatic task notification when items become available or space becomes free.
 *
 * Key features:
 * - Ring buffer for pointer passing between tasks
 * - Automatic task scheduling on data/space availability
 * - Zero-copy message passing
 * - Thread-safe operations
 * - Suitable for event passing and message queues
 *
 * Typical usage pattern:
 * @code
 * struct my_message {
 *     int type;
 *     void *data;
 * };
 *
 * struct pr_queue queue;
 * struct pr_task consumer_task;
 *
 * void consumer_handler(struct pr_task *task) {
 *     struct my_message *msg;
 *     if (pr_queue_pop(&queue, (void **)&msg) == PR_OK) {
 *         // Process message...
 *         free(msg);
 *     }
 * }
 *
 * int main(void) {
 *     pr_queue_init(&queue, 16); // 16 pointers
 *     pr_task_init(&consumer_task, &task_queue, consumer_handler);
 *     pr_queue_consumer_set(&queue, &consumer_task);
 *
 *     // Producer pushes messages, consumer_task is automatically scheduled
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_QUEUE_H_
#define PR_QUEUE_H_

#include <stdint.h>
#include <stdlib.h>
#include <hardware/sync.h>
#include <pr/task.h>
#include <pr/error.h>

/**
 * @brief Pointer queue structure
 *
 * Circular buffer for passing pointers between producers and consumers.
 * Can automatically schedule consumer and producer tasks when items
 * or space becomes available.
 */
struct pr_queue
{
    struct pr_task *consumer;   /**< Task to schedule when items become available */
    struct pr_task *producer;   /**< Task to schedule when space becomes available */
    void **entry;               /**< Internal array of pointers */
    uint32_t size;              /**< Total queue capacity (number of pointers) */
    uint32_t wptr;              /**< Write pointer position */
    uint32_t rptr;              /**< Read pointer position */
    uint32_t free;              /**< Number of free slots */
    uint32_t used;              /**< Number of used slots */
};

/**
 * @brief Initialize a pointer queue
 *
 * Allocates and initializes a queue with the specified number of pointer slots.
 * The queue storage is allocated using malloc().
 *
 * @param queue Pointer to the queue structure to initialize
 * @param size Number of pointer slots in the queue
 */
void pr_queue_init(struct pr_queue *queue,
                   size_t size);

/**
 * @brief Check if queue is empty
 *
 * @param queue Pointer to the queue
 * @return true if queue contains no items, false otherwise
 */
bool pr_queue_is_empty(struct pr_queue *queue);

/**
 * @brief Check if queue is full
 *
 * @param queue Pointer to the queue
 * @return true if queue has no free slots, false otherwise
 */
bool pr_queue_is_full(struct pr_queue *queue);

/**
 * @brief Get number of items available in queue
 *
 * @param queue Pointer to the queue
 * @return Number of items that can be popped from the queue
 */
size_t pr_queue_available(struct pr_queue *queue);

/**
 * @brief Get number of free slots in queue
 *
 * @param queue Pointer to the queue
 * @return Number of items that can be pushed to the queue
 */
size_t pr_queue_free(struct pr_queue *queue);

/**
 * @brief Set consumer task for automatic scheduling
 *
 * When an item is pushed to the queue, the consumer task will be automatically
 * scheduled for execution. Set to NULL to disable automatic scheduling.
 *
 * @param queue Pointer to the queue
 * @param task Pointer to the consumer task, or NULL
 */
void pr_queue_consumer_set(struct pr_queue *queue,
                           struct pr_task *task);

/**
 * @brief Set producer task for automatic scheduling
 *
 * When an item is popped from the queue (freeing space), the producer task will
 * be automatically scheduled for execution. Set to NULL to disable automatic
 * scheduling.
 *
 * @param queue Pointer to the queue
 * @param task Pointer to the producer task, or NULL
 */
void pr_queue_producer_set(struct pr_queue *queue,
                           struct pr_task *task);

/**
 * @brief Push a pointer onto the queue
 *
 * Adds a pointer to the queue. If the consumer task is set, it will be
 * scheduled after the push completes. The queue does not take ownership
 * of the pointed-to data; the caller must manage memory lifecycle.
 *
 * @param queue Pointer to the queue
 * @param entry Pointer to push onto the queue
 * @return PR_OK on success, PR_ERR_BUSY if queue is full
 */
pr_error_t pr_queue_push(struct pr_queue *queue,
                         void *entry);

/**
 * @brief Pop a pointer from the queue
 *
 * Removes and returns a pointer from the queue. If the producer task
 * is set, it will be scheduled after the pop completes.
 *
 * @param queue Pointer to the queue
 * @param entry Pointer to receive the popped value
 * @return PR_OK on success, PR_ERR_BUSY if queue is empty
 */
pr_error_t pr_queue_pop(struct pr_queue *queue,
                        void **entry);

/** @} */ // end of pr_queue group

#endif
