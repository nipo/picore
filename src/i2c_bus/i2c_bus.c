#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pr/error.h>
#include <pr/i2c_bus.h>

static
i2c_inst_t *instance_for_pin(uint8_t pin)
{
    int index = (pin & 2) >> 1;

    return index ? i2c1 : i2c0;
}

pr_error_t pr_i2c_bus_init(struct pr_i2c_bus *bus,
                           uint8_t sda,
                           uint8_t scl,
                           uint32_t rate)
{
    i2c_inst_t *sda_inst = instance_for_pin(sda);
    i2c_inst_t *scl_inst = instance_for_pin(scl);

    if (sda_inst != scl_inst)
        return PR_ERR_INVAL;

    bus->instance = sda_inst;
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    i2c_init(bus->instance, rate);

    return PR_OK;
}

static
pr_error_t pico_to_pr_err(int err, int count)
{
    if (err == count)
        return PR_OK;

    if (err >= 0 && err < count)
        return PR_ERR_IO;

    switch (err) {
    case PICO_ERROR_GENERIC:
        return PR_ERR_ADDRESS; 
    case PICO_ERROR_TIMEOUT:
        return PR_ERR_TIMEOUT;
    }

    return PR_ERR_IO;
}

pr_error_t pr_i2c_bus_write(
    struct pr_i2c_bus *bus,
    uint8_t saddr,
    const uint8_t *data, size_t size)
{
    int err = i2c_write_blocking(bus->instance, saddr, data, size, false);

    return pico_to_pr_err(err, size);
}

pr_error_t pr_i2c_bus_read(
    struct pr_i2c_bus *bus,
    uint8_t saddr,
    uint8_t *data, size_t size)
{
    int err = i2c_read_blocking(bus->instance, saddr, data, size, false);

    return pico_to_pr_err(err, size);
}

pr_error_t pr_i2c_bus_write_read(
    struct pr_i2c_bus *bus,
    uint8_t saddr,
    const uint8_t *wdata, size_t wsize,
    uint8_t *rdata, size_t rsize)
{
    int err = i2c_write_blocking(bus->instance, saddr, wdata, wsize, true);
    pr_error_t be = pico_to_pr_err(err, wsize);
    if (be)
        return be;
    err = i2c_read_blocking(bus->instance, saddr, rdata, rsize, false);
    return pico_to_pr_err(err, rsize);
}
