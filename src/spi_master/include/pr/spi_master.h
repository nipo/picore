#ifndef PR_SPI_MASTER_H
#define PR_SPI_MASTER_H

#include <stddef.h>
#include <stdint.h>
#include <pr/error.h>

struct spi_inst;

struct pr_spi_master
{
    struct spi_inst *instance;
};

pr_error_t pr_spi_master_init(struct pr_spi_master *spi,
                              int8_t sck, int8_t mosi, int8_t miso);

/* 0x00 will be sent if mosi is NULL.
   MISO data will be ignored if miso is NULL. */
pr_error_t pr_spi_master_transfer(
    struct pr_spi_master *spi,
    uint8_t spi_mode, uint32_t rate,
    const uint8_t *mosi, uint8_t *miso, size_t byte_count);

#define PR_SPI_DEVICE_MODE_CSN_PP 4
#define PR_SPI_DEVICE_MODE_LSB_FIRST 8

struct pr_spi_device
{
    struct pr_spi_master *master;
    int8_t csn;
    uint32_t bitrate;
    uint8_t mode;
};

pr_error_t pr_spi_device_init(struct pr_spi_device *dev,
                              struct pr_spi_master *master,
                              int8_t csn, uint8_t mode,
                              uint32_t bitrate);

enum pr_spi_transfer_flags
{
    /* Do not touch CS */
    PR_SPI_NO_CS,
    /* Activate CS at start, do ont release */
    PR_SPI_KEEP_CS,
};

pr_error_t pr_spi_device_transfer(
    struct pr_spi_device *dev,
    const uint8_t *mosi, uint8_t *miso, size_t byte_count,
    uint8_t flags);

#endif
