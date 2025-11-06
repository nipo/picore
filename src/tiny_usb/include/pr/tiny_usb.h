/**
 * @file tiny_usb.h
 * @defgroup pr_tiny_usb TinyUSB Task Integration
 * @brief Task-based integration for TinyUSB stack with Raspberry Pi Pico reset interface
 *
 * The tiny_usb library integrates the TinyUSB stack with the cooperative task
 * scheduler. It provides a task that continuously services the USB stack,
 * ensuring USB events are processed without requiring polling in the main loop.
 *
 * Additionally, this module provides the Raspberry Pi Pico vendor-specific reset
 * interface, which allows host software (like picotool) to reset the device into
 * BOOTSEL mode or trigger a flash reset via USB control requests.
 *
 * Key features:
 * - Task-based USB stack servicing
 * - Automatic chip UID-based USB serial number generation
 * - Raspberry Pi Pico reset interface for BOOTSEL and flash reset
 * - Integration with watchdog for reliable resets
 *
 * Typical usage pattern:
 * @code
 * #include <pr/tiny_usb.h>
 * #include <pr/task.h>
 *
 * struct pr_tiny_usb usb;
 * struct pr_task_queue queue;
 *
 * int main(void) {
 *     pr_task_queue_init(&queue);
 *
 *     // Initialize TinyUSB with task integration
 *     pr_tiny_usb_init(&usb, &queue);
 *
 *     // USB stack is now serviced automatically by the task system
 *     for (;;) {
 *         pr_task_queue_run_until_empty(&queue);
 *     }
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_TINY_USB_H_
#define PR_TINY_USB_H_

#include <stdint.h>
#include <stdlib.h>
#include <pr/task.h>
#include <pico/bootrom.h>
#include <hardware/watchdog.h>

/**
 * @brief TinyUSB task integration structure
 *
 * Contains the task that continuously services the TinyUSB stack.
 */
struct pr_tiny_usb
{
    struct pr_task runner; /**< Task for running tud_task() */
};

/**
 * @brief Helper macro for composing TinyUSB structures
 *
 * Generates functions to retrieve the parent pr_tiny_usb structure from
 * the embedded task member.
 */
PR_TASK_STRUCT_COMPOSE(pr_tiny_usb, runner)

/**
 * @brief Initialize TinyUSB with task integration
 *
 * Initializes the TinyUSB device stack and creates a task that continuously
 * services USB events. The task automatically reschedules itself, ensuring
 * the USB stack is serviced without requiring explicit polling.
 *
 * @param tiny_usb Pointer to TinyUSB integration structure
 * @param queue Task queue for USB servicing task
 *
 * @note This function calls tusb_init() and schedules the USB task
 * @note The USB task runs continuously until the program terminates
 */
void pr_tiny_usb_init(struct pr_tiny_usb *tiny_usb,
                      struct pr_task_queue *queue);

/**
 * @brief Generate USB serial number from chip UID
 *
 * Returns a pointer to a UTF-16 encoded string containing the device serial
 * number derived from the RP2040 chip unique ID. This is used by TinyUSB
 * for the USB device serial number descriptor.
 *
 * @return Pointer to UTF-16 encoded serial number string
 *
 * @note The returned pointer points to static storage
 * @note The serial number is generated from pr_chip_uid32()
 */
uint16_t* pr_usb_serial_number(void);

/**
 * @brief Raspberry Pi Pico reset interface subclass
 *
 * USB interface subclass identifier for the Raspberry Pi Pico reset interface.
 */
#define PR_RPI_RESET_INTERFACE_SUBCLASS 0x00

/**
 * @brief Raspberry Pi Pico reset interface protocol
 *
 * USB interface protocol identifier for the Raspberry Pi Pico reset interface.
 */
#define PR_RPI_RESET_INTERFACE_PROTOCOL 0x01

/**
 * @brief USB control request to reset into BOOTSEL mode
 *
 * When this request is received, the device will reset into USB BOOTSEL mode,
 * allowing firmware updates via USB mass storage.
 */
#define PR_RPI_RESET_REQUEST_BOOTSEL 0x01

/**
 * @brief USB control request to reset into flash mode
 *
 * When this request is received, the device will perform a watchdog reset,
 * rebooting the device and running the application from flash.
 */
#define PR_RPI_RESET_REQUEST_FLASH 0x02

/**
 * @brief Length of the reset interface descriptor
 */
#define PR_RPI_RESET_DESC_LEN  9

/**
 * @brief USB descriptor macro for Raspberry Pi Pico reset interface
 *
 * Use this macro in your USB configuration descriptor to add the reset
 * interface, which allows picotool and other utilities to reset the device.
 *
 * @param _itfnum Interface number for this interface
 * @param _stridx String descriptor index for interface name
 */
