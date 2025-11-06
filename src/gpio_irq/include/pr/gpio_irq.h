/**
 * @file gpio_irq.h
 * @defgroup pr_gpio_irq GPIO Interrupt Handling
 * @brief Task-based GPIO interrupt handling with per-pin task binding
 *
 * The gpio_irq library provides interrupt-driven GPIO event handling integrated
 * with the task system. GPIO pins can be bound to tasks that are automatically
 * scheduled when interrupts occur, enabling efficient event-driven applications.
 *
 * Key features:
 * - Per-pin task binding for interrupt events
 * - Support for edge and level-triggered interrupts
 * - Integration with Pico SDK GPIO IRQ system
 * - Task-based notification (no busy-waiting)
 *
 * Typical usage pattern:
 * @code
 * #include <pr/gpio_irq.h>
 * #include <pr/task.h>
 * #include <hardware/gpio.h>
 *
 * struct pr_task gpio_task;
 * struct pr_task_queue queue;
 *
 * void gpio_handler(struct pr_task *task) {
 *     // Handle GPIO event on pin 15
 *     printf("GPIO 15 interrupt!\n");
 * }
 *
 * int main(void) {
 *     pr_task_queue_init(&queue);
 *     pr_task_init(&gpio_task, &queue, gpio_handler);
 *
 *     // Set up GPIO IRQ system
 *     pr_gpio_irq_hookup();
 *
 *     // Configure pin 15 as input with pull-up
 *     gpio_init(15);
 *     gpio_set_dir(15, GPIO_IN);
 *     gpio_pull_up(15);
 *
 *     // Bind task to GPIO 15 and enable falling edge interrupt
 *     pr_gpio_irq_bind(15, &gpio_task);
 *     pr_gpio_irq_mask_set(15, GPIO_IRQ_EDGE_FALL);
 *
 *     // gpio_task will be scheduled when interrupt occurs
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_GPIO_IRQ_H
#define PR_GPIO_IRQ_H

#include <pico/stdlib.h>
#include <pr/task.h>
#include <hardware/gpio.h>

/**
 * @brief Initialize GPIO IRQ system
 *
 * Sets up the global GPIO interrupt handler that dispatches to per-pin tasks.
 * This must be called once before using pr_gpio_irq_bind() or
 * pr_gpio_irq_mask_set().
 *
 * @note This function installs a global GPIO IRQ handler
 * @note Call this only once during initialization
 */
void pr_gpio_irq_hookup();

/**
 * @brief Bind a task to a GPIO pin
 *
 * Associates a task with a GPIO pin. When an interrupt occurs on this pin,
 * the task will be automatically scheduled for execution.
 *
 * @param gpio GPIO pin number (0-29)
 * @param task Task to schedule on interrupt, or NULL to unbind
 *
 * @note pr_gpio_irq_hookup() must be called before binding tasks
 * @note Multiple pins can share the same task
 * @note Setting task to NULL removes the binding
 */
void pr_gpio_irq_bind(uint gpio,
                      struct pr_task *task);

/**
 * @brief Set GPIO interrupt event mask
 *
 * Configures which events trigger interrupts on a GPIO pin. Events are
 * combined with bitwise OR.
 *
 * @param gpio GPIO pin number (0-29)
 * @param event_mask Event mask (GPIO_IRQ_LEVEL_LOW, GPIO_IRQ_LEVEL_HIGH,
 *                   GPIO_IRQ_EDGE_FALL, GPIO_IRQ_EDGE_RISE, or combination)
 *
 * @note Use 0 to disable all interrupts on the pin
 * @note The task must be bound with pr_gpio_irq_bind() before enabling interrupts
 * @note Edge interrupts are automatically acknowledged in the IRQ handler
 *
 * Example:
 * @code
 * // Enable falling and rising edge interrupts
 * pr_gpio_irq_mask_set(15, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE);
 * @endcode
 */
void pr_gpio_irq_mask_set(uint gpio,
                          uint32_t event_mask);

/** @} */ // end of pr_gpio_irq group

#endif
