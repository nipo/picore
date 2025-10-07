#include <stdint.h>
#include <stdlib.h>
typedef uint16_t modbus_crc16_state_t;
typedef uint16_t modbus_crc16_final_t;
static const modbus_crc16_state_t modbus_crc16_init = 0xffff;
static const modbus_crc16_state_t modbus_crc16_check_state = 0x0;

modbus_crc16_final_t modbus_crc16_initialize(modbus_crc16_state_t init);
modbus_crc16_state_t modbus_crc16_update(modbus_crc16_state_t state, const uint8_t *data, size_t size);
modbus_crc16_final_t modbus_crc16_finalize(modbus_crc16_state_t state);
void modbus_crc16_serialize(uint8_t dest[static 2], modbus_crc16_state_t final_value);
