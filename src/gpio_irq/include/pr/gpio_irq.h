#ifndef PR_GPIO_IRQ_H
#define PR_GPIO_IRQ_H

#include <pico/stdlib.h>
#include <pr/task.h>
#include <hardware/gpio.h>

void pr_gpio_irq_hookup();
void pr_gpio_irq_bind(uint gpio,
                      struct pr_task *task);
void pr_gpio_irq_mask_set(uint gpio,
                          uint32_t event_mask);

#endif
