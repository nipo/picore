#ifndef PR_CDC_INTERFACE_H
#define PR_CDC_INTERFACE_H

#include <pico/stdlib.h>
#include <pr/fifo.h>
#include <pr/task.h>
#include <tusb.h>
#include <class/cdc/cdc.h>

#define PR_CDC_INTERFACE_EVENT_RX 1
#define PR_CDC_INTERFACE_EVENT_TX 2
#define PR_CDC_INTERFACE_EVENT_OPEN 4
#define PR_CDC_INTERFACE_EVENT_LC 8
#define PR_CDC_INTERFACE_EVENT_CTSRTS 16

struct pr_cdc_interface {
    cdc_line_coding_t lc;
    bool ctsrts;
    uint8_t open;
    uint8_t index;
    uint8_t events;
    struct pr_fifo *to_usb;
    struct pr_fifo *from_usb;
    struct pr_task *on_event;
    struct pr_task handler;
};

void pr_cdc_interface_init(struct pr_cdc_interface *intf,
                             struct pr_task_queue *queue,
                             int cdc_index,
                             struct pr_fifo *to_usb,
                             struct pr_fifo *from_usb,
                             struct pr_task *on_event);

PR_TASK_STRUCT_COMPOSE(pr_cdc_interface, handler);

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

#endif
