#include <pico/stdlib.h>
#include <pr/fifo.h>
#include <pr/task.h>
#include <pr/cdc_interface.h>
#include <class/cdc/cdc_device.h>
#include <tusb.h>

static struct pr_cdc_interface *g_intf[2] = {};

static
size_t to_usb_try_send(struct pr_cdc_interface *intf)
{
    size_t usb_free = tud_cdc_n_write_available(intf->index);

    if (!usb_free) {
        pr_task_exec(&intf->handler);
        return 0;
    }

    const uint8_t *tr_data;
    size_t tr_size = pr_fifo_read_prepare(intf->to_usb, usb_free, &tr_data);
    if (tr_size) {
        size_t written_size = tud_cdc_n_write(intf->index, tr_data, tr_size);
        assert(tr_size == written_size);
        pr_fifo_read_done(intf->to_usb, tr_size);
    }

    if (pr_fifo_is_empty(intf->to_usb)) {
        tud_cdc_n_write_flush(intf->index);
    } else {
        pr_task_exec(&intf->handler);
    }        

    return tr_size;
}

static
size_t from_usb_try_recv(struct pr_cdc_interface *intf)
{
    size_t usb_available = tud_cdc_n_available(intf->index);

    if (!usb_available)
        return 0;

    uint8_t *tr_data;
    size_t tr_size = pr_fifo_write_prepare(intf->from_usb, usb_available, &tr_data);
    size_t read_size = tud_cdc_n_read(intf->index, tr_data, tr_size);
    assert(tr_size == read_size);
    pr_fifo_write_done(intf->from_usb, tr_size);

    return tr_size;
}

static
void handler(struct pr_task *task)
{
    struct pr_cdc_interface *intf = pr_cdc_interface_from_handler(task);
    bool up = tud_cdc_n_connected(intf->index);

    if (up) {
        if (!intf->open) {
            intf->open = 1;
            intf->events |= PR_CDC_INTERFACE_EVENT_OPEN;
        }

        if (to_usb_try_send(intf)) {
            intf->events |= PR_CDC_INTERFACE_EVENT_TX;
        }
        
        if (from_usb_try_recv(intf)) {
            intf->events |= PR_CDC_INTERFACE_EVENT_RX;
        }
    } else {
        if (intf->open) {
            intf->open = 0;
            intf->events |= PR_CDC_INTERFACE_EVENT_OPEN;
        }
    }

    if (intf->events)
        pr_task_exec(intf->on_event);
}

void tud_cdc_tx_complete_cb(uint8_t itf)
{
    struct pr_cdc_interface *intf = g_intf[itf];

    if (!intf)
        return;

    pr_task_exec(&intf->handler);
}

void tud_cdc_rx_cb(uint8_t itf)
{
    struct pr_cdc_interface *intf = g_intf[itf];

    if (!intf)
        return;

    pr_task_exec(&intf->handler);
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
{
    struct pr_cdc_interface *intf = g_intf[itf];

    if (!intf)
        return;
    
    intf->lc = *p_line_coding;

    intf->events |= PR_CDC_INTERFACE_EVENT_LC;
    pr_task_exec(intf->on_event);
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    struct pr_cdc_interface *intf = g_intf[itf];

    if (!intf)
        return;
    
    intf->ctsrts = rts;

    intf->events |= PR_CDC_INTERFACE_EVENT_CTSRTS;
    pr_task_exec(intf->on_event);
}

void pr_cdc_interface_init(struct pr_cdc_interface *intf,
                             struct pr_task_queue *queue,
                             int cdc_index,
                             struct pr_fifo *to_usb,
                             struct pr_fifo *from_usb,
                             struct pr_task *on_event)
{
    pr_task_init(&intf->handler, queue, handler);
    intf->on_event = on_event;
    intf->to_usb = to_usb;
    intf->from_usb = from_usb;
    intf->events = 0;
    intf->open = 0;
    intf->index = cdc_index;
    intf->lc.bit_rate = 115200;
    intf->lc.data_bits = 8;
    intf->lc.parity = 0;
    intf->lc.stop_bits = 1;

    pr_fifo_consumer_set(to_usb, &intf->handler);
    pr_fifo_producer_set(from_usb, &intf->handler);

    g_intf[cdc_index] = intf;
    
    pr_task_exec(&intf->handler);
}
