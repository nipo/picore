/**
 * @file error.h
 * @defgroup pr_error Error Codes
 * @brief Common error code definitions for Pico-RE
 *
 * This module defines standard error codes used throughout the Pico-RE library.
 * All error codes are negative values except PR_OK which is zero. Functions
 * that return pr_error_t typically return PR_OK (0) on success and a negative
 * error code on failure.
 *
 * @{
 */

#ifndef PR_ERROR_H_
#define PR_ERROR_H_

#include <stdint.h>

/**
 * @brief Error code type
 *
 * Signed 8-bit integer for error codes. Zero indicates success, negative
 * values indicate various error conditions.
 */
typedef int8_t pr_error_t;

/** @brief Success (no error) */
#define PR_OK 0

/** @brief Unknown or unspecified error */
#define PR_ERR_UNKNOWN -1

/** @brief Memory allocation failure */
#define PR_ERR_MEMORY -2

/** @brief I/O operation failed */
#define PR_ERR_IO -3

/** @brief Invalid address or addressing error */
#define PR_ERR_ADDRESS -4

/** @brief Operation timed out */
#define PR_ERR_TIMEOUT -5

/** @brief Invalid argument or parameter */
#define PR_ERR_INVAL -6

/** @brief Resource busy or unavailable */
#define PR_ERR_BUSY -7

/** @brief Not implemented or unsupported operation */
#define PR_ERR_IMPL -8

/** @} */ // end of pr_error group

#endif
