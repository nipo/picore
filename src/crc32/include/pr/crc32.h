#include <sys/types.h>
#include <stdint.h>

uint32_t pr_crc32c_update(uint32_t state, const uint8_t *data, size_t size);
