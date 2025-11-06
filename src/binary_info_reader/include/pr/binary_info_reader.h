/**
 * @file binary_info_reader.h
 * @defgroup pr_binary_info_reader Binary Info Reader
 * @brief Reader for Pico SDK binary_info metadata embedded in binaries
 *
 * The binary_info_reader library provides utilities for reading metadata
 * embedded in Raspberry Pi Pico binaries by the Pico SDK's binary_info system.
 * This metadata includes program name, version, build date, GPIO pin mappings,
 * and other configuration information that can be extracted without running
 * the program.
 *
 * The Pico SDK's picotool utility uses this same format to display information
 * about .uf2 and .elf files. This library allows programs to read their own
 * metadata at runtime.
 *
 * Key features:
 * - Locate binary info descriptors in flash
 * - Iterate through all binary info entries
 * - Search for specific metadata by type and tag
 * - Helper functions for common metadata (program name, build date, etc.)
 *
 * Typical usage pattern:
 * @code
 * #include <pr/binary_info_reader.h>
 * #include <stdio.h>
 *
 * int main(void) {
 *     // Get common metadata
 *     const char *program_name = pr_binary_info_program_name();
 *     const char *build_date = pr_binary_info_build_date();
 *
 *     printf("Program: %s\n", program_name);
 *     printf("Built: %s\n", build_date);
 *
 *     // Or iterate through all entries
 *     const struct binary_info_descriptor *desc = pr_bi_desc_find();
 *     uint32_t state = pr_bi_start(desc);
 *     while (state) {
 *         const binary_info_core_t *info = pr_bi_get(desc, state);
 *         // Process info...
 *         state = pr_bi_next(desc, state);
 *     }
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_BINARY_INFO_READER_H
#define PR_BINARY_INFO_READER_H

#include <stdint.h>
#include <pico/binary_info.h>

struct binary_info_descriptor;

/**
 * @brief Find the binary info descriptor in flash
 *
 * Locates the binary info descriptor structure embedded in the program's
 * flash memory. This descriptor contains pointers to all binary info entries.
 *
 * @return Pointer to binary info descriptor, or NULL if not found
 *
 * @note The descriptor is located by searching for a magic header in flash
 * @note This function caches the result on first call
 */
const struct binary_info_descriptor *pr_bi_desc_find(void);

/**
 * @brief Get starting state for iterating binary info entries
 *
 * Returns the initial state value for beginning iteration through all
 * binary info entries in the descriptor.
 *
 * @param desc Pointer to binary info descriptor from pr_bi_desc_find()
 * @return Initial state value, or 0 if descriptor is NULL
 *
 * @see pr_bi_next(), pr_bi_get()
 */
uint32_t pr_bi_start(const struct binary_info_descriptor *desc);

/**
 * @brief Get next state for iterating binary info entries
 *
 * Advances the iteration state to the next binary info entry.
 *
 * @param desc Pointer to binary info descriptor
 * @param state Current state value
 * @return Next state value, or 0 when iteration is complete
 *
 * @see pr_bi_start(), pr_bi_get()
 */
uint32_t pr_bi_next(const struct binary_info_descriptor *desc,
                    uint32_t state);

/**
 * @brief Get binary info entry for current state
 *
 * Returns a pointer to the binary info entry corresponding to the
 * given state value.
 *
 * @param desc Pointer to binary info descriptor
 * @param state State value from pr_bi_start() or pr_bi_next()
 * @return Pointer to binary info entry, or NULL if state is invalid
 *
 * @see pr_bi_start(), pr_bi_next()
 */
const binary_info_core_t *pr_bi_get(const struct binary_info_descriptor *desc,
                                    uint32_t state);

