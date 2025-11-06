/**
 * @file i2s_master_out.h
 * @defgroup pr_i2s_master_out I2S Master Output
 * @brief I2S digital audio output using PIO
 *
 * The i2s_master_out library provides I2S (Inter-IC Sound) digital audio
 * output using the RP2040's PIO system. It can drive I2S DACs and other
 * digital audio devices with task-based buffer management.
 *
 * Key features:
 * - PIO-based I2S protocol generation
 * - DMA for efficient data transfer
 * - Task notification on buffer completion
 * - Support for normal and inverted signals
 * - Configurable sample rate and bit depth
 *
 * Typical usage pattern:
 * @code
 * #include <pr/i2s_master_out.h>
 * #include <pr/task.h>
 *
 * struct pr_i2s_master_out_strip i2s;
 * struct pr_task_queue queue;
 *
 * int main(void) {
 *     pr_task_queue_init(&queue);
 *
 *     // Initialize I2S on GPIO 16 (data), using PIO 0 SM 0
 *     // 1024 samples buffer size
 *     pr_i2s_master_out_strip_init(&i2s, &queue, 0, 0, 1024, 16, false);
 *
 *     // Fill buffer with audio samples
 *     for (size_t i = 0; i < 1024; i++) {
 *         pr_i2s_master_out_set_norefresh(&i2s, i, audio_sample[i]);
 *     }
 *
 *     // Trigger playback
 *     pr_i2s_master_out_refresh(&i2s);
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_I2S_MASTER_OUT_H
#define PR_I2S_MASTER_OUT_H

#include <pico/stdlib.h>
#include <pr/task.h>
#include <hardware/pio.h>

/**
 * @brief I2S master output structure
 *
 * Manages I2S audio output with PIO and task integration.
 * All fields are internal.
 */
struct pr_i2s_master_out_strip
{
    PIO pio;                        /**< PIO instance (pio0 or pio1) */
    uint8_t sm;                     /**< State machine index (0-3) */
    struct pr_task_queue *task_queue; /**< Task queue for notifications */
    struct pr_task refresh_task;    /**< Task for initiating transfer */
    struct pr_task done_task;       /**< Task for transfer completion */
    uint32_t *pixel_data;           /**< Audio sample data buffer */
    size_t pixel_count;             /**< Number of samples in buffer */
    size_t offset;                  /**< Current DMA transfer offset */
    bool refreshing;                /**< Transfer in progress flag */
    bool dirty;                     /**< Data modified since last transfer */
    bool inv;                       /**< Signal inversion flag */
};

/**
 * @brief Helper macros for composing I2S structures
 *
 * Generate functions to retrieve the parent pr_i2s_master_out_strip structure
 * from embedded task members.
 */
PR_TASK_STRUCT_COMPOSE(pr_i2s_master_out_strip, refresh_task);
PR_TASK_STRUCT_COMPOSE(pr_i2s_master_out_strip, done_task);

/**
 * @brief Initialize I2S master output
 *
 * Configures PIO and DMA for I2S audio output. The interface is ready
 * to transmit audio data after initialization.
 *
 * @param strip Pointer to I2S master output structure
 * @param queue Task queue for buffer management
 * @param pio_index PIO instance index (0 or 1)
 * @param sm_index State machine index within PIO (0-3)
 * @param led_count Number of audio samples in buffer
 * @param pin_data GPIO pin for I2S data output (SD)
 * @param inv true to invert signal, false for normal
 *
 * @note The PIO state machine and DMA channel are claimed automatically
 * @note Additional I2S signals (LRCLK, BCLK) use adjacent GPIO pins
 * @note Sample format depends on PIO program configuration
 */
void pr_i2s_master_out_strip_init(struct pr_i2s_master_out_strip *strip,
                             struct pr_task_queue *queue,
                             uint8_t pio_index, uint8_t sm_index,
                             size_t led_count, uint pin_data, bool inv);

/**
 * @brief Trigger I2S transfer
 *
 * Schedules an I2S transfer if data has been modified. If a transfer is
 * already in progress, it will be deferred until completion.
 *
 * @param strip Pointer to I2S master output structure
 *
 * @note Transfer happens asynchronously via DMA
 * @note Multiple calls while transferring are coalesced
 */
void pr_i2s_master_out_refresh(struct pr_i2s_master_out_strip *strip);

/**
 * @brief Set audio sample without triggering transfer
 *
 * Sets a single audio sample and marks the buffer as dirty, but does not
 * immediately trigger transfer. Use for batch updates.
 *
 * @param strip Pointer to I2S master output structure
 * @param offset Sample index (0 to sample_count-1)
 * @param value Audio sample value (format depends on configuration)
 *
 * @note offset must be less than sample_count
 */
static inline
void pr_i2s_master_out_set_norefresh(struct pr_i2s_master_out_strip *strip,
                                size_t offset, uint32_t value)
{
    assert(offset < strip->pixel_count);
    strip->dirty = true;
    strip->pixel_data[offset] = value;
}

/**
 * @brief Set audio sample and trigger transfer
 *
 * Sets a single audio sample and schedules a transfer.
 *
 * @param strip Pointer to I2S master output structure
 * @param offset Sample index (0 to sample_count-1)
 * @param value Audio sample value (format depends on configuration)
 *
 * @note offset must be less than sample_count
 */
static inline
void pr_i2s_master_out_set(struct pr_i2s_master_out_strip *strip,
                      size_t offset, uint32_t value)
{
    pr_i2s_master_out_set_norefresh(strip, offset, value);
    pr_i2s_master_out_refresh(strip);
}

/**
 * @brief Set multiple audio samples and trigger transfer
 *
 * Sets multiple consecutive audio samples and schedules a transfer.
 * More efficient than multiple individual calls.
 *
 * @param strip Pointer to I2S master output structure
 * @param offset Starting sample index
 * @param value Pointer to array of sample values
 * @param count Number of samples to update
 *
 * @note offset + count must be <= sample_count
 */
static inline
void pr_i2s_master_out_set_many(struct pr_i2s_master_out_strip *strip,
                           size_t offset, const uint32_t *value, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        pr_i2s_master_out_set_norefresh(strip, offset+i, value[i]);
    pr_i2s_master_out_refresh(strip);
}

/** @} */ // end of pr_i2s_master_out group

#endif
