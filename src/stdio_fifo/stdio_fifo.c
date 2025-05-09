#include <pico/stdio.h>
#include <pico/stdio/driver.h>
#include <pr/fifo.h>

struct pr_stdio_driver
{
    struct stdio_driver base;
    struct pr_fifo *fifo_stdout, *fifo_stdin;
};

static struct pr_stdio_driver driver;

static
void pr_stdio_fifo_out_chars(const char *buf, int len)
{
    pr_fifo_write(driver.fifo_stdout, buf, len);
}

static
void pr_stdio_fifo_out_flush(void)
{
}

static
int pr_stdio_fifo_in_chars(char *buf, int len)
{
    return pr_fifo_read(driver.fifo_stdin, buf, len);
}

void pr_stdio_fifo_driver_bind(struct pr_fifo *fifo_stdout, struct pr_fifo *fifo_stdin)
{
    driver.fifo_stdout = fifo_stdout;
    driver.fifo_stdin = fifo_stdin;
    driver.base.out_chars = pr_stdio_fifo_out_chars;
    driver.base.out_flush = pr_stdio_fifo_out_flush;
    driver.base.in_chars = pr_stdio_fifo_in_chars;
    driver.base.next = NULL;
#if PICO_STDIO_ENABLE_CRLF_SUPPORT
    driver.base.last_ended_with_cr = false;
    driver.base.crlf_enabled = false;
#endif
    
    stdio_set_driver_enabled(&driver.base, true);
}

