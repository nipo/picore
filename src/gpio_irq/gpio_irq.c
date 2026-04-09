#include <hardware/gpio.h>
#include <pr/gpio_irq.h>

struct pr_gpio_irq
{
    struct pr_task *handler[30];
};

static struct pr_gpio_irq g_irq = {};

static
void gpio_irq_callback(uint gpio, uint32_t event_mask)
{
    struct pr_task *h = g_irq.handler[gpio];

    if (gpio >= 31)
        return;

    gpio_acknowledge_irq(gpio, event_mask);

    // For level-triggered interrupts, disable until the task re-enables.
    // Otherwise the IRQ fires continuously while the pin stays asserted.
    if (event_mask & (GPIO_IRQ_LEVEL_LOW | GPIO_IRQ_LEVEL_HIGH))
        gpio_set_irq_enabled(gpio, event_mask, false);

    if (!h)
        return;

    pr_task_exec(h);
}

void pr_gpio_irq_hookup()
{
    gpio_set_irq_callback(gpio_irq_callback);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

void pr_gpio_irq_bind(uint gpio,
                      struct pr_task *task)
{
    gpio_init(gpio);
    gpio_set_function(gpio, GPIO_FUNC_SIO);
    gpio_set_pulls(gpio, 0, 0);
    gpio_set_dir(gpio, 0);

    g_irq.handler[gpio] = task;
}

void pr_gpio_irq_mask_set(uint gpio,
                          uint32_t event_mask)
{
    gpio_set_irq_enabled(gpio, event_mask, !!event_mask);
}
