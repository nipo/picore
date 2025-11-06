/**
 * @file cdc_interface.h
 * @defgroup pr_cdc_interface USB CDC Interface
 * @brief USB CDC (Communication Device Class) interface with FIFO integration
 *
 * The cdc_interface library provides a bridge between TinyUSB CDC interfaces
 * and FIFO buffers, enabling USB serial communication with automatic buffering
 * and event notification. CDC interfaces appear as virtual serial ports on the
 * host system.
 *
 * Key features:
 * - FIFO-based buffering for USB CDC TX and RX
 * - Event notification for data, connection, and configuration changes
 * - Line coding (baud rate, data bits, parity) monitoring
 * - Flow control (RTS/CTS) state tracking
 * - Helper functions to extract UART-compatible settings
 *
 * Events that trigger notifications:
 * - RX: Data received from host
 * - TX: Buffer space available for transmission
 * - OPEN: USB CDC connection state changed
 * - LC: Line coding changed by host
 * - CTSRTS: Flow control state changed
 *
 * Typical usage pattern:
 * @code
 * struct pr_cdc_interface cdc;
 * struct pr_fifo rx_fifo, tx_fifo;
 * struct pr_task event_handler;
 * struct pr_task_queue queue;
 *
 * void on_cdc_event(struct pr_task *task) {
 *     // Handle CDC events (line coding changes, connection, etc.)
 * }
 *
 * int main(void) {
 *     pr_task_queue_init(&queue);
 *     pr_fifo_init(&rx_fifo, 256);
 *     pr_fifo_init(&tx_fifo, 256);
 *     pr_task_init(&event_handler, &queue, on_cdc_event);
 *
 *     // Initialize CDC interface 0
 *     pr_cdc_interface_init(&cdc, &queue, 0, &tx_fifo, &rx_fifo, &event_handler);
 *
 *     // Data from host appears in rx_fifo, write to tx_fifo to send to host
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_CDC_INTERFACE_H
#define PR_CDC_INTERFACE_H

#include <pico/stdlib.h>
#include <pr/fifo.h>
#include <pr/task.h>
#include <tusb.h>
#include <class/cdc/cdc.h>

/**
 * @brief Event flag: Data received from host
 */
#define PR_CDC_INTERFACE_EVENT_RX 1

/**
 * @brief Event flag: Buffer space available for transmission
 */
#define PR_CDC_INTERFACE_EVENT_TX 2

/**
 * @brief Event flag: Connection state changed
 */
#define PR_CDC_INTERFACE_EVENT_OPEN 4

/**
 * @brief Event flag: Line coding changed
 */
#define PR_CDC_INTERFACE_EVENT_LC 8

/**
 * @brief Event flag: Flow control state changed
 */
#define PR_CDC_INTERFACE_EVENT_CTSRTS 16

/**
 * @brief USB CDC interface structure
 *
 * Manages a single USB CDC interface with FIFO buffering and event notification.
 */
struct pr_cdc_interface {
    cdc_line_coding_t lc;       /**< Current line coding (baud, parity, etc.) */
    bool ctsrts;                /**< Flow control enabled state */
    uint8_t open;               /**< Connection state (0=closed, 1=open) */
    uint8_t index;              /**< CDC interface index */
    uint8_t events;             /**< Pending event flags (PR_CDC_INTERFACE_EVENT_*) */
    struct pr_fifo *to_usb;     /**< FIFO for data to transmit to host */
    struct pr_fifo *from_usb;   /**< FIFO for data received from host */
    struct pr_task *on_event;   /**< Task to schedule on events */
    struct pr_task handler;     /**< Internal task for CDC handling */
};

/**
 * @brief Initialize a USB CDC interface
 *
 * Sets up a CDC interface with FIFO buffering and event notification. The
 * interface will automatically transfer data between FIFOs and USB, and
 * schedule the event task when significant events occur.
 *
 * @param intf Pointer to CDC interface structure
 * @param queue Task queue for CDC handler task
 * @param cdc_index TinyUSB CDC interface index (0 for first CDC, 1 for second, etc.)
 * @param to_usb FIFO for data to transmit to host, or NULL
 * @param from_usb FIFO for data received from host, or NULL
 * @param on_event Task to schedule on events (connection, line coding, etc.), or NULL
 *
 * @note The on_event task should check intf->events to determine what happened
 * @note Line coding and flow control states are available in intf->lc and intf->ctsrts
 */
void pr_cdc_interface_init(struct pr_cdc_interface *intf,
                             struct pr_task_queue *queue,
                             int cdc_index,
                             struct pr_fifo *to_usb,
                             struct pr_fifo *from_usb,
                             struct pr_task *on_event);

/**
 * @brief Helper macro for composing CDC interface structures
 *
 * Generates functions to retrieve the parent pr_cdc_interface structure from
 * the embedded task member.
 */
PR_TASK_STRUCT_COMPOSE(pr_cdc_interface, handler);

/**
 * @brief Extract data bits from CDC line coding
 *
 * Converts the CDC line coding data bits field to a simple integer
 * suitable for UART configuration.
 *
 * @param lc Pointer to CDC line coding structure
 * @return Number of data bits (5, 6, 7, or 8)
 */
static inline
uint usb_cdc_lc_bits_get(const cdc_line_coding_t *lc)
{
    switch (lc->data_bits) {
    case 5:
        return 5;
    case 6:
        return 6;
    case 7:
        return 7;
    default:
        return 8;
    }
}

/**
 * @brief Extract parity from CDC line coding
 *
 * Converts the CDC line coding parity field to Pico SDK UART parity enum.
 *
 * @param lc Pointer to CDC line coding structure
 * @return UART_PARITY_NONE, UART_PARITY_ODD, or UART_PARITY_EVEN
 */
static inline
uart_parity_t usb_cdc_lc_parity_get(const cdc_line_coding_t *lc)
{
    switch (lc->parity) {
    case 1:
        return UART_PARITY_ODD;
    case 2:
        return UART_PARITY_EVEN;
    default:
        return UART_PARITY_NONE;
    }
}

/**
 * @brief Extract stop bits from CDC line coding
 *
 * Converts the CDC line coding stop bits field to a simple integer
 * suitable for UART configuration.
 *
 * @param lc Pointer to CDC line coding structure
 * @return Number of stop bits (1 or 2)
 */
static inline
uint usb_cdc_lc_stop_get(const cdc_line_coding_t *lc)
{
    switch (lc->stop_bits) {
    case 2:
        return 2;
    default:
        return 1;
    }
}

/** @} */ // end of pr_cdc_interface group

#endif
