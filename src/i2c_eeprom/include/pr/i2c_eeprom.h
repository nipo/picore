#ifndef PR_I2C_EEPROM_H_
#define PR_I2C_EEPROM_H_

#include <stdint.h>
#include <pr/error.h>

struct pr_i2c_bus;
struct pr_i2c_eeprom;

struct pr_i2c_eeprom_vtable
{
    pr_error_t (*write)(struct pr_i2c_eeprom *i2c_eeprom,
                        uint16_t addr,
                        const uint8_t *data,
                        size_t size);

    pr_error_t (*read)(struct pr_i2c_eeprom *i2c_eeprom,
                       uint16_t addr,
                       uint8_t *data,
                       size_t size);
};

struct pr_i2c_eeprom
{
    struct pr_i2c_bus *bus;
    uint8_t saddr;
    const struct pr_i2c_eeprom_vtable *vtable;
};

enum pr_i2c_eeprom_type
{
    PR_I2C_EEPROM_24LC08,
};

pr_error_t pr_i2c_eeprom_init(struct pr_i2c_eeprom *i2c_eeprom,
                          struct pr_i2c_bus *bus,
                          uint8_t saddr,
                          enum pr_i2c_eeprom_type type);

static inline
pr_error_t pr_i2c_eeprom_write(struct pr_i2c_eeprom *i2c_eeprom,
                           uint16_t addr,
                           const uint8_t *data,
                           size_t size)
{
    return i2c_eeprom->vtable->write(i2c_eeprom, addr, data, size);
}

static inline
pr_error_t pr_i2c_eeprom_read(struct pr_i2c_eeprom *i2c_eeprom,
                          uint16_t addr,
                          uint8_t *data,
                          size_t size)
{
    return i2c_eeprom->vtable->read(i2c_eeprom, addr, data, size);
}

#endif
