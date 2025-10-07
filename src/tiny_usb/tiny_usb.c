#include <string.h>
#include <pr/task.h>
#include <pr/tiny_usb.h>
#include <pico/unique_id.h>
#include <pr/crc32.h>


uint8_t pr_rpi_resetd_itf_num;

static
uint16_t to_hex(uint8_t v)
{
    v &= 0xf;

    if (v < 10)
        return '0' + v;
    return v - 10 + 'a';
}

uint16_t* pr_usb_serial_number(void)
{
    static uint16_t serno_data[8 + 2] = {};

    if (serno_data[0])
        return serno_data;

    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);

    uint32_t uid_crc = pr_crc32c_update(0, id.id, sizeof(id.id));
    size_t len;

    for (len = 0; len < 8; ++len) {
        serno_data[1 + len] = to_hex((uid_crc >> 28) & 0xf);
        uid_crc <<= 4;
    }

    serno_data[0] = (3 << 8) | (2 * len + 2);
    
    return serno_data;
}
