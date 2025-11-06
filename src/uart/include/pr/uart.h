/**
 * @file uart.h
 * @defgroup pr_uart UART Driver
 * @brief UART driver with FIFO interface and task integration
 *
 * The UART driver provides asynchronous serial communication using FIFOs
 * for buffering. It integrates with the task system for interrupt-driven
 * I/O without blocking the application. Both RX and TX are buffered through
 * FIFOs, allowing efficient producer-consumer patterns.
 *
 * Key features:
 * - Interrupt-driven TX and RX using FIFOs
 * - Hardware flow control (RTS/CTS) support
 * - Task-based notification on data availability
 * - Configurable baud rate, data bits, parity, and stop bits
 * - Compatible with stdio redirection
 *
 * Typical usage pattern:
 * @code
 * struct pr_uart uart;
 * struct pr_fifo rx_fifo, tx_fifo;
 * struct pr_task_queue queue;
 *
 * int main(void) {
 *     pr_task_queue_init(&queue);
 *     pr_fifo_init(&rx_fifo, 256);
 *     pr_fifo_init(&tx_fifo, 256);
 *
 *     // Initialize UART on GPIO 0 (TX) and 1 (RX), no flow control
 *     pr_uart_init(&uart, &queue, 1, 0, -1, -1);
 *
 *     // Configure for 115200 baud, 8N1
 *     pr_uart_config_set(&uart, 115200, 8, UART_PARITY_NONE, 1);
 *
 *     // Attach FIFOs
 *     pr_uart_attach(&uart, &rx_fifo, &tx_fifo);
 *
 *     // Now write to tx_fifo to send data, read from rx_fifo to receive
 *     pr_fifo_write(&tx_fifo, (uint8_t*)"Hello\n", 6);
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_UART_H
#define PR_UART_H

#include <pico/stdlib.h>
#include <pr/fifo.h>
#include <pr/task.h>
#include <hardware/uart.h>

/**
 * @brief UART driver structure
 *
 * Manages UART communication with FIFO buffering and task integration.
 * All fields are internal and should not be accessed directly.
 */
struct pr_uart
{
    struct uart_inst *instance;     /**< Pico SDK UART instance */
    struct pr_fifo *to_port;        /**< FIFO for transmit data */
    struct pr_fifo *from_port;      /**< FIFO for received data */
    struct pr_task to_port_task;    /**< Task for TX interrupt handler */
    struct pr_task from_port_task;  /**< Task for RX interrupt handler */
    uint8_t rxbuf, txbuf;           /**< Single-byte buffers for IRQ */
    uint8_t rxbuf_free, txbuf_busy; /**< Buffer state flags */
    bool has_cts, has_rts;          /**< Flow control pin availability */
};

/**
 * @brief Helper macro for composing UART structures
 *
 * Generates functions to retrieve the parent pr_uart structure from
 * embedded task members.
 */
PR_TASK_STRUCT_COMPOSE(pr_uart, from_port_task);
PR_TASK_STRUCT_COMPOSE(pr_uart, to_port_task);

/**
 * @brief Initialize a UART driver
 *
 * Configures GPIO pins for UART operation and sets up interrupt handlers.
 * The UART is configured but not yet attached to FIFOs. Call pr_uart_attach()
 * to connect FIFOs for data transfer.
 *
 * @param uart Pointer to UART driver structure
 * @param queue Task queue for interrupt handler tasks
 * @param rx GPIO pin number for RX, or -1 if not used
 * @param tx GPIO pin number for TX, or -1 if not used
 * @param cts GPIO pin number for CTS (hardware flow control), or -1 if not used
 * @param rts GPIO pin number for RTS (hardware flow control), or -1 if not used
 *
 * @note This function enables UART interrupts but does not configure baud rate
 *       or data format. Call pr_uart_config_set() to configure these.
 */
void pr_uart_init(struct pr_uart *uart,
                  struct pr_task_queue *queue,
                  uint8_t rx,
                  uint8_t tx,
                  uint8_t cts,
                  uint8_t rts);

/**
 * @brief Configure UART communication parameters
 *
 * Sets the baud rate, data bits, parity, and stop bits for the UART.
 * Can be called at any time to reconfigure the UART.
 *
 * @param uart Pointer to UART driver structure
 * @param baudrate Baud rate in bits per second (e.g., 115200)
 * @param bits Number of data bits (5, 6, 7, or 8)
 * @param parity Parity mode: UART_PARITY_NONE, UART_PARITY_EVEN, or UART_PARITY_ODD
 * @param stops Number of stop bits (1 or 2)
 *
 * @note Changing configuration while data is being transmitted may result
 *       in corrupted data on the wire
 */
void pr_uart_config_set(struct pr_uart *uart,
                        uint32_t baudrate,
                        uint8_t bits,
                        uart_parity_t parity,
                        uint8_t stops);

/**
 * @brief Enable or disable hardware flow control
 *
 * Enables or disables RTS/CTS hardware flow control. The CTS and RTS pins
 * must have been specified during pr_uart_init() for this to have any effect.
 *
 * @param uart Pointer to UART driver structure
 * @param ctsrts_enable true to enable flow control, false to disable
 *
 * @note Flow control pins must be configured during initialization
 */
void pr_uart_flow_control_set(struct pr_uart *uart,
                              bool ctsrts_enable);

/**
 * @brief Attach FIFOs to the UART
 *
 * Connects FIFOs for transmit and receive data. After calling this function,
 * writing to to_uart FIFO will transmit data, and received data will appear
 * in from_uart FIFO.
 *
 * @param uart Pointer to UART driver structure
 * @param from_uart FIFO to receive data from UART (RX), or NULL
 * @param to_uart FIFO to transmit data to UART (TX), or NULL
 *
 * @note Either FIFO can be NULL to disable that direction
 * @note FIFOs should be initialized before calling this function
 */
void pr_uart_attach(struct pr_uart *uart,
                    struct pr_fifo *from_uart,
                    struct pr_fifo *to_uart);

/** @} */ // end of pr_uart group

#endif