#define PR_RPI_RESET_DESCRIPTOR(_itfnum, _stridx)                       \
    9, TUSB_DESC_INTERFACE, _itfnum, 0, 0,                              \
    TUSB_CLASS_VENDOR_SPECIFIC, PR_RPI_RESET_INTERFACE_SUBCLASS,        \
    PR_RPI_RESET_INTERFACE_PROTOCOL, _stridx

/**
 * @brief Interface number assigned to reset interface
 *
 * This variable is set by the TinyUSB driver during interface enumeration.
 */
extern uint8_t pr_rpi_resetd_itf_num;

/**
 * @brief Implementation macro for TinyUSB integration
 *
 * This macro provides the complete implementation of the TinyUSB task integration
 * and Raspberry Pi reset interface driver. Include this macro once in your
 * application's source file (typically in the file that defines your USB
 * descriptors and implements TinyUSB callbacks).
 *
 * The macro expands to:
 * - pr_tiny_usb_init() implementation
 * - TinyUSB reset interface driver callbacks
 * - USB class driver structure
 *
 * Usage:
 * @code
 * #include <pr/tiny_usb.h>
 *
 * // In your .c file, after includes:
 * PR_TINY_USB_IMPL
 *
 * // Then define your USB descriptors and other TinyUSB callbacks
 * @endcode
 *
 * @note This macro must be used exactly once in your application
 * @note The reset interface driver will be registered with TinyUSB
 */
#define PR_TINY_USB_IMPL                                                \
    static                                                              \
    void tiny_usb_runner(struct pr_task *runner)                        \
    {                                                                   \
        struct pr_tiny_usb *context = pr_tiny_usb_from_runner(runner);  \
                                                                        \
        tud_task();                                                     \
                                                                        \
        pr_task_exec(&context->runner);                                 \
    }                                                                   \
                                                                        \
    void pr_tiny_usb_init(struct pr_tiny_usb *context,                  \
                          struct pr_task_queue *queue)                  \
    {                                                                   \
        tusb_init();                                                    \
                                                                        \
        pr_task_init(&context->runner, queue, tiny_usb_runner);         \
        pr_task_exec(&context->runner);                                 \
    }                                                                   \
                                                                        \
    void pr_rpi_resetd_init(void)                                       \
    {                                                                   \
    }                                                                   \
                                                                        \
    void pr_rpi_resetd_reset(uint8_t __unused rhport)                   \
    {                                                                   \
        pr_rpi_resetd_itf_num = 0;                                      \
    }                                                                   \
                                                                        \
    uint16_t pr_rpi_resetd_open(uint8_t __unused rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len)\
    {                                                                   \
        TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass && \
                  PR_RPI_RESET_INTERFACE_SUBCLASS == itf_desc->bInterfaceSubClass && \
                  PR_RPI_RESET_INTERFACE_PROTOCOL == itf_desc->bInterfaceProtocol, 0); \
                                                                        \
        uint16_t const drv_len = sizeof(tusb_desc_interface_t);         \
        TU_VERIFY(max_len >= drv_len, 0);                               \
                                                                        \
        pr_rpi_resetd_itf_num = itf_desc->bInterfaceNumber;             \
        return drv_len;                                                 \
    }                                                                   \
                                                                        \
    bool pr_rpi_resetd_control_xfer_cb(uint8_t __unused rhport, uint8_t stage, tusb_control_request_t const * request)\
    {                                                                   \
        if (stage != CONTROL_STAGE_SETUP) return true;                  \
                                                                        \
        if (request->wIndex == pr_rpi_resetd_itf_num) {                 \
                                                                        \
            if (request->bRequest == PR_RPI_RESET_REQUEST_BOOTSEL) {    \
                reset_usb_boot(0, 1);                                   \
            }                                                           \
                                                                        \
            if (request->bRequest == PR_RPI_RESET_REQUEST_FLASH) {      \
                watchdog_reboot(0, 0, 100);                             \
                return true;                                            \
            }                                                           \
        }                                                               \
        return false;                                                   \
    }                                                                   \
                                                                        \
    bool pr_rpi_resetd_xfer_cb(uint8_t __unused rhport, uint8_t __unused ep_addr, xfer_result_t __unused result, uint32_t __unused xferred_bytes)\
    {                                                                   \
        return true;                                                    \
    }                                                                   \
                                                                        \
    const usbd_class_driver_t pr_rpi_resetd_driver =                    \
    {                                                                   \
        .init             = pr_rpi_resetd_init,                         \
        .reset            = pr_rpi_resetd_reset,                        \
        .open             = pr_rpi_resetd_open,                         \
        .control_xfer_cb  = pr_rpi_resetd_control_xfer_cb,              \
        .xfer_cb          = pr_rpi_resetd_xfer_cb,                      \
        .sof              = NULL                                        \
    };                                                                  \

/** @} */ // end of pr_tiny_usb group

#endif
