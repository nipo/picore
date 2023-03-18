#ifndef PR_I2C_BUS_H
#define PR_I2C_BUS_H

#include <stddef.h>
#include <stdint.h>
#include <pr/error.h>

struct pr_i2c_bus {
    struct i2c_inst *instance;
};

pr_error_t pr_i2c_bus_init(struct pr_i2c_bus *bus,
                           uint8_t sda, uint8_t scl,
                           uint32_t rate);

pr_error_t pr_i2c_bus_write(
    struct pr_i2c_bus *bus,
    uint8_t saddr,
    const uint8_t *data,
    size_t size);

pr_error_t pr_i2c_bus_read(
    struct pr_i2c_bus *bus,
    uint8_t saddr, uint8_t *data,
    size_t size);

pr_error_t pr_i2c_bus_write_read(
    struct pr_i2c_bus *bus, 
    uint8_t saddr,
    const uint8_t *wdata, size_t wsize,
    uint8_t *rdata, size_t rsize);

#endif
