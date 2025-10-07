#ifndef PR_I2C_GPIO_H_
#define PR_I2C_GPIO_H_

#include <stdint.h>
#include <pr/error.h>

struct pr_i2c_bus;
struct pr_i2c_gpio;

struct pr_i2c_gpio_vtable
{
    pr_error_t (*set)(struct pr_i2c_gpio *i2c_gpio,
                      uint32_t mask, uint32_t value);

    pr_error_t (*oe)(struct pr_i2c_gpio *i2c_gpio,
                     uint32_t mask, uint32_t value);

    pr_error_t (*get)(struct pr_i2c_gpio *i2c_gpio,
                      uint32_t *value);

    pr_error_t (*pull_en)(struct pr_i2c_gpio *i2c_gpio,
                          uint32_t mask, uint32_t value);
};

struct pr_i2c_gpio
{
    struct pr_i2c_bus *bus;
    uint8_t saddr;
    const struct pr_i2c_gpio_vtable *vtable;
    uint32_t set_cache;
    uint32_t oe_cache;
    uint32_t pull_cache;
    uint32_t pull_en_cache;
};

extern const struct pr_i2c_gpio_vtable pr_i2c_gpio_tca9534;

pr_error_t pr_i2c_gpio_init(struct pr_i2c_gpio *i2c_gpio,
                            const struct pr_i2c_gpio_vtable *vtable,
                            struct pr_i2c_bus *bus,
                            uint8_t saddr);

static inline
pr_error_t pr_i2c_gpio_set(struct pr_i2c_gpio *i2c_gpio,
                           uint32_t mask, uint32_t value)
{
    if (!i2c_gpio->vtable->set)
        return PR_ERR_IMPL;
    return i2c_gpio->vtable->set(i2c_gpio, mask, value);
}

static inline
pr_error_t pr_i2c_gpio_oe(struct pr_i2c_gpio *i2c_gpio,
                           uint32_t mask, uint32_t value)
{
    if (!i2c_gpio->vtable->oe)
        return PR_ERR_IMPL;
    return i2c_gpio->vtable->oe(i2c_gpio, mask, value);
}

static inline
pr_error_t pr_i2c_gpio_pull_en(struct pr_i2c_gpio *i2c_gpio,
                           uint32_t mask, uint32_t value)
{
    if (!i2c_gpio->vtable->pull_en)
        return PR_ERR_IMPL;
    return i2c_gpio->vtable->pull_en(i2c_gpio, mask, value);
}

static inline
pr_error_t pr_i2c_gpio_get(struct pr_i2c_gpio *i2c_gpio,
                           uint32_t *value)
{
    if (!i2c_gpio->vtable->get)
        return PR_ERR_IMPL;
    return i2c_gpio->vtable->get(i2c_gpio, value);
}

#endif
