#ifndef PR_STDIO_FIFO_H_
#define PR_STDIO_FIFO_H_

#include <pr/fifo.h>

void pr_stdio_fifo_driver_bind(struct pr_fifo *fifo_stdout, struct pr_fifo *fifo_stdin);

#endif
