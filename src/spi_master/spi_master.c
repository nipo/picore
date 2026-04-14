#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pr/error.h>
#include <pr/spi_master.h>

//#define dprintf printf
#define dprintf(...) do{}while(0)

// RP2040's PL022 SSP only supports MSB-first. For LSB-first devices,
// we do byte-level bit reversal in software.
static const uint8_t bitrev_table[256] = {
    0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
    0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
    0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
    0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
    0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
    0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
    0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
    0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
    0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
    0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
    0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
    0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
    0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
    0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
    0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
    0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF,
};

static void bitrev_buf(uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; i++)
        buf[i] = bitrev_table[buf[i]];
}

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
                   SPI_MSB_FIRST);
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
        uint8_t z[16] = {};
        size_t left = byte_count;
        while (left) {
            size_t count = left > sizeof(z) ? sizeof(z) : left;
            spi_write_blocking(spi->instance, z, count);
            left -= count;
        }
        return PR_OK;
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

// Chunk size for software bit-reversal of MOSI data.
// MOSI may point to flash/rodata so we copy+reverse into a stack buffer.
// Use a large chunk to avoid intra-transfer pauses that some SPI slaves
// (e.g. CG5317 firmware) may not tolerate.
#define BITREV_CHUNK_SIZE 1600

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

    if (!(dev->mode & PR_SPI_DEVICE_MODE_LSB_FIRST)) {
        // MSB-first: straight through to hardware
        pr_spi_master_transfer(dev->master,
                               dev->mode,
                               dev->bitrate,
                               mosi, miso, byte_count);
    } else {
        // LSB-first: software bit-reversal.
        // Process in chunks so MOSI (which may be in flash) gets
        // reversed into a small stack buffer.
        uint8_t tx_chunk[BITREV_CHUNK_SIZE];
        uint8_t hw_mode = dev->mode & ~PR_SPI_DEVICE_MODE_LSB_FIRST;
        size_t offset = 0;

        while (offset < byte_count) {
            size_t chunk = byte_count - offset;
            if (chunk > BITREV_CHUNK_SIZE)
                chunk = BITREV_CHUNK_SIZE;

            const uint8_t *tx = NULL;
            uint8_t *rx = miso ? miso + offset : NULL;

            if (mosi) {
                for (size_t i = 0; i < chunk; i++)
                    tx_chunk[i] = bitrev_table[mosi[offset + i]];
                tx = tx_chunk;
            }

            pr_spi_master_transfer(dev->master,
                                   hw_mode, dev->bitrate,
                                   tx, rx, chunk);

            if (rx)
                bitrev_buf(rx, chunk);

            offset += chunk;
        }
    }

    if (!(flags & (PR_SPI_NO_CS | PR_SPI_KEEP_CS))) {
        if (dev->mode & PR_SPI_DEVICE_MODE_CSN_PP)
            gpio_put(dev->csn, 1);
        else
            gpio_set_dir(dev->csn, 0);
    }

    return PR_OK;
}
