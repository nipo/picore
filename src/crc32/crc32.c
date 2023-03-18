#include <stdint.h>
#include <pr/crc32.h>

uint32_t pr_crc32c_update(uint32_t crc, const uint8_t *data, size_t size)
{
#define U(x, sh) (((x) & (1 << (sh))) ? (0xEDB88320 >> (3 - (sh))) : 0)
#define T(x) (U(x, 0) ^ U(x, 1) ^ U(x, 2) ^ U(x, 3))
    const uint32_t t[16] = {
        T(0), T(1), T(2), T(3), 
        T(4), T(5), T(6), T(7), 
        T(8), T(9), T(10), T(11), 
        T(12), T(13), T(14), T(15), 
    };

    const uint8_t *d = data;

    crc = ~crc;
    
    for (size_t i = 0; i < size; ++i) {
        crc = crc ^ d[i];
        crc = (crc >> 4) ^ t[crc & 0xf];
        crc = (crc >> 4) ^ t[crc & 0xf];
    }

    return ~crc;
}

