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

static
pr_error_t pca9575_w8(struct pr_i2c_gpio *i2c_gpio, uint8_t reg, uint8_t value)
{
    uint8_t data[] = { reg, value };
  
    return pr_i2c_bus_write(i2c_gpio->bus, i2c_gpio->saddr,
                            data, sizeof(data));
}

static
pr_error_t pca9575_w16(struct pr_i2c_gpio *i2c_gpio, uint8_t reg, uint16_t value)
{
    uint8_t data[] = { 0x80 | reg, value & 0xff, (value >> 8) & 0xff };
  
    return pr_i2c_bus_write(i2c_gpio->bus, i2c_gpio->saddr,
                            data, sizeof(data));
}

static
pr_error_t pca9575_set(struct pr_i2c_gpio *i2c_gpio,
                       uint32_t mask, uint32_t value)
{
    uint16_t old_set = i2c_gpio->set_cache;
    uint16_t set = (old_set & ~mask) | (value & mask);
    uint16_t changed = old_set ^ set;
    i2c_gpio->set_cache = set;

    dprintf("St %04x + %04x/%04x -> %04x\n", old_set, value, mask, set);

    if (!changed)
        return 0;
    
    if ((changed & 0xff00) == 0) {
        return pca9575_w8(i2c_gpio, 0x0a, set);
    } else if ((changed & 0xff00) == 0) {
        return pca9575_w8(i2c_gpio, 0x0b, set >> 8);
    } else {
        return pca9575_w16(i2c_gpio, 0x0a, set);
    }
}

static
pr_error_t pca9575_oe(struct pr_i2c_gpio *i2c_gpio,
                      uint32_t mask, uint32_t value)
{
    uint16_t old_oe = i2c_gpio->oe_cache;
    uint16_t oe = (old_oe & ~mask) | (value & mask);
    uint16_t changed = old_oe ^ oe;
    i2c_gpio->oe_cache = oe;

    dprintf("OE %04x + %04x/%04x -> %04x\n", old_oe, value, mask, oe);

    if (!changed)
        return 0;

    if ((changed & 0xff00) == 0) {
        return pca9575_w8(i2c_gpio, 0x08, ~oe);
    } else if ((changed & 0xff00) == 0) {
        return pca9575_w8(i2c_gpio, 0x09, ~oe >> 8);
    } else {
        return pca9575_w16(i2c_gpio, 0x08, ~oe);
    }
}

static
pr_error_t pca9575_get(struct pr_i2c_gpio *i2c_gpio,
                       uint32_t *value)
{
    const uint8_t cmd[] = { 0x80 };
    uint8_t rsp[2] = {};

    pr_error_t err = pr_i2c_bus_write_read(
        i2c_gpio->bus, i2c_gpio->saddr,
        cmd, sizeof(cmd),
        rsp, sizeof(rsp));

    *value = rsp[0] | ((uint32_t)rsp[1] << 8);

    return err;
}

static
pr_error_t pca9575_pull_en(struct pr_i2c_gpio *i2c_gpio,
                           uint32_t mask, uint32_t enable, uint32_t bias)
{
    uint16_t old_pull = i2c_gpio->pull_bias_cache;
    uint16_t pull = (old_pull & ~mask) | (bias & mask);
    i2c_gpio->pull_bias_cache = pull;

    uint16_t old_pull_en = i2c_gpio->pull_en_cache;
    uint16_t pull_en = (old_pull_en & ~mask) | (enable & mask);
    i2c_gpio->pull_en_cache = pull_en;

    const uint8_t cmd[] = { 0x84, !!(pull_en & 0xff), !!(pull_en & 0xff00), pull & 0xff, (pull >> 8) & 0xff };

    pr_error_t err = pr_i2c_bus_write(
        i2c_gpio->bus, i2c_gpio->saddr,
        cmd, sizeof(cmd));

    return err;
}

const struct pr_i2c_gpio_vtable pr_i2c_gpio_pca9575 = {
    .get = pca9575_get,
    .oe = pca9575_oe,
    .set = pca9575_set,
    .pull_en = pca9575_pull_en,
};

pr_error_t pr_i2c_gpio_init(struct pr_i2c_gpio *i2c_gpio,
                            const struct pr_i2c_gpio_vtable *vtable,
                            struct pr_i2c_bus *bus,
                            uint8_t saddr)
{
    i2c_gpio->bus = bus;
    i2c_gpio->saddr = saddr;
    i2c_gpio->vtable = vtable;

    i2c_gpio->set_cache = 0;
    i2c_gpio->oe_cache = 0;
    i2c_gpio->pull_bias_cache = 0;
    i2c_gpio->pull_en_cache = 0;
    
    return PR_OK;
}
