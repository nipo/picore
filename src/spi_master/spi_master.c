#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pr/error.h>
#include <pr/spi_master.h>

//#define dprintf printf
#define dprintf(...) do{}while(0)

static
spi_inst_t *instance_for_pin(int8_t pin)
{
    if (pin < 0)
        return NULL;

    int index = (pin & 8) >> 3;

    return index ? spi1 : spi0;
}

pr_error_t pr_spi_master_init(struct pr_spi_master *spi,
                              int8_t sck, int8_t mosi, int8_t miso)
{
    spi_inst_t *sck_inst = instance_for_pin(sck);
    spi_inst_t *mosi_inst = instance_for_pin(mosi);
    spi_inst_t *miso_inst = instance_for_pin(miso);

    if (!sck_inst)
        return PR_ERR_INVAL;

    if (mosi_inst && (sck_inst != mosi_inst))
        return PR_ERR_INVAL;

    if (miso_inst && (sck_inst != miso_inst))
        return PR_ERR_INVAL;

    spi->instance = sck_inst;

    spi_init(spi->instance, 100000);

    gpio_set_function(sck, GPIO_FUNC_SPI);
    gpio_set_pulls(sck, 0, 0);
    gpio_set_dir(sck, 1);
    gpio_set_slew_rate(sck, 1);
    if (mosi_inst) {
        gpio_set_function(mosi, GPIO_FUNC_SPI);
        gpio_set_pulls(mosi, 0, 0);
        gpio_set_dir(mosi, 1);
        gpio_set_slew_rate(mosi, 1);
    }
    if (miso_inst) {
        gpio_set_function(miso, GPIO_FUNC_SPI);
        gpio_set_pulls(miso, 0, 0);
        gpio_set_dir(miso, 0);
    }

    dprintf("spi OK SCK %d MOSI %d MISO %d, instance %d\n",
           sck, mosi, miso, spi_get_index(sck_inst));

    return PR_OK;
}

pr_error_t pr_spi_master_transfer(
    struct pr_spi_master *spi,
    uint8_t spi_mode, uint32_t bitrate,
    const uint8_t *mosi, uint8_t *miso, size_t byte_count)
{
    spi_set_format(spi->instance, 8,
                   !!(spi_mode & 2),
                   spi_mode & 1,
                   spi_mode & 8 ? SPI_LSB_FIRST : SPI_MSB_FIRST);
    spi_set_baudrate(spi->instance, bitrate);


    if (mosi && miso) {
        spi_write_read_blocking(spi->instance,
                                mosi, miso, byte_count);
        return PR_OK;
    } else if (mosi) {
        spi_write_blocking(spi->instance,
                           mosi, byte_count);
        return PR_OK;
    } else if (miso) {
        spi_read_blocking(spi->instance,
                          0, miso, byte_count);
        return PR_OK;
    } else {
        return PR_ERR_INVAL;
    }
}

pr_error_t pr_spi_device_init(struct pr_spi_device *dev,
                              struct pr_spi_master *master,
                              int8_t csn, uint8_t mode,
                              uint32_t bitrate)
{
    dev->master = master;
    dev->csn = csn;
    dev->mode = mode;
    dev->bitrate = bitrate;

    gpio_init(csn);
    gpio_set_function(csn, GPIO_FUNC_SIO);
    gpio_set_pulls(csn, 1, 0);
    if (dev->mode & PR_SPI_DEVICE_MODE_CSN_PP) {
        gpio_put(csn, 1);
        gpio_set_dir(csn, 1);
    } else {
        gpio_put(csn, 0);
        gpio_set_dir(csn, 0);
    }

    dprintf("spi slave OK, cs %d, mode %d, rate %d\n", csn, mode, bitrate);

    return PR_OK;
}

pr_error_t pr_spi_device_transfer(
    struct pr_spi_device *dev,
    const uint8_t *mosi, uint8_t *miso, size_t byte_count,
    uint8_t flags)
{
    if (!(flags & PR_SPI_NO_CS)) {
        if (dev->mode & PR_SPI_DEVICE_MODE_CSN_PP)
            gpio_put(dev->csn, 0);
        else
            gpio_set_dir(dev->csn, 1);
    }

    pr_error_t ret = pr_spi_master_transfer(dev->master,
                                            dev->mode,
                                            dev->bitrate,
                                            mosi, miso, byte_count);
    
    if (!(flags & (PR_SPI_NO_CS | PR_SPI_KEEP_CS))) {
        if (dev->mode & PR_SPI_DEVICE_MODE_CSN_PP)
            gpio_put(dev->csn, 1);
        else
            gpio_set_dir(dev->csn, 0);
    }

    return ret;
}

