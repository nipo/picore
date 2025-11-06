/**
 * @file pixel_led.h
 * @defgroup pr_pixel_led Addressable LED Driver (WS2812/NeoPixel)
 * @brief WS2812/NeoPixel addressable LED strip driver using PIO
 *
 * The pixel_led library provides control of WS2812-style addressable RGB LED
 * strips (commonly known as NeoPixels) using the RP2040's PIO (Programmable I/O)
 * system. It supports task-based refresh with automatic DMA transfer.
 *
 * Key features:
 * - PIO-based timing (no CPU intervention during transfer)
 * - DMA for efficient data transfer
 * - Task notification on refresh complete
 * - Support for normal and inverted data signals
 * - Dirty tracking to avoid unnecessary refreshes
 *
 * Typical usage pattern:
 * @code
 * #include <pr/pixel_led.h>
 * #include <pr/task.h>
 *
 * struct pr_pixel_led_strip strip;
 * struct pr_task_queue queue;
 *
 * int main(void) {
 *     pr_task_queue_init(&queue);
 *
 *     // Initialize 30 LEDs on GPIO 16, using PIO 0 SM 0, normal signal
 *     pr_pixel_led_strip_init(&strip, &queue, 0, 0, 30, 16, false);
 *
 *     // Set first LED to red (GRB format: 0x00FF0000 = green=0, red=255, blue=0)
 *     pr_pixel_led_set(&strip, 0, 0x00FF0000);
 *
 *     // Set multiple LEDs
 *     uint32_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF}; // Red, Green, Blue
 *     pr_pixel_led_set_many(&strip, 1, colors, 3);
 *
 *     // Refresh is automatic
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_PIXEL_LED_H
#define PR_PIXEL_LED_H

#include <pico/stdlib.h>
#include <pr/task.h>
#include <hardware/pio.h>

/**
 * @brief Pixel LED strip structure
 *
 * Manages an addressable LED strip with PIO and task integration.
 * All fields are internal.
 */
struct pr_pixel_led_strip
{
    PIO pio;                        /**< PIO instance (pio0 or pio1) */
    uint8_t sm;                     /**< State machine index (0-3) */
    struct pr_task_queue *task_queue; /**< Task queue for notifications */
    struct pr_task refresh_task;    /**< Task for initiating refresh */
    struct pr_task done_task;       /**< Task for refresh completion */
    uint32_t *pixel_data;           /**< Pixel color data buffer */
    size_t pixel_count;             /**< Number of LEDs in strip */
    size_t offset;                  /**< Current DMA transfer offset */
    bool refreshing;                /**< Refresh in progress flag */
    bool dirty;                     /**< Data modified since last refresh */
    bool inv;                       /**< Signal inversion flag */
};

/**
 * @brief Helper macros for composing pixel LED structures
 *
 * Generate functions to retrieve the parent pr_pixel_led_strip structure
 * from embedded task members.
 */
PR_TASK_STRUCT_COMPOSE(pr_pixel_led_strip, refresh_task);
PR_TASK_STRUCT_COMPOSE(pr_pixel_led_strip, done_task);

/**
 * @brief Initialize a pixel LED strip
 *
 * Configures PIO and DMA for controlling an addressable LED strip. The strip
 * is ready for use immediately after initialization.
 *
 * @param strip Pointer to pixel LED strip structure
 * @param queue Task queue for refresh management
 * @param pio_index PIO instance index (0 or 1)
 * @param sm_index State machine index within PIO (0-3)
 * @param led_count Number of LEDs in the strip
 * @param pin_data GPIO pin for data output
 * @param inv true to invert signal (for level shifters), false for normal
 *
 * @note The PIO state machine and DMA channel are claimed automatically
 * @note Color format is GRB (green, red, blue), 8 bits each, stored in uint32_t
 */
void pr_pixel_led_strip_init(struct pr_pixel_led_strip *strip,
                             struct pr_task_queue *queue,
                             uint8_t pio_index, uint8_t sm_index,
                             size_t led_count, uint pin_data, bool inv);

/**
 * @brief Trigger strip refresh
 *
 * Schedules a refresh if data has been modified. If a refresh is already
 * in progress, it will be deferred until the current refresh completes.
 *
 * @param strip Pointer to pixel LED strip structure
 *
 * @note Refresh happens asynchronously via DMA
 * @note Multiple calls while refreshing are coalesced
 */
void pr_pixel_led_refresh(struct pr_pixel_led_strip *strip);

/**
 * @brief Set pixel color without triggering refresh
 *
 * Sets the color of a single pixel and marks the strip as dirty, but does
 * not immediately trigger a refresh. Use this for batch updates followed
 * by a single pr_pixel_led_refresh() call.
 *
 * @param strip Pointer to pixel LED strip structure
 * @param offset Pixel index (0 to led_count-1)
 * @param value Color in GRB format (0xGGRRBB00, 8 bits per channel)
 *
 * @note Color format is GRB, not RGB
 * @note offset must be less than led_count
 */
static inline
void pr_pixel_led_set_norefresh(struct pr_pixel_led_strip *strip,
                                size_t offset, uint32_t value)
{
    assert(offset < strip->pixel_count);
    strip->dirty = true;
    strip->pixel_data[offset] = value;
}

/**
 * @brief Set pixel color and trigger refresh
 *
 * Sets the color of a single pixel and schedules a refresh.
 *
 * @param strip Pointer to pixel LED strip structure
 * @param offset Pixel index (0 to led_count-1)
 * @param value Color in GRB format (0xGGRRBB00, 8 bits per channel)
 *
 * @note Color format is GRB, not RGB
 * @note offset must be less than led_count
 */
static inline
void pr_pixel_led_set(struct pr_pixel_led_strip *strip,
                      size_t offset, uint32_t value)
{
    pr_pixel_led_set_norefresh(strip, offset, value);
    pr_pixel_led_refresh(strip);
}

/**
 * @brief Set multiple pixel colors and trigger refresh
 *
 * Sets the colors of multiple consecutive pixels and schedules a refresh.
 * This is more efficient than multiple pr_pixel_led_set() calls.
 *
 * @param strip Pointer to pixel LED strip structure
 * @param offset Starting pixel index
 * @param value Pointer to array of colors in GRB format
 * @param count Number of pixels to update
 *
 * @note Color format is GRB, not RGB
 * @note offset + count must be <= led_count
 */
static inline
void pr_pixel_led_set_many(struct pr_pixel_led_strip *strip,
                           size_t offset, const uint32_t *value, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        pr_pixel_led_set_norefresh(strip, offset+i, value[i]);
    pr_pixel_led_refresh(strip);
}

/** @} */ // end of pr_pixel_led group

#endif
