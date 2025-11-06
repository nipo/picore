/**
 * @file i2c_eeprom.h
 * @defgroup pr_i2c_eeprom I2C EEPROM
 * @brief I2C EEPROM device driver with support for multiple chip types
 *
 * The i2c_eeprom library provides a unified interface for reading and writing
 * I2C EEPROM devices. It uses a vtable-based architecture to support different
 * EEPROM chips with varying addressing schemes and page sizes.
 *
 * Supported EEPROM types:
 * - 24LC08: 1KB EEPROM with 8-bit addressing
 * - 24AA64: 8KB EEPROM with 16-bit addressing
 *
 * Key features:
 * - Abstracted interface for different EEPROM types
 * - Automatic handling of addressing differences
 * - Read and write operations at arbitrary addresses
 * - Built on pr_i2c_bus infrastructure
 *
 * Typical usage pattern:
 * @code
 * #include <pr/i2c_eeprom.h>
 * #include <pr/i2c_bus.h>
 *
 * struct pr_i2c_bus i2c;
 * struct pr_i2c_eeprom eeprom;
 *
 * int main(void) {
 *     pr_i2c_bus_init(&i2c, 4, 5, 400000);
 *
 *     // Initialize 24AA64 EEPROM at address 0x50
 *     pr_i2c_eeprom_init(&eeprom, &i2c, 0x50, PR_I2C_EEPROM_24AA64);
 *
 *     // Write data to address 0x100
 *     uint8_t data[] = {1, 2, 3, 4};
 *     pr_i2c_eeprom_write(&eeprom, 0x100, data, 4);
 *
 *     // Read back
 *     uint8_t buffer[4];
 *     pr_i2c_eeprom_read(&eeprom, 0x100, buffer, 4);
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_I2C_EEPROM_H_
#define PR_I2C_EEPROM_H_

#include <stdint.h>
#include <pr/error.h>

struct pr_i2c_bus;
struct pr_i2c_eeprom;

/**
 * @brief I2C EEPROM virtual function table
 *
 * Function pointers for EEPROM-specific operations. Different EEPROM types
 * implement these functions with chip-specific addressing and timing.
 */
struct pr_i2c_eeprom_vtable
{
    pr_error_t (*write)(struct pr_i2c_eeprom *i2c_eeprom,
                        uint16_t addr,
                        const uint8_t *data,
                        size_t size); /**< Write function pointer */

    pr_error_t (*read)(struct pr_i2c_eeprom *i2c_eeprom,
                       uint16_t addr,
                       uint8_t *data,
                       size_t size); /**< Read function pointer */
};

/**
 * @brief I2C EEPROM device structure
 *
 * Represents an I2C EEPROM device on a bus with its vtable for operations.
 */
struct pr_i2c_eeprom
{
    struct pr_i2c_bus *bus;                     /**< I2C bus the EEPROM is on */
    uint8_t saddr;                              /**< 7-bit I2C address */
    const struct pr_i2c_eeprom_vtable *vtable;  /**< Operations vtable */
};

/**
 * @brief EEPROM type enumeration
 *
 * Supported EEPROM chip types with different capacities and addressing.
 */
enum pr_i2c_eeprom_type
{
    PR_I2C_EEPROM_24LC08,  /**< 24LC08: 1KB (8Kbit) EEPROM */
    PR_I2C_EEPROM_24AA64,  /**< 24AA64: 8KB (64Kbit) EEPROM */
};

/**
 * @brief Initialize an I2C EEPROM device
 *
 * Sets up an EEPROM device structure with the appropriate vtable for the
 * specified EEPROM type. The EEPROM is ready for read/write operations
 * after initialization.
 *
 * @param i2c_eeprom Pointer to EEPROM device structure
 * @param bus Pointer to I2C bus the EEPROM is connected to
 * @param saddr 7-bit I2C address of the EEPROM
 * @param type EEPROM type (PR_I2C_EEPROM_24LC08 or PR_I2C_EEPROM_24AA64)
 * @return PR_OK on success, error code on failure
 *
 * @note The I2C bus must be initialized before calling this function
 */
pr_error_t pr_i2c_eeprom_init(struct pr_i2c_eeprom *i2c_eeprom,
                          struct pr_i2c_bus *bus,
                          uint8_t saddr,
                          enum pr_i2c_eeprom_type type);

/**
 * @brief Write data to EEPROM
 *
 * Writes data to the EEPROM starting at the specified address. The write
 * operation handles page boundaries and timing requirements automatically
 * based on the EEPROM type.
 *
 * @param i2c_eeprom Pointer to EEPROM device structure
 * @param addr Starting address in EEPROM (0 to capacity-1)
 * @param data Pointer to data to write
 * @param size Number of bytes to write
 * @return PR_OK on success, error code on failure
 *
 * @note This function may take several milliseconds for write completion
 * @note Writing beyond EEPROM capacity will fail or wrap depending on chip
 */
static inline
pr_error_t pr_i2c_eeprom_write(struct pr_i2c_eeprom *i2c_eeprom,
                           uint16_t addr,
                           const uint8_t *data,
                           size_t size)
{
    return i2c_eeprom->vtable->write(i2c_eeprom, addr, data, size);
}

/**
 * @brief Read data from EEPROM
 *
 * Reads data from the EEPROM starting at the specified address.
 *
 * @param i2c_eeprom Pointer to EEPROM device structure
 * @param addr Starting address in EEPROM (0 to capacity-1)
 * @param data Pointer to buffer to receive data
 * @param size Number of bytes to read
 * @return PR_OK on success, error code on failure
 *
 * @note Reading beyond EEPROM capacity will fail or wrap depending on chip
 */
static inline
pr_error_t pr_i2c_eeprom_read(struct pr_i2c_eeprom *i2c_eeprom,
                          uint16_t addr,
                          uint8_t *data,
                          size_t size)
{
    return i2c_eeprom->vtable->read(i2c_eeprom, addr, data, size);
}

/** @} */ // end of pr_i2c_eeprom group

#endif
