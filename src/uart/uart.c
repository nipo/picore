#include <string.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <hardware/irq.h>
#include <pr/fifo.h>
#include <pr/task.h>
#include <pr/uart.h>

static struct pr_uart *g_uart_inst[2];

static inline uint8_t uart_no_for_pin(uint8_t pin)
{
    if (pin >= 30)
        return -1;

    int idx = pin / 4;
    return (idx ^ (idx >> 1)) & 1;
}

static inline uint8_t uart_bit_for_pin(uint8_t pin)
{
    if (pin >= 30)
        return 0;

    int idx = pin / 4;
    return 1 << ((idx ^ (idx >> 1)) & 1);
}

static
void on_from_port(struct pr_task *from_port_task)
{
    struct pr_uart *uart = pr_uart_from_from_port_task(from_port_task);

    while (uart->from_port) {
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

    while (uart->to_port) {
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

void pr_uart_init(struct pr_uart *uart,
                  struct pr_task_queue *queue,
                  uint8_t rx,
                  uint8_t tx,
                  uint8_t cts,
                  uint8_t rts)
{
    int instance_bit = 0;

    instance_bit |= uart_bit_for_pin(rx);
    instance_bit |= uart_bit_for_pin(tx);
    instance_bit |= uart_bit_for_pin(cts);
    instance_bit |= uart_bit_for_pin(rts);

    // Only one selected
    assert(instance_bit);
    assert((instance_bit & (instance_bit - 1)) == 0);

    int instance = instance_bit == 1 ? 0 : 1;

    g_uart_inst[instance] = uart;

    uart->has_cts = false;
    uart->has_rts = false;
    uart->instance = instance ? uart1 : uart0;
    uart_init(uart->instance, 115200);
    uart_set_irq_enables(uart->instance, 0, 0);
    uart_set_fifo_enabled(uart->instance, 1);
    uart_set_translate_crlf(uart->instance, 0);

    if (rx <= 29)
        gpio_set_function(rx, GPIO_FUNC_UART);
    if (tx <= 29)
        gpio_set_function(tx, GPIO_FUNC_UART);
    if (cts <= 29) {
        gpio_set_function(cts, GPIO_FUNC_UART);
        uart->has_cts = true;
    }
    if (rts <= 29) {
        gpio_set_function(rts, GPIO_FUNC_UART);
        uart->has_rts = true;
    }

    uart_set_hw_flow(uart->instance, uart->has_cts, uart->has_rts);

    pr_task_init(&uart->to_port_task, queue, on_to_port);
    pr_task_init(&uart->from_port_task, queue, on_from_port);

    uart->from_port = NULL;
    uart->to_port = NULL;
    uart->rxbuf_free = 1;

    irq_add_shared_handler(UART0_IRQ + instance, instance ? on_uart1_io : on_uart0_io, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    user_irq_claim(UART0_IRQ + instance);
    irq_set_enabled(UART0_IRQ + instance, 1);
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
    uart_set_hw_flow(uart->instance, uart->has_cts && ctsrts, uart->has_rts && ctsrts);
}

void pr_uart_attach(struct pr_uart *uart,
                    struct pr_fifo *from_uart,
                    struct pr_fifo *to_uart)
{
    if (uart->from_port)
        pr_fifo_producer_set(uart->from_port, NULL);

    if (uart->to_port)
        pr_fifo_consumer_set(uart->to_port, NULL);

    uart->to_port = to_uart;
    uart->from_port = from_uart;

    if (uart->from_port)
        pr_fifo_producer_set(uart->from_port, &uart->from_port_task);
    if (uart->to_port)
        pr_fifo_consumer_set(uart->to_port, &uart->to_port_task);

    uart_set_irq_enables(uart->instance, 1, 1);
}
