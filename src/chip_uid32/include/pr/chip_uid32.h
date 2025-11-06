/**
 * @file chip_uid32.h
 * @defgroup pr_chip_uid32 Chip Unique ID
 * @brief Chip unique identifier utilities for RP2040
 *
 * The chip_uid32 library provides access to the unique identifier of
 * the RP2040 instance. RP2040 relies on its attached QSPI flash for
 * unique ID.  This library computes a 32-bit hash from the full
 * 64-bit identifier for use in applications requiring a compact
 * unique ID.
 *
 * The unique ID can be used for:
 * - Device identification and registration
 * - USB serial number generation
 * - License validation
 * - Network MAC address generation
 *
 * Typical usage pattern:
 * @code
 * #include <pr/chip_uid32.h>
 * #include <stdio.h>
 *
 * int main(void) {
 *     uint32_t uid = pr_chip_uid32();
 *     printf("Chip UID: %08lx\n", uid);
 *     // Output: Chip UID: 12345678
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_CHIP_UID32_H_
#define PR_CHIP_UID32_H_

#include <stdint.h>

/**
 * @brief Get the 32-bit chip unique identifier
 *
 * Returns a 32-bit unique identifier derived from the RP2040's
 * attached QSPI flash 64-bit unique ID. The full 64-bit ID is read
 * from the flash and hashed to produce a 32-bit value. Each hardware
 * instance should return a value with a low-enough probability of
 * collision.
 *
 * @return 32-bit unique identifier for this chip
 *
 * @note The returned value is deterministic - calling this function multiple
 *       times will return the same value for a given chip.
 * @note This function reads from flash memory, so it may take a few
 *       microseconds to execute.
 */
uint32_t pr_chip_uid32(void);

/** @} */ // end of pr_chip_uid32 group

#endif
