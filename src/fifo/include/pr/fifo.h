/**
 * @file fifo.h
 * @defgroup pr_fifo FIFO Ring Buffer
 * @brief Task-integrated ring buffer for byte stream communication
 *
 * The FIFO library provides circular ring buffers with automatic task
 * notification. When data becomes available or space becomes free, associated
 * consumer or producer tasks can be automatically scheduled, enabling
 * efficient producer-consumer patterns.
 *
 * Key features:
 * - Ring buffer for byte streams with minimal copying
 * - Automatic task scheduling on data/space availability
 * - Zero-copy read/write operations via prepare/done API
 * - Thread-safe operations
 * - Suitable for UART, USB, and other streaming I/O
 *
 * Typical usage pattern:
 * @code
 * struct pr_fifo fifo;
 * struct pr_task consumer_task;
 *
 * void consumer_handler(struct pr_task *task) {
 *     uint8_t buffer[64];
 *     size_t n = pr_fifo_read(&fifo, buffer, sizeof(buffer));
 *     // Process n bytes from buffer...
 * }
 *
 * int main(void) {
 *     pr_fifo_init(&fifo, 1024);
 *     pr_task_init(&consumer_task, &queue, consumer_handler);
 *     pr_fifo_consumer_set(&fifo, &consumer_task);
 *
 *     // When data is written to FIFO, consumer_task is automatically scheduled
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_FIFO_H_
#define PR_FIFO_H_

#include <stdint.h>
#include <stdlib.h>
#include <hardware/sync.h>
#include <pr/task.h>

/**
 * @brief FIFO ring buffer structure
 *
 * Circular buffer for byte streams with read and write pointers.
 * Can automatically schedule consumer and producer tasks when data
 * or space becomes available.
 */
struct pr_fifo
{
    struct pr_task *consumer;   /**< Task to schedule when data becomes available */
    struct pr_task *producer;   /**< Task to schedule when space becomes available */
    uint8_t *buffer;            /**< Internal buffer storage */
    uint32_t size;              /**< Total buffer size in bytes */
    uint32_t wptr;              /**< Write pointer position */
    uint32_t rptr;              /**< Read pointer position */
    uint32_t free;              /**< Number of free bytes */
    uint32_t used;              /**< Number of used bytes */
};

/**
 * @brief Initialize a FIFO buffer
 *
 * Allocates and initializes a ring buffer of the specified size.
 * The buffer is allocated using malloc() and should be freed when
 * no longer needed (though no explicit free function is provided).
 *
 * @param fifo Pointer to the FIFO structure to initialize
 * @param size Size of the buffer in bytes (power of 2 recommended for efficiency)
 */
void pr_fifo_init(struct pr_fifo *fifo,
                  size_t size);

/**
 * @brief Clear all data from the FIFO
 *
 * Resets the FIFO to empty state, discarding all buffered data.
 * Does not deallocate the buffer.
 *
 * @param fifo Pointer to the FIFO
 */
void pr_fifo_clear(struct pr_fifo *fifo);

/**
 * @brief Check if FIFO is empty
 *
 * @param fifo Pointer to the FIFO
 * @return true if FIFO contains no data, false otherwise
 */
bool pr_fifo_is_empty(struct pr_fifo *fifo);

/**
 * @brief Check if FIFO is full
 *
 * @param fifo Pointer to the FIFO
 * @return true if FIFO has no free space, false otherwise
 */
bool pr_fifo_is_full(struct pr_fifo *fifo);

/**
 * @brief Get number of bytes available to read
 *
 * @param fifo Pointer to the FIFO
 * @return Number of bytes that can be read from the FIFO
 */
size_t pr_fifo_available(struct pr_fifo *fifo);

/**
 * @brief Get number of bytes available to write
 *
 * @param fifo Pointer to the FIFO
 * @return Number of bytes that can be written to the FIFO
 */
size_t pr_fifo_free(struct pr_fifo *fifo);

/**
 * @brief Set consumer task for automatic scheduling
 *
 * When data is written to the FIFO, the consumer task will be automatically
 * scheduled for execution. Set to NULL to disable automatic scheduling.
 *
 * @param fifo Pointer to the FIFO
 * @param task Pointer to the consumer task, or NULL
 */
void pr_fifo_consumer_set(struct pr_fifo *fifo,
                          struct pr_task *task);

/**
 * @brief Set producer task for automatic scheduling
 *
 * When data is read from the FIFO (freeing space), the producer task will
 * be automatically scheduled for execution. Set to NULL to disable automatic
 * scheduling.
 *
 * @param fifo Pointer to the FIFO
 * @param task Pointer to the producer task, or NULL
 */
