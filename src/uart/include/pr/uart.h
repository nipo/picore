#ifndef PR_UART_H
#define PR_UART_H

#include <pico/stdlib.h>
#include <pr/fifo.h>
#include <pr/task.h>
#include <hardware/uart.h>

struct pr_uart
{
    struct uart_inst *instance;
    struct pr_fifo *to_port;
    struct pr_fifo *from_port;
    struct pr_task to_port_task;
    struct pr_task from_port_task;
    uint8_t rxbuf, txbuf;
    uint8_t rxbuf_free, txbuf_busy;
};

PR_TASK_STRUCT_COMPOSE(pr_uart, from_port_task);
PR_TASK_STRUCT_COMPOSE(pr_uart, to_port_task);

void pr_uart_init(struct pr_uart *uart,
                  struct pr_task_queue *queue,
                  uint8_t rx,
                  uint8_t tx,
                  uint8_t cts,
                  uint8_t rts);

void pr_uart_config_set(struct pr_uart *uart,
                        uint32_t baudrate,
                        uint8_t bits,
                        uart_parity_t parity,
                        uint8_t stops);

void pr_uart_flow_control_set(struct pr_uart *uart,
                              bool ctsrts_enable);

#endif
