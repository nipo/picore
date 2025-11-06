/**
 * @file build_id.h
 * @defgroup pr_build_id Build Identifier
 * @brief Build identifier utilities for extracting binary build information
 *
 * The build_id library provides access to the build identifier
 * embedded in the binary by the linker. This is typically a unique ID
 * for version tracking and identification.
 *
 * The build ID is automatically generated during the linking process and
 * provides a convenient way to uniquely identify a specific build of the
 * firmware without manual version management.
 *
 * Typical usage pattern:
 * @code
 * #include <pr/build_id.h>
 * #include <stdio.h>
 *
 * int main(void) {
 *     const char *build_id = pr_build_id_get();
 *     printf("Build ID: %s\n", build_id);
 *     // Output: Build ID: a1b2c3d4e5f6...
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_BUILD_ID_H_
#define PR_BUILD_ID_H_

#include <stdint.h>

/**
 * @brief Get the build identifier string
 *
 * Returns a pointer to a null-terminated string containing the build
 * identifier. The build ID is a UUID embedded in the binary by the
 * linker.
 *
 * @return Pointer to build identifier string (never NULL)
 *
 * @note The returned pointer points to read-only memory and remains
 *       valid for the lifetime of the program.
 */
const char *pr_build_id_get(void);

/** @} */ // end of pr_build_id group

#endif