void pr_fifo_producer_set(struct pr_fifo *fifo,
                          struct pr_task *task);

/**
 * @brief Write data to the FIFO
 *
 * Copies data into the FIFO buffer. If the consumer task is set, it will
 * be scheduled after the write completes.
 *
 * @param fifo Pointer to the FIFO
 * @param data Pointer to data to write
 * @param size Number of bytes to write
 * @return Number of bytes actually written (may be less than requested if FIFO is full)
 */
size_t pr_fifo_write(struct pr_fifo *fifo,
                     const uint8_t *data,
                     size_t size);

/**
 * @brief Read data from the FIFO
 *
 * Copies data from the FIFO buffer and removes it. If the producer task
 * is set, it will be scheduled after the read completes.
 *
 * @param fifo Pointer to the FIFO
 * @param data Pointer to buffer to receive data
 * @param size Maximum number of bytes to read
 * @return Number of bytes actually read (may be less than requested if FIFO has less data)
 */
size_t pr_fifo_read(struct pr_fifo *fifo,
                    uint8_t *data,
                    size_t size);

/**
 * @brief Peek at data in the FIFO without removing it
 *
 * Copies data from the FIFO buffer without consuming it. The data remains
 * in the FIFO and can be read again. Does not schedule any tasks.
 *
 * @param fifo Pointer to the FIFO
 * @param data Pointer to buffer to receive data
 * @param size Maximum number of bytes to peek
 * @return Number of bytes actually copied (may be less than requested if FIFO has less data)
 */
size_t pr_fifo_peek(struct pr_fifo *fifo,
                    uint8_t *data,
                    size_t size);

/**
 * @brief Prepare for zero-copy read operation
 *
 * Returns a pointer directly into the FIFO's internal buffer for reading
 * data without copying. Must be followed by pr_fifo_read_done() to consume
 * the data. This enables efficient zero-copy operations.
 *
 * @param fifo Pointer to the FIFO
 * @param size Maximum number of bytes requested
 * @param data Output pointer to the data in the FIFO buffer
 * @return Number of bytes available at the returned pointer (may be less than
 *         requested due to ring wrap-around or available data)
 *
 * @note Due to ring buffer wrap-around, the returned size may be less than
 *       available data. Call again after pr_fifo_read_done() to get remaining data.
 *
 * @see pr_fifo_read_done()
 */
size_t pr_fifo_read_prepare(struct pr_fifo *fifo,
                            size_t size,
                            const uint8_t **data);

/**
 * @brief Complete a zero-copy read operation
 *
 * Consumes the specified number of bytes from the FIFO after reading them
 * via pr_fifo_read_prepare(). If the producer task is set, it will be
 * scheduled.
 *
 * @param fifo Pointer to the FIFO
 * @param size Number of bytes to consume (should be <= size returned by pr_fifo_read_prepare())
 *
 * @see pr_fifo_read_prepare()
 */
void pr_fifo_read_done(struct pr_fifo *fifo,
                       size_t size);

/**
 * @brief Prepare for zero-copy write operation
 *
 * Returns a pointer directly into the FIFO's internal buffer for writing
 * data without copying. Must be followed by pr_fifo_write_done() to commit
 * the data. This enables efficient zero-copy operations.
 *
 * @param fifo Pointer to the FIFO
 * @param size Maximum number of bytes requested
 * @param data Output pointer to writable space in the FIFO buffer
 * @return Number of bytes available at the returned pointer (may be less than
 *         requested due to ring wrap-around or available space)
 *
 * @note Due to ring buffer wrap-around, the returned size may be less than
 *       available space. Call again after pr_fifo_write_done() to get remaining space.
 *
 * @see pr_fifo_write_done()
 */
size_t pr_fifo_write_prepare(struct pr_fifo *fifo,
                             size_t size,
                             uint8_t **data);

/**
 * @brief Complete a zero-copy write operation
 *
 * Commits the specified number of bytes to the FIFO after writing them
 * via pr_fifo_write_prepare(). If the consumer task is set, it will be
 * scheduled.
 *
 * @param fifo Pointer to the FIFO
 * @param size Number of bytes to commit (should be <= size returned by pr_fifo_write_prepare())
 *
 * @see pr_fifo_write_prepare()
 */
void pr_fifo_write_done(struct pr_fifo *fifo,
                        size_t size);

/** @} */ // end of pr_fifo group

#endif
