/**
 * @file stdio_fifo.h
 * @defgroup pr_stdio_fifo Standard I/O FIFO Binding
 * @brief Redirect standard I/O streams to FIFO buffers
 *
 * The stdio_fifo library provides a bridge between the standard C library
 * I/O functions (printf, scanf, getchar, putchar, etc.) and FIFO buffers.
 * This enables stdout and stdin to be redirected to any FIFO-based transport
 * such as UART, USB CDC, or network sockets.
 *
 * Once bound, all standard I/O operations will use the provided FIFOs:
 * - printf, puts, putchar -> write to stdout FIFO
 * - scanf, getchar -> read from stdin FIFO
 *
 * This is particularly useful for:
 * - Redirecting console output to USB or UART
 * - Implementing remote shells over serial or network
 * - Testing and debugging with virtual I/O
 *
 * Typical usage pattern:
 * @code
 * #include <pr/stdio_fifo.h>
 * #include <pr/fifo.h>
 * #include <stdio.h>
 *
 * struct pr_fifo stdout_fifo, stdin_fifo;
 *
 * int main(void) {
 *     pr_fifo_init(&stdout_fifo, 1024);
 *     pr_fifo_init(&stdin_fifo, 256);
 *
 *     // Bind stdio to FIFOs
 *     pr_stdio_fifo_driver_bind(&stdout_fifo, &stdin_fifo);
 *
 *     // Now printf goes to stdout_fifo
 *     printf("Hello, world!\n");
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_STDIO_FIFO_H_
#define PR_STDIO_FIFO_H_

#include <pr/fifo.h>

/**
 * @brief Bind standard I/O streams to FIFO buffers
 *
 * Redirects standard output and standard input to the provided FIFO buffers.
 * After calling this function, all standard C library I/O operations will
 * read from and write to these FIFOs instead of the default console.
 *
 * @param fifo_stdout FIFO to receive standard output data (printf, puts, etc.)
 *                    Pass NULL to leave stdout unchanged
 * @param fifo_stdin FIFO to provide standard input data (scanf, getchar, etc.)
 *                   Pass NULL to leave stdin unchanged
 *
 * @note This function should be called once during initialization before
 *       performing any I/O operations
 * @note The FIFOs must remain valid for the lifetime of the program
 * @note This affects ALL standard I/O operations in the program
 */
void pr_stdio_fifo_driver_bind(struct pr_fifo *fifo_stdout, struct pr_fifo *fifo_stdin);

/** @} */ // end of pr_stdio_fifo group

#endif
