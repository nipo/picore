#include <string.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <hardware/irq.h>
#include <pr/fifo.h>
#include <pr/task.h>
#include <pr/io.h>
#include <pr/uart.h>

static struct pr_uart *g_uart_inst[2];

static
void on_from_port(struct pr_task *from_port_task)
{
    struct pr_uart *uart = pr_uart_from_from_port_task(from_port_task);

    while (uart->mode != PR_UART_MODE_NONE) {
        if (!uart->rxbuf_free) {
            if (uart->from_port) {
                size_t written = pr_fifo_write(uart->from_port, &uart->rxbuf, 1);

                if (!written)
                    break;
            }

            uart->rxbuf_free = 1;
        } else if (uart_is_readable(uart->instance)) {
            uart->rxbuf = uart_getc(uart->instance);
            uart->rxbuf_free = 0;
        } else {
            break;
        }
    }

    uart_set_irq_enables(uart->instance, uart->rxbuf_free, uart->txbuf_busy);
}

static
void on_to_port(struct pr_task *to_port_task)
{
    struct pr_uart *uart = pr_uart_from_to_port_task(to_port_task);

    while (uart->mode != PR_UART_MODE_NONE) {
        if (!uart->txbuf_busy) {
            if (uart->to_port) {
                size_t read = pr_fifo_read(uart->to_port, &uart->txbuf, 1);

                if (!read)
                    break;

                uart->txbuf_busy = 1;
            }
        } else if (uart_is_writable(uart->instance)) {
            uart_putc_raw(uart->instance, uart->txbuf);
            uart->txbuf_busy = 0;
        } else {
            break;
        }
    }

    uart_set_irq_enables(uart->instance, uart->rxbuf_free, uart->txbuf_busy);
}

static
void on_uart0_io(void)
{
    struct pr_uart *uart = g_uart_inst[0];

    on_uart_io(uart);
}

static
void on_uart1_io(void)
{
    struct pr_uart *uart = g_uart_inst[1];

    on_uart_io(uart);
}

static
void on_uart_io(struct pr_uart *uart)
{
    bool is_w = uart_is_writable(uart->instance);
    bool is_r = uart_is_readable(uart->instance);
    
    if (is_w)
        pr_task_exec(&uart->to_port_task);

    if (is_r)
        pr_task_exec(&uart->from_port_task);

    uart_set_irq_enables(uart->instance, 0, 0);
}

void pr_uart_init(struct pr_uart *uart,
                    struct pr_task_queue *queue)
{
    uart_init(uart->instance, 115200);
    uart_set_irq_enables(uart->instance, 0, 0);
    uart_set_fifo_enabled(uart->instance, 1);
    uart_set_translate_crlf(uart->instance, 0);
    uart_set_hw_flow(uart->instance, 0, 0);

    pr_task_init(&uart->to_port_task, queue, on_to_port);
    pr_task_init(&uart->from_port_task, queue, on_from_port);

    uart->from_port = NULL;
    uart->to_port = NULL;
    uart->rxbuf_free = 1;
    uart->mode = PR_UART_MODE_NONE;
    g_uart_inst = uart;

    irq_add_shared_handler(UART_IRQ, on_uart_io, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    user_irq_claim(UART_IRQ);
    irq_set_enabled(UART_IRQ, 1);
}

static void uart_mode_set(struct pr_uart *uart, enum pr_uart_mode mode)
{
    uart_set_irq_enables(uart->instance, 0, 0);

    switch (uart->mode) {
    case PR_UART_MODE_NONE:
    default:
        break;

    case PR_UART_MODE_MAIN:
        pr_tms_pp_init(0);
        pr_tdo_pp_init(0);
        uart->mode = PR_UART_MODE_NONE;
        break;

    case PR_UART_MODE_MAIN_CTSRTS:
        pr_tck_pp_init(0);
        pr_tms_pp_init(0);
        pr_tdo_pp_init(0);
        pr_tdi_pp_init(0);
        uart->mode = PR_UART_MODE_NONE;
        break;

    case PR_UART_MODE_SIDE:
        pr_tdo_pp_init(0);
        pr_tdi_pp_init(0);
        uart->mode = PR_UART_MODE_NONE;
        break;
    }        

    uart->mode = mode;
    uart->rxbuf_free = 1;
    uart->txbuf_busy = 0;

    switch (uart->mode) {
    case PR_UART_MODE_NONE:
    default:
        break;

    case PR_UART_MODE_MAIN:
        pr_tms_alt_init(GPIO_FUNC_UART, 0); // RX
        pr_tdo_alt_init(GPIO_FUNC_UART, 1, 0); // TX
        break;

    case PR_UART_MODE_MAIN_CTSRTS:
        pr_tck_alt_init(GPIO_FUNC_UART, 0); // CTS
        pr_tms_alt_init(GPIO_FUNC_UART, 0); // RX
        pr_tdo_alt_init(GPIO_FUNC_UART, 1, 0); // TX
        pr_tdi_alt_init(GPIO_FUNC_UART, 1, 0); // RTS
        break;

    case PR_UART_MODE_SIDE:
        pr_tdo_alt_init(GPIO_FUNC_UART, 0, 1); // RX
        pr_tdi_alt_init(GPIO_FUNC_UART, 1, 1); // TX
        break;
    }        

    uart_set_irq_enables(uart->instance, 1, 1);
}

void pr_uart_config_set(struct pr_uart *uart,
                          uint32_t baudrate,
                          uint8_t bits,
                          uart_parity_t parity,
                          uint8_t stops)
{
    uart_set_baudrate(uart->instance, baudrate);
    uart_set_format(uart->instance, bits, stops, parity);
}

void pr_uart_flow_control_set(struct pr_uart *uart,
                              bool ctsrts)
{
    switch (uart->mode) {
    case PR_UART_MODE_MAIN_CTSRTS:
        break;
    default:
        ctsrts = 0;
        break;
    }

    uart_set_hw_flow(uart->instance, ctsrts, ctsrts);
}

void pr_uart_attach(struct pr_uart *uart,
                    struct pr_fifo *to_port,
                    struct pr_fifo *from_port,
                    enum pr_uart_mode mode)
{
    uart->to_port = to_port;
    uart->from_port = from_port;

    pr_fifo_producer_set(uart->from_port, &uart->from_port_task);
    pr_fifo_consumer_set(uart->to_port, &uart->to_port_task);

    uart_mode_set(uart, mode);

    uart_set_irq_enables(uart->instance, 1, 1);
}

void pr_uart_detach(struct pr_uart *uart)
{
    uart_mode_set(uart, PR_UART_MODE_NONE);

    if (uart->from_port)
        pr_fifo_producer_set(uart->from_port, NULL);

    if (uart->to_port)
        pr_fifo_consumer_set(uart->to_port, NULL);

    uart->from_port = NULL;
    uart->to_port = NULL;
}
