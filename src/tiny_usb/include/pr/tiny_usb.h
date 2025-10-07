#ifndef PR_TINY_USB_H_
#define PR_TINY_USB_H_

#include <stdint.h>
#include <stdlib.h>
#include <pr/task.h>
#include <pico/bootrom.h>
#include <hardware/watchdog.h>

struct pr_tiny_usb
{
    struct pr_task runner;
};

PR_TASK_STRUCT_COMPOSE(pr_tiny_usb, runner)

void pr_tiny_usb_init(struct pr_tiny_usb *tiny_usb,
                      struct pr_task_queue *queue);

uint16_t* pr_usb_serial_number(void);

#define PR_RPI_RESET_INTERFACE_SUBCLASS 0x00
#define PR_RPI_RESET_INTERFACE_PROTOCOL 0x01
#define PR_RPI_RESET_REQUEST_BOOTSEL 0x01
#define PR_RPI_RESET_REQUEST_FLASH 0x02

#define PR_RPI_RESET_DESC_LEN  9
#define PR_RPI_RESET_DESCRIPTOR(_itfnum, _stridx)                       \
    9, TUSB_DESC_INTERFACE, _itfnum, 0, 0,                              \
    TUSB_CLASS_VENDOR_SPECIFIC, PR_RPI_RESET_INTERFACE_SUBCLASS,        \
    PR_RPI_RESET_INTERFACE_PROTOCOL, _stridx

extern uint8_t pr_rpi_resetd_itf_num;

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
                                                                        \

#endif
