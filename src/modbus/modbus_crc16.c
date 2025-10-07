#include <stdint.h>
#include <sys/types.h>
#include "modbus_crc16.h"

/*
  Calculation:
  - Polynom representation: exponent 0 at MSB
  - Order: 16
  - Divisor: 0x14003
    Exponents: x^16 + x^15 + x^2 + 1
    - Initial value: 0xffff
      Exponents: x^16 + x^15 + x^14 + x^13 + x^12 + x^11 + x^10 + x^9 + x^8 + x^7 + x^6 + x^5 + x^4 + x^3 + x^2 + x^1
      Bitstream processing:
      - Read bits in bytestream from LSB of each byte
      - Complement input data: False
      - Complement internal state: False
      Output generation:
      - Bitswap output: False
      - Byte order: little
      Properties:
      - Transparent to post-image zero-padding
      - CRC state after blob with valid CRC: 0x0
      - CRC bytes computed over a blob with valid CRC: <0000>
      Trivia:
      - Crobe short definition: 0x14003:0xffffll
      - Crobe instantiation: Crc(poly = 0x14003, init = 0xffff, pop_lsb = True, order0_at_lsb = False, complement_input = False, complement_state = False, spill_bitswap = False, spill_byte_order = 'little')
      - Reveng-like definition: width=16 poly=0x4003 init=0xffff refin=true refout=false xorout=0x0000 check=0x4b37 residue=0x0000
*/

static inline
modbus_crc16_state_t modbus_crc16_insert1(modbus_crc16_state_t state, uint8_t word)
{
    static const modbus_crc16_state_t state_update_table[] = {
        0x0000, 0xa001,
    };

    const uint8_t index = (state ^ word) & 0x1;
    const modbus_crc16_state_t shifted_state = (state >> 1) & 0xffff;
    return shifted_state ^ state_update_table[index];
}

modbus_crc16_final_t modbus_crc16_initialize(modbus_crc16_state_t init)
{
    modbus_crc16_state_t state = init;
    return state;
}

modbus_crc16_state_t modbus_crc16_update(modbus_crc16_state_t state, const uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        const uint8_t word = data[i];
        for (int8_t b = 0; b < 8; b += 1)
            state = modbus_crc16_insert1(state, (word >> b) & 0x1);
    }

    return state;
}

modbus_crc16_final_t modbus_crc16_finalize(modbus_crc16_state_t state)
{
    return state;
}

void modbus_crc16_serialize(uint8_t dest[static 2], modbus_crc16_state_t final_value)
{
    for (ssize_t i = 0; i < 2; ++i) {
        dest[i] = final_value & 0xff;
        final_value >>= 8;
    }
}
