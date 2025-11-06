/**
 * @file spi_master.h
 * @defgroup pr_spi_master SPI Master
 * @brief SPI master interface with device abstraction
 *
 * The spi_master library provides SPI master mode communication with support
 * for multiple devices sharing a bus. It handles GPIO configuration, chip
 * select management, and provides both bus-level and device-level APIs.
 *
 * Key features:
 * - Multiple devices on shared SPI bus
 * - Automatic chip select handling
 * - Configurable mode, bit order, and speed per device
 * - Full-duplex and half-duplex transfers
 * - Support for CS hold across multiple transfers
 *
 * Typical usage pattern:
 * @code
 * #include <pr/spi_master.h>
 *
 * struct pr_spi_master spi;
 * struct pr_spi_device dev1, dev2;
 *
 * int main(void) {
 *     // Initialize SPI bus on GPIO 18 (SCK), 19 (MOSI), 16 (MISO)
 *     pr_spi_master_init(&spi, 18, 19, 16);
 *
 *     // Initialize device 1: CS on pin 17, mode 0, 1MHz
 *     pr_spi_device_init(&dev1, &spi, 17, 0, 1000000);
 *
 *     // Initialize device 2: CS on pin 20, mode 3, 10MHz
 *     pr_spi_device_init(&dev2, &spi, 20, 3, 10000000);
 *
 *     // Transfer data to device 1
 *     uint8_t tx[] = {0x01, 0x02, 0x03};
 *     uint8_t rx[3];
 *     pr_spi_device_transfer(&dev1, tx, rx, 3, 0);
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_SPI_MASTER_H
#define PR_SPI_MASTER_H

#include <stddef.h>
#include <stdint.h>
#include <pr/error.h>

struct spi_inst;

/**
 * @brief SPI master bus structure
 *
 * Represents an SPI master bus instance. Multiple devices can share one bus.
 */
struct pr_spi_master
{
    struct spi_inst *instance; /**< Pico SDK SPI instance */
};

/**
 * @brief Initialize an SPI master bus
 *
 * Configures GPIO pins for SPI master operation. The bus is ready to use
 * but devices must be initialized separately.
 *
 * @param spi Pointer to SPI master structure
 * @param sck GPIO pin for SCK (clock), or -1 if not used
 * @param mosi GPIO pin for MOSI (master out), or -1 if not used
 * @param miso GPIO pin for MISO (master in), or -1 if not used
 * @return PR_OK on success, error code on failure
 *
 * @note At least one of sck, mosi, or miso should be specified
 * @note Setting mosi=-1 creates a read-only SPI bus
 * @note Setting miso=-1 creates a write-only SPI bus
 */
pr_error_t pr_spi_master_init(struct pr_spi_master *spi,
                              int8_t sck, int8_t mosi, int8_t miso);

/**
 * @brief Perform SPI transfer at bus level
 *
 * Low-level SPI transfer function. Caller is responsible for chip select
 * management. Use pr_spi_device_transfer() for automatic CS handling.
 *
 * @param spi Pointer to SPI master structure
 * @param spi_mode SPI mode (0-3: CPOL/CPHA combinations)
 * @param rate Bit rate in Hz (e.g., 1000000 for 1MHz)
 * @param mosi Pointer to transmit data, or NULL to send 0x00
 * @param miso Pointer to receive buffer, or NULL to discard received data
 * @param byte_count Number of bytes to transfer
 * @return PR_OK on success, error code on failure
 *
 * @note This function does not manage chip select
 * @note If mosi is NULL, 0x00 is sent for each byte
 * @note If miso is NULL, received data is discarded
 */
pr_error_t pr_spi_master_transfer(
    struct pr_spi_master *spi,
    uint8_t spi_mode, uint32_t rate,
    const uint8_t *mosi, uint8_t *miso, size_t byte_count);

/**
 * @brief Device mode flag: CS is active low (push-pull)
 *
 * Normally CS is active low with open-drain. This flag makes it push-pull.
 */
#define PR_SPI_DEVICE_MODE_CSN_PP 4

/**
 * @brief Device mode flag: LSB first
 *
 * Normally data is transmitted MSB first. This flag selects LSB first.
 */
#define PR_SPI_DEVICE_MODE_LSB_FIRST 8

/**
 * @brief SPI device structure
 *
 * Represents a device on an SPI bus with its configuration.
 */
struct pr_spi_device
{
    struct pr_spi_master *master; /**< SPI bus this device is on */
    int8_t csn;                   /**< Chip select GPIO pin, or -1 */
    uint32_t bitrate;             /**< Device bit rate in Hz */
    uint8_t mode;                 /**< SPI mode (0-3) plus optional flags */
};

/**
 * @brief Initialize an SPI device
 *
 * Configures an SPI device on a bus with its chip select, mode, and speed.
 *
 * @param dev Pointer to SPI device structure
 * @param master Pointer to SPI master bus
 * @param csn GPIO pin for chip select (active low), or -1 for no CS
 * @param mode SPI mode (0-3) optionally OR'd with PR_SPI_DEVICE_MODE_* flags
 * @param bitrate Bit rate in Hz for this device
 * @return PR_OK on success, error code on failure
 *
 * @note The CS pin is configured as output and set inactive (high)
 * @note mode can include PR_SPI_DEVICE_MODE_CSN_PP or PR_SPI_DEVICE_MODE_LSB_FIRST
 */
pr_error_t pr_spi_device_init(struct pr_spi_device *dev,
                              struct pr_spi_master *master,
                              int8_t csn, uint8_t mode,
                              uint32_t bitrate);

/**
 * @brief SPI transfer flags
 *
 * Flags to control chip select behavior during transfers.
 */
enum pr_spi_transfer_flags
{
    PR_SPI_NO_CS,   /**< Do not touch CS (for manual control) */
    PR_SPI_KEEP_CS, /**< Activate CS at start, do not release at end */
};

/**
 * @brief Perform SPI transfer to a device
 *
 * Transfers data to/from an SPI device with automatic chip select management.
 * The bus is configured for this device's mode and speed.
 *
 * @param dev Pointer to SPI device structure
 * @param mosi Pointer to transmit data, or NULL to send 0x00
 * @param miso Pointer to receive buffer, or NULL to discard received data
 * @param byte_count Number of bytes to transfer
 * @param flags Transfer flags (0, PR_SPI_NO_CS, or PR_SPI_KEEP_CS)
 * @return PR_OK on success, error code on failure
 *
 * @note With flags=0, CS is asserted, transfer occurs, CS is released
 * @note With PR_SPI_KEEP_CS, CS stays active after transfer (for multiple transfers)
 * @note With PR_SPI_NO_CS, caller manages CS manually
 * @note If mosi is NULL, 0x00 is sent for each byte
 * @note If miso is NULL, received data is discarded
 */
pr_error_t pr_spi_device_transfer(
    struct pr_spi_device *dev,
    const uint8_t *mosi, uint8_t *miso, size_t byte_count,
    uint8_t flags);

/** @} */ // end of pr_spi_master group

#endif
