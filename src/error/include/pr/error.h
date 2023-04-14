#ifndef PR_ERROR_H_
#define PR_ERROR_H_

#include <stdint.h>

typedef int8_t pr_error_t;

#define PR_OK 0
#define PR_ERR_UNKNOWN _1
#define PR_ERR_MEMORY -2
#define PR_ERR_IO -3
#define PR_ERR_ADDRESS -4
#define PR_ERR_TIMEOUT -5
#define PR_ERR_INVAL -6
#define PR_ERR_BUSY -7

#endif
