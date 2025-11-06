/**
 * @file button.h
 * @defgroup pr_button Button Debouncing
 * @brief Software debouncing for button/switch inputs with task notification
 *
 * The button library provides debounced button/switch input handling using
 * GPIO interrupts and delayed tasks. It filters out switch bounce and provides
 * reliable button state change notifications.
 *
 * Key features:
 * - Automatic debouncing using task delays
 * - Support for active-high and active-low buttons
 * - Task notification on stable state changes
 * - GPIO IRQ integration for efficient operation
 *
 * Typical usage pattern:
 * @code
 * #include <pr/button.h>
 * #include <pr/gpio_irq.h>
 * #include <pr/task.h>
 *
 * struct pr_button button;
 * struct pr_task button_event;
 * struct pr_task_queue queue;
 *
 * void on_button_change(struct pr_task *task) {
 *     // Button state changed (debounced)
 *     printf("Button pressed/released\n");
 * }
 *
 * int main(void) {
 *     pr_task_queue_init(&queue);
 *     pr_task_init(&button_event, &queue, on_button_change);
 *
 *     // Set up GPIO IRQ system
 *     pr_gpio_irq_hookup();
 *
 *     // Initialize button on GPIO 15, active low (button to ground)
 *     pr_button_init(&button, &queue, &button_event, 15, false);
 *
 *     // button_event task will be called on each debounced state change
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_BUTTON_H
#define PR_BUTTON_H

#include <pico/stdlib.h>
#include <pr/task.h>

/**
 * @brief Button structure
 *
 * Manages button state and debouncing for a single GPIO pin.
 */
struct pr_button
{
    struct pr_task worker;     /**< Internal debounce worker task */
    struct pr_task *wake;      /**< Task to notify on state change */
    uint8_t io;                /**< GPIO pin number */
    bool active_high;          /**< true if button is active-high, false if active-low */
    bool last_value;           /**< Last stable button state */
    uint32_t mask;             /**< IRQ event mask */
};

/**
 * @brief Helper macro for composing button structures
 *
 * Generates functions to retrieve the parent pr_button structure from
 * the embedded worker task.
 */
PR_TASK_STRUCT_COMPOSE(pr_button, worker);

/**
 * @brief Initialize a button
 *
 * Configures a GPIO pin as a debounced button input. The pin is configured
 * with appropriate pull resistor and GPIO IRQ for efficient operation.
 * The wake task is scheduled whenever the button state changes after debouncing.
 *
 * @param button Pointer to button structure
 * @param queue Task queue for debounce worker
 * @param wake Task to schedule on button state change, or NULL for no notification
 * @param pin GPIO pin number for button input
 * @param active_high true if button is active-high (pull-down, button to VCC),
 *                    false if active-low (pull-up, button to ground)
 *
 * @note pr_gpio_irq_hookup() must be called before initializing buttons
 * @note The debounce time is fixed (typically 20-50ms)
 * @note The wake task's last_value field indicates the current stable state
 */
void pr_button_init(struct pr_button *button,
                    struct pr_task_queue *queue,
                    struct pr_task *wake,
                    uint pin, bool active_high);

/** @} */ // end of pr_button group

#endif
