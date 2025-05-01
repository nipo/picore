#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <pico.h>
#include <hardware/timer.h>
#include <pr/i2c_bus.h>
#include <pr/i2c_eeprom.h>

//#define dprintf printf
#define dprintf(...) do{}while(0)

#define EEPROM_24LC08_SIZE (256 * 4)
#define EEPROM_24LC08_ADDR_SIZE 1
#define EEPROM_24LC08_PAGE_SIZE 16

static
pr_error_t eeprom_24lc08_read(
    struct pr_i2c_eeprom *eeprom,
    uint16_t addr,
    uint8_t *data,
    size_t size)
{
    pr_error_t err;

    if (addr > EEPROM_24LC08_SIZE || addr + size > EEPROM_24LC08_SIZE) {
        dprintf("24LC08 read address/size overflow %04x %04x\n", addr, size);
        return PR_ERR_INVAL;
    }

    uint8_t saddr = eeprom->saddr | ((addr >> 8) & 3);
    uint8_t ptr[EEPROM_24LC08_ADDR_SIZE] = {addr & 0xff,};

    err = pr_i2c_bus_write_read(eeprom->bus, saddr, ptr, sizeof(ptr), data, size);

    dprintf("24LC08 read saddr %02x 0x%02x size %d -> %d\n", saddr, ptr[0], size, err);
    
    return err;
}

static
pr_error_t eeprom_24lc08_write(
    struct pr_i2c_eeprom *eeprom,
    uint16_t addr,
    const uint8_t *data,
    size_t size)
{
    bool first = true;
    pr_error_t err;
    uint16_t retry_count = 1;
    uint8_t saddr;
    uint8_t buf[EEPROM_24LC08_ADDR_SIZE + EEPROM_24LC08_PAGE_SIZE];

    if (addr > EEPROM_24LC08_SIZE || addr + size > EEPROM_24LC08_SIZE) {
        dprintf("24LC08 write address/size overflow %04x %04x\n", addr, size);
        return PR_ERR_INVAL;
    }

    uint16_t offset = 0;

    while (offset < size) {
        uint16_t chuck_size = 16;
        if (addr % EEPROM_24LC08_PAGE_SIZE)
            chuck_size = EEPROM_24LC08_PAGE_SIZE - (addr % EEPROM_24LC08_PAGE_SIZE);
        if (chuck_size > (size - offset))
            chuck_size = size - offset;

        saddr = eeprom->saddr | (((addr + offset) >> 8) & 3);
        buf[0] = (addr + offset) & 0xff;
        memcpy(buf + EEPROM_24LC08_ADDR_SIZE, data + offset, chuck_size);
        
        for (;;) {
            retry_count--;
            err = pr_i2c_bus_write(eeprom->bus, saddr, buf, chuck_size + EEPROM_24LC08_ADDR_SIZE);
            if (!err || !retry_count)
                break;

            busy_wait_ms(1);
        }

        dprintf("EEPROM write saddr %02x 0x%02x size %d -> %d\n", saddr, buf[0], chuck_size, err);

        if (err)
            return err;

        offset += chuck_size;
        retry_count = 128;
    }

    do {
        retry_count--;
        err = pr_i2c_bus_write_read(eeprom->bus, saddr, buf, EEPROM_24LC08_ADDR_SIZE, buf+1, 1);
        if (retry_count && err)
            busy_wait_ms(1);
    } while (err && retry_count);

    return PR_OK;
}

#define EEPROM_24AA64_SIZE (1 << 13)
#define EEPROM_24AA64_ADDR_SIZE 2
#define EEPROM_24AA64_PAGE_SIZE 32

static
pr_error_t eeprom_24aa64_read(
    struct pr_i2c_eeprom *eeprom,
    uint16_t addr,
    uint8_t *data,
    size_t size)
{
    pr_error_t err;

    if (addr > EEPROM_24AA64_SIZE || addr + size > EEPROM_24AA64_SIZE) {
        dprintf("24AA64 read address/size overflow %04x %04x\n", addr, size);
        return PR_ERR_INVAL;
    }

    uint8_t saddr = eeprom->saddr;
    uint8_t ptr[EEPROM_24AA64_ADDR_SIZE] = {(addr >> 8) & 0x1f, addr & 0xff};

    err = pr_i2c_bus_write_read(eeprom->bus, saddr, ptr, sizeof(ptr), data, size);

    dprintf("24AA64 read saddr %02x 0x%04x size %d -> %d\n", saddr, addr, size, err);
    
    return err;
}

static
pr_error_t eeprom_24aa64_write(
    struct pr_i2c_eeprom *eeprom,
    uint16_t addr,
    const uint8_t *data,
    size_t size)
{
    bool first = true;
    pr_error_t err;
    uint16_t retry_count = 1;
    uint8_t saddr;
    uint8_t buf[EEPROM_24AA64_ADDR_SIZE + EEPROM_24AA64_PAGE_SIZE];

    if (addr > EEPROM_24AA64_SIZE || addr + size > EEPROM_24AA64_SIZE) {
        dprintf("24AA64 write address/size overflow %04x %04x\n", addr, size);
        return PR_ERR_INVAL;
    }

    uint16_t offset = 0;

    while (offset < size) {
        uint16_t chuck_size = 16;
        if (addr % EEPROM_24AA64_PAGE_SIZE)
            chuck_size = EEPROM_24AA64_PAGE_SIZE - (addr % EEPROM_24AA64_PAGE_SIZE);
        if (chuck_size > (size - offset))
            chuck_size = size - offset;

        saddr = eeprom->saddr;
        buf[0] = ((addr + offset) >> 8) & 0x1f;
        buf[1] = (addr + offset) & 0xff;
        memcpy(buf + EEPROM_24AA64_ADDR_SIZE, data + offset, chuck_size);
        
        for (;;) {
            retry_count--;
            err = pr_i2c_bus_write(eeprom->bus, saddr, buf, chuck_size + EEPROM_24AA64_ADDR_SIZE);
            if (!err || !retry_count)
                break;

            busy_wait_ms(1);
        }

        dprintf("EEPROM write saddr %02x 0x%04x size %d -> %d\n", saddr, addr + offset, chuck_size, err);

        if (err)
            return err;

        offset += chuck_size;
        retry_count = 128;
    }

    for (;;) {
        retry_count--;
        err = pr_i2c_bus_write_read(eeprom->bus, saddr, buf, EEPROM_24AA64_ADDR_SIZE, buf+1, 1);
        if (!retry_count || !err)
            break;

        busy_wait_ms(1);
    }

    return err;
}

static struct pr_i2c_eeprom_vtable vtables[] = {
    [PR_I2C_EEPROM_24LC08] = {
        .write = eeprom_24lc08_write,
        .read = eeprom_24lc08_read,
    },
    [PR_I2C_EEPROM_24AA64] = {
        .write = eeprom_24aa64_write,
        .read = eeprom_24aa64_read,
    },
};

pr_error_t pr_i2c_eeprom_init(struct pr_i2c_eeprom *eeprom,
                          struct pr_i2c_bus *bus,
                          uint8_t saddr,
                          enum pr_i2c_eeprom_type type)
{
    if (type >= count_of(vtables))
        return PR_ERR_INVAL;

    eeprom->bus = bus;
    eeprom->saddr = saddr;
    eeprom->vtable = &vtables[type];

    return PR_OK;
}
