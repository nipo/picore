/**
 * @file crc32.h
 * @defgroup pr_crc32 CRC32
 * @brief CRC32C (Castagnoli) calculation
 *
 * This module provides CRC32C calculation using the Castagnoli polynomial.
 * CRC32C is used in protocols like iSCSI, SCTP, and provides better error
 * detection than the standard CRC32 polynomial.
 *
 * The function supports incremental CRC calculation, allowing data to be
 * processed in chunks.
 *
 * @{
 */

#ifndef PR_CRC32_H_
#define PR_CRC32_H_

#include <sys/types.h>
#include <stdint.h>

/**
 * @brief Update CRC32C checksum with new data
 *
 * Calculates or updates a CRC32C checksum over the provided data.
 * Supports incremental calculation by passing the previous state.
 *
 * @param state Previous CRC32C state (use 0 for first call)
 * @param data Pointer to data to process
 * @param size Number of bytes to process
 * @return Updated CRC32C checksum
 *
 * Example:
 * @code
 * uint32_t crc = 0;
 * crc = pr_crc32c_update(crc, data1, len1);
 * crc = pr_crc32c_update(crc, data2, len2);
 * // Final crc value
 * @endcode
 */
uint32_t pr_crc32c_update(uint32_t state, const uint8_t *data, size_t size);

/** @} */ // end of pr_crc32 group

#endif
