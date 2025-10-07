#ifndef PR_MODBUS_H_
#define PR_MODBUS_H_

#include <stdbool.h>
#include <stdint.h>
#include <pr/task.h>
#include <pr/fifo.h>

enum modbus_error {
    MODBUS_OK = 0,
    MODBUS_ILLEGAL_FUNCTION = 1,
    MODBUS_ILLEGAL_DATA_ADDRESS = 2,
    MODBUS_ILLEGAL_DATA_VALUE = 3,
    MODBUS_SERVER_DEVICE_FAILURE = 4,
};

enum modbus_devid_obj
{
    MODBUS_DEVID_OBJ_VENDOR_NAME,
    MODBUS_DEVID_OBJ_PRODUCT_CODE,
    MODBUS_DEVID_OBJ_MAJOR_MINOR_REVISION,
    MODBUS_DEVID_OBJ_VENDOR_URL,
    MODBUS_DEVID_OBJ_PRODUCT_NAME,
    MODBUS_DEVID_OBJ_MODEL_NAME,
    MODBUS_DEVID_OBJ_USER_APPLICATION_NAME,
};

struct pr_modbus;

typedef enum modbus_error (*modbus_bool_getter_func_t)(struct pr_modbus *bus, uint16_t address, bool *value);
typedef enum modbus_error (*modbus_bool_setter_func_t)(struct pr_modbus *bus, uint16_t address, bool value);
typedef enum modbus_error (*modbus_reg_getter_func_t)(struct pr_modbus *bus, uint16_t address, uint16_t *value);
typedef enum modbus_error (*modbus_reg_setter_func_t)(struct pr_modbus *bus, uint16_t address, uint16_t value);
typedef const char *(*modbus_devid_func_t)(struct pr_modbus *bus, enum modbus_devid_obj obj);

struct pr_modbus_handler
{
    modbus_bool_getter_func_t coil_get, discrete_input_get;
    modbus_bool_setter_func_t coil_set;
    modbus_reg_getter_func_t holding_register_get, input_register_get;
    modbus_reg_setter_func_t holding_register_set;
    modbus_devid_func_t devid;
#if 0
    enum modbus_error (*file_record_read)(struct pr_modbus *bus, uint16_t address, );
    enum modbus_error (*file_record_write)(struct pr_modbus *bus, uint16_t address, );
#endif
};

struct pr_modbus
{
    struct pr_fifo *rx, *tx;
    struct pr_task worker, clear;
    uint8_t addr;
    const struct pr_modbus_handler *handler;
    uint8_t buffer[256];
    uint16_t ptr, rsp_size;
};

PR_TASK_STRUCT_COMPOSE(pr_modbus, worker);
PR_TASK_STRUCT_COMPOSE(pr_modbus, clear);

const char* modbus_strerror(enum modbus_error error);

void pr_modbus_init(struct pr_modbus *bus,
                    const struct pr_modbus_handler *handler,
                    uint8_t addr,
                    struct pr_task_queue *queue,
                    struct pr_fifo *tx,
                    struct pr_fifo *rx);

#endif