/**
 * @brief Find binary info entry by type and tag
 *
 * Searches for a binary info entry matching the specified type and tag.
 * Can be called repeatedly to find multiple matching entries.
 *
 * @param desc Pointer to binary info descriptor
 * @param state Pointer to state variable (in/out). Initialize to pr_bi_start()
 *              value before first call. Updated to point past found entry.
 * @param type Binary info type to search for (e.g., BINARY_INFO_TYPE_ID_AND_STRING)
 * @param tag Binary info tag to search for (e.g., BINARY_INFO_TAG_RASPBERRY_PI)
 * @return Pointer to matching binary info entry, or NULL if not found
 *
 * @note To find multiple matches, call repeatedly with the same state pointer
 */
const binary_info_core_t *pr_bi_find(const struct binary_info_descriptor *desc,
                                     uint32_t *state,
                                     uint16_t type, uint16_t tag);

/**
 * @brief Find raw data binary info entry
 *
 * Searches for a BINARY_INFO_TYPE_RAW_DATA entry with the specified tag.
 * Helper function that calls pr_bi_find() with the appropriate type.
 *
 * @param desc Pointer to binary info descriptor
 * @param state Pointer to state variable (in/out)
 * @param tag Binary info tag to search for
 * @return Pointer to raw data, or NULL if not found
 */
const void *pr_bi_find_raw(const struct binary_info_descriptor *desc,
                           uint32_t *state,
                           uint16_t tag);

/**
 * @brief Find sized data binary info entry
 *
 * Searches for a BINARY_INFO_TYPE_SIZED_DATA entry with the specified tag.
 * Helper function that calls pr_bi_find() with the appropriate type.
 *
 * @param desc Pointer to binary info descriptor
 * @param state Pointer to state variable (in/out)
 * @param tag Binary info tag to search for
 * @param length Output pointer to receive data length in bytes
 * @return Pointer to data, or NULL if not found
 */
const void *pr_bi_find_sized_data(const struct binary_info_descriptor *desc,
                                  uint32_t *state,
                                  uint16_t tag, uint32_t *length);

/**
 * @brief Find integer binary info entry
 *
 * Searches for a BINARY_INFO_TYPE_ID_AND_INT entry with the specified
 * tag and ID. Helper function that calls pr_bi_find() with the appropriate type.
 *
 * @param desc Pointer to binary info descriptor
 * @param state Pointer to state variable (in/out)
 * @param tag Binary info tag to search for
 * @param id Binary info ID to match
 * @return Integer value from entry, or 0 if not found
 */
int32_t pr_bi_find_int(const struct binary_info_descriptor *desc,
                       uint32_t *state,
                       uint16_t tag, uint32_t id);

/**
 * @brief Find string binary info entry
 *
 * Searches for a BINARY_INFO_TYPE_ID_AND_STRING entry with the specified
 * tag and ID. Helper function that calls pr_bi_find() with the appropriate type.
 *
 * @param desc Pointer to binary info descriptor
 * @param state Pointer to state variable (in/out)
 * @param tag Binary info tag to search for
 * @param id Binary info ID to match
 * @return Pointer to null-terminated string, or NULL if not found
 */
const char *pr_bi_find_string(const struct binary_info_descriptor *desc,
                              uint32_t *state,
                              uint16_t tag, uint32_t id);

/**
 * @brief Get program name from binary info
 *
 * Convenience function to retrieve the program name embedded in the binary.
 * This is typically set with bi_decl(bi_program_name("My Program")).
 *
 * @return Pointer to program name string, or NULL if not found
 */
const char *pr_binary_info_program_name(void);

/**
 * @brief Get build date from binary info
 *
 * Convenience function to retrieve the build date embedded in the binary.
 * This is typically set automatically by the build system.
 *
 * @return Pointer to build date string, or NULL if not found
 */
const char *pr_binary_info_build_date(void);

/**
 * @brief Get board name from binary info
 *
 * Convenience function to retrieve the target board name embedded in the binary.
 * This is typically set by the Pico SDK based on the selected board.
 *
 * @return Pointer to board name string, or NULL if not found
 */
const char *pr_binary_info_board_name(void);

/** @} */ // end of pr_binary_info_reader group

#endif
