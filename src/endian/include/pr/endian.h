/**
 * @file endian.h
 * @defgroup pr_endian Endian Conversion
 * @brief Unaligned big-endian and little-endian data access
 *
 * This module provides functions for reading and writing integer values
 * in big-endian or little-endian byte order from/to potentially unaligned
 * memory locations. These functions are particularly useful for protocol
 * handling and binary file formats.
 *
 * All functions handle unaligned access safely and perform explicit byte
 * ordering, making them portable across different architectures.
 *
 * @{
 */

#ifndef PR_ENDIAN_H_
#define PR_ENDIAN_H_

#include <stdint.h>

/**
 * @brief Read 16-bit big-endian value from unaligned memory
 *
 * @param ptr Pointer to data (may be unaligned)
 * @return Value in native byte order
 */
inline
uint16_t pr_be16_na_read(const void *ptr)
{
    const uint8_t *data = (const uint8_t *)ptr;

    return ((uint16_t)data[0] << 8) | data[1];
}

/**
 * @brief Read 16-bit little-endian value from unaligned memory
 *
 * @param ptr Pointer to data (may be unaligned)
 * @return Value in native byte order
 */
inline
uint16_t pr_le16_na_read(const void *ptr)
{
    const uint8_t *data = (const uint8_t *)ptr;

    return ((uint16_t)data[1] << 8) | data[0];
}

/**
 * @brief Read 32-bit big-endian value from unaligned memory
 *
 * @param ptr Pointer to data (may be unaligned)
 * @return Value in native byte order
 */
inline
uint32_t pr_be32_na_read(const void *ptr)
{
    const uint8_t *data = (const uint8_t *)ptr;

    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16)
        | ((uint32_t)data[2] << 8) | (uint32_t)data[3];
}

/**
 * @brief Read 32-bit little-endian value from unaligned memory
 *
 * @param ptr Pointer to data (may be unaligned)
 * @return Value in native byte order
 */
inline
uint32_t pr_le32_na_read(const void *ptr)
{
    const uint8_t *data = (const uint8_t *)ptr;

    return ((uint32_t)data[3] << 24) | ((uint32_t)data[2] << 16)
        | ((uint32_t)data[1] << 8) | (uint32_t)data[0];
}

/**
 * @brief Write 16-bit value as big-endian to unaligned memory
 *
 * @param ptr Pointer to destination (may be unaligned)
 * @param value Value to write in native byte order
 */
inline
void pr_be16_na_write(void *ptr, uint16_t value)
{
    uint8_t *data = (uint8_t *)ptr;

    data[0] = (value >> 8) & 0xff;
    data[1] = value & 0xff;
}

/**
 * @brief Write 16-bit value as little-endian to unaligned memory
 *
 * @param ptr Pointer to destination (may be unaligned)
 * @param value Value to write in native byte order
 */
inline
void pr_le16_na_write(void *ptr, uint16_t value)
{
    uint8_t *data = (uint8_t *)ptr;

    data[0] = value & 0xff;
    data[1] = (value >> 8) & 0xff;
}

/**
 * @brief Write 32-bit value as big-endian to unaligned memory
 *
 * @param ptr Pointer to destination (may be unaligned)
 * @param value Value to write in native byte order
 */
inline
void pr_be32_na_write(void *ptr, uint32_t value)
{
    uint8_t *data = (uint8_t *)ptr;

    data[0] = (value >> 24) & 0xff;
    data[1] = (value >> 16) & 0xff;
    data[2] = (value >> 8) & 0xff;
    data[3] = value & 0xff;
}

/**
 * @brief Write 32-bit value as little-endian to unaligned memory
 *
 * @param ptr Pointer to destination (may be unaligned)
 * @param value Value to write in native byte order
 */
inline
void pr_le32_na_write(void *ptr, uint32_t value)
{
    uint8_t *data = (uint8_t *)ptr;

    data[0] = value & 0xff;
    data[1] = (value >> 8) & 0xff;
    data[2] = (value >> 16) & 0xff;
    data[3] = (value >> 24) & 0xff;
}

/** @} */ // end of pr_endian group

#endif
