/**
 * @file i2c_bus.h
 * @defgroup pr_i2c_bus I2C Bus
 * @brief I2C bus abstraction for master mode communication
 *
 * The i2c_bus library provides a simplified interface to the Pico SDK I2C
 * peripheral for master mode operation. It handles GPIO configuration and
 * provides convenient functions for common I2C transactions.
 *
 * Key features:
 * - Automatic GPIO configuration for I2C pins
 * - Configurable bus speed
 * - Read, write, and combined write-read transactions
 * - Error handling with pr_error_t return codes
 * - Blocking operation (suitable for use in tasks)
 *
 * Typical usage pattern:
 * @code
 * #include <pr/i2c_bus.h>
 *
 * struct pr_i2c_bus i2c;
 *
 * int main(void) {
 *     // Initialize I2C on GPIO 4 (SDA) and 5 (SCL) at 400kHz
 *     pr_i2c_bus_init(&i2c, 4, 5, 400000);
 *
 *     // Write 2 bytes to device at address 0x50
 *     uint8_t data[] = {0x00, 0xFF};
 *     pr_i2c_bus_write(&i2c, 0x50, data, 2);
 *
 *     // Read 4 bytes from device at address 0x51
 *     uint8_t buffer[4];
 *     pr_i2c_bus_read(&i2c, 0x51, buffer, 4);
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_I2C_BUS_H
#define PR_I2C_BUS_H

#include <stddef.h>
#include <stdint.h>
#include <pr/error.h>

/**
 * @brief I2C bus structure
 *
 * Represents an I2C bus instance. All fields are internal.
 */
struct pr_i2c_bus {
    struct i2c_inst *instance; /**< Pico SDK I2C instance */
};

/**
 * @brief Initialize an I2C bus
 *
 * Configures the specified GPIO pins for I2C operation and sets the bus
 * speed. The I2C peripheral is enabled and ready for transactions.
 *
 * @param bus Pointer to I2C bus structure
 * @param sda GPIO pin number for SDA (data line)
 * @param scl GPIO pin number for SCL (clock line)
 * @param rate Bus speed in Hz (e.g., 100000 for 100kHz, 400000 for 400kHz)
 * @return PR_OK on success, error code on failure
 *
 * @note The actual rate may differ slightly from the requested rate
 * @note Pull-up resistors are required on SDA and SCL lines
 */
pr_error_t pr_i2c_bus_init(struct pr_i2c_bus *bus,
                           uint8_t sda, uint8_t scl,
                           uint32_t rate);

/**
 * @brief Write data to an I2C device
 *
 * Performs a complete I2C write transaction. The bus is acquired, data is
 * sent, and the bus is released. This is a blocking operation.
 *
 * @param bus Pointer to I2C bus structure
 * @param saddr 7-bit I2C device address (not shifted)
 * @param data Pointer to data to write
 * @param size Number of bytes to write
 * @return PR_OK on success, error code on failure (e.g., NACK, timeout)
 *
 * @note This function blocks until the transaction completes or times out
 */
pr_error_t pr_i2c_bus_write(
    struct pr_i2c_bus *bus,
    uint8_t saddr,
    const uint8_t *data,
    size_t size);

/**
 * @brief Read data from an I2C device
 *
 * Performs a complete I2C read transaction. The bus is acquired, data is
 * received, and the bus is released. This is a blocking operation.
 *
 * @param bus Pointer to I2C bus structure
 * @param saddr 7-bit I2C device address (not shifted)
 * @param data Pointer to buffer to receive data
 * @param size Number of bytes to read
 * @return PR_OK on success, error code on failure (e.g., NACK, timeout)
 *
 * @note This function blocks until the transaction completes or times out
 */
pr_error_t pr_i2c_bus_read(
    struct pr_i2c_bus *bus,
    uint8_t saddr, uint8_t *data,
    size_t size);

/**
 * @brief Write then read from an I2C device
 *
 * Performs a combined write-read transaction with a repeated START condition.
 * This is commonly used to write a register address then read the register
 * value without releasing the bus. This is a blocking operation.
 *
 * @param bus Pointer to I2C bus structure
 * @param saddr 7-bit I2C device address (not shifted)
 * @param wdata Pointer to data to write
 * @param wsize Number of bytes to write
 * @param rdata Pointer to buffer to receive read data
 * @param rsize Number of bytes to read
 * @return PR_OK on success, error code on failure (e.g., NACK, timeout)
 *
 * @note This function blocks until the transaction completes or times out
 * @note The write and read occur in a single transaction without releasing the bus
 */
pr_error_t pr_i2c_bus_write_read(
    struct pr_i2c_bus *bus,
    uint8_t saddr,
    const uint8_t *wdata, size_t wsize,
    uint8_t *rdata, size_t rsize);

/** @} */ // end of pr_i2c_bus group

#endif
