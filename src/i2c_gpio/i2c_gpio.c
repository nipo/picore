#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <pico.h>
#include <hardware/timer.h>
#include <pr/i2c_bus.h>
#include <pr/i2c_gpio.h>

//#define dprintf printf
#define dprintf(...) do{}while(0)

static
pr_error_t tca9534_set(struct pr_i2c_gpio *i2c_gpio,
                       uint32_t mask, uint32_t value)
{
    uint8_t old_set = i2c_gpio->set_cache;
    uint8_t set = (old_set & ~mask) | (value & mask);
    uint8_t data[] = { 0x01, (uint8_t)set };
    i2c_gpio->set_cache = set;
    
    dprintf("St %02x + %02x/%02x -> %02x\n", old_set, value, mask, set);

    return pr_i2c_bus_write(i2c_gpio->bus, i2c_gpio->saddr,
                            data, sizeof(data));
}

static
pr_error_t tca9534_oe(struct pr_i2c_gpio *i2c_gpio,
                      uint32_t mask, uint32_t value)
{
    uint8_t old_oe = i2c_gpio->oe_cache;
    uint8_t oe = (old_oe & ~mask) | (value & mask);
    uint8_t data[] = { 0x03, (uint8_t)~oe };
    i2c_gpio->oe_cache = oe;
    
    dprintf("OE %02x + %02x/%02x -> %02x\n", old_oe, value, mask, oe);

    return pr_i2c_bus_write(i2c_gpio->bus, i2c_gpio->saddr,
                            data, sizeof(data));
}

static
pr_error_t tca9534_get(struct pr_i2c_gpio *i2c_gpio,
                       uint32_t *value)
{
    const uint8_t cmd[] = { 0x00 };
    uint8_t rsp[1] = {};

    pr_error_t err = pr_i2c_bus_write_read(
        i2c_gpio->bus, i2c_gpio->saddr,
        cmd, sizeof(cmd),
        rsp, sizeof(rsp));

    *value = rsp[0];

    return err;
}

const struct pr_i2c_gpio_vtable pr_i2c_gpio_tca9534 = {
    .get = tca9534_get,
    .oe = tca9534_oe,
    .set = tca9534_set,
};

pr_error_t pr_i2c_gpio_init(struct pr_i2c_gpio *i2c_gpio,
                            const struct pr_i2c_gpio_vtable *vtable,
                            struct pr_i2c_bus *bus,
                            uint8_t saddr)
{
    i2c_gpio->bus = bus;
    i2c_gpio->saddr = saddr;
    i2c_gpio->vtable = vtable;

    return PR_OK;
}
