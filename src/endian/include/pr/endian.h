#ifndef PR_ENDIAN_H_
#define PR_ENDIAN_H_

#include <stdint.h>

inline
uint16_t pr_be16_na_read(const void *ptr)
{
    const uint8_t *data = (const uint8_t *)ptr;

    return ((uint16_t)data[0] << 8) | data[1];
}

inline
uint16_t pr_le16_na_read(const void *ptr)
{
    const uint8_t *data = (const uint8_t *)ptr;

    return ((uint16_t)data[1] << 8) | data[0];
}

inline
uint32_t pr_be32_na_read(const void *ptr)
{
    const uint8_t *data = (const uint8_t *)ptr;

    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16)
        | ((uint32_t)data[2] << 8) | (uint32_t)data[3];
}

inline
uint32_t pr_le32_na_read(const void *ptr)
{
    const uint8_t *data = (const uint8_t *)ptr;

    return ((uint32_t)data[3] << 24) | ((uint32_t)data[2] << 16)
        | ((uint32_t)data[1] << 8) | (uint32_t)data[0];
}

inline
void pr_be16_na_write(void *ptr, uint16_t value)
{
    uint8_t *data = (uint8_t *)ptr;

    data[0] = (value >> 8) & 0xff;
    data[1] = value & 0xff;
}

inline
void pr_le16_na_write(void *ptr, uint16_t value)
{
    uint8_t *data = (uint8_t *)ptr;

    data[0] = value & 0xff;
    data[1] = (value >> 8) & 0xff;
}

inline
void pr_be32_na_write(void *ptr, uint32_t value)
{
    uint8_t *data = (uint8_t *)ptr;

    data[0] = (value >> 24) & 0xff;
    data[1] = (value >> 16) & 0xff;
    data[2] = (value >> 8) & 0xff;
    data[3] = value & 0xff;
}

inline
void pr_le32_na_write(void *ptr, uint32_t value)
{
    uint8_t *data = (uint8_t *)ptr;

    data[0] = value & 0xff;
    data[1] = (value >> 8) & 0xff;
    data[2] = (value >> 16) & 0xff;
    data[3] = (value >> 24) & 0xff;
}

#endif
