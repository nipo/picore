#include <hardware/irq.h>
#include <hardware/clocks.h>
#include <hardware/structs/sio.h>
#include <pico/stdlib.h>
#include <string.h>
#include <tusb.h>
#include <pr/fifo.h>
#include <pr/task.h>
#include <pr/stdio_fifo.h>
#include <pr/tiny_usb.h>
#include <pr/cdc_interface.h>
#include <pr/modbus.h>

struct app_context
{
    struct pr_task_queue queue;
    struct pr_modbus usb_modbus;
    struct pr_tiny_usb usb;
    struct pr_cdc_interface cdc;
    struct pr_fifo cdc_rx, cdc_tx;
    struct pr_task cdc_event;
    struct pr_cdc_interface cdc_modbus;
    struct pr_fifo modbus_rx, modbus_tx;
    struct pr_task modbus_event;
    uint16_t coils;
};

PR_TASK_STRUCT_COMPOSE(app_context, cdc_event);
PR_TASK_STRUCT_COMPOSE(app_context, modbus_event);
PR_STRUCT_COMPOSE(app_context, pr_modbus, usb_modbus);

enum coil_index
{
    COIL_A,
    COIL_B,
};

enum register_index
{
    REG_COIL_STATE,
};

static
enum modbus_error usb_coil_get(
    struct pr_modbus *modbus, uint16_t addr, bool *value)
{
    struct app_context *app = app_context_from_usb_modbus(modbus);

    printf("Modbus coil get %d\n", addr);

    switch (addr) {
    case COIL_A:
        *value = !!(app->coils & 1);
        return 0;
    case COIL_B:
        *value = !!(app->coils & 2);
        return 0;
    default:
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    }
}

static
enum modbus_error usb_coil_set(
    struct pr_modbus *modbus, uint16_t addr, bool value)
{
    struct app_context *app = app_context_from_usb_modbus(modbus);

    printf("Modbus coil set %d %d\n", addr, value);

    switch (addr) {
    case COIL_A:
        if (value)
            app->coils |= 0x1;
        else
            app->coils &= ~0x1;
        return 0;
    case COIL_B:
        if (value)
            app->coils |= 0x2;
        else
            app->coils &= ~0x2;
        return 0;
    default:
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    }
}

static
enum modbus_error usb_reg_get(
    struct pr_modbus *modbus, uint16_t addr, uint16_t *value)
{
    struct app_context *app = app_context_from_usb_modbus(modbus);

    printf("Modbus reg get %d\n", addr);

    switch (addr) {
    case REG_COIL_STATE:
        *value = app->coils;
        return 0;
    default:
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    }
}

static
enum modbus_error usb_reg_set(
    struct pr_modbus *modbus, uint16_t addr, uint16_t value)
{
    struct app_context *app = app_context_from_usb_modbus(modbus);

    printf("Modbus reg set %d 0x%04x\n", addr, value);

    switch (addr) {
    case REG_COIL_STATE:
        app->coils = value & 3;
        return 0;
    default:
        return MODBUS_ILLEGAL_DATA_ADDRESS;
    }
}

static
const char *usb_devid(
    struct pr_modbus *modbus, enum modbus_devid_obj obj)
{
    struct app_context *app = app_context_from_usb_modbus(modbus);

    switch (obj) {
    case MODBUS_DEVID_OBJ_VENDOR_NAME: return "Picore";
    case MODBUS_DEVID_OBJ_PRODUCT_CODE: return "Modbus responder";
    case MODBUS_DEVID_OBJ_MAJOR_MINOR_REVISION: return "A";
    case MODBUS_DEVID_OBJ_VENDOR_URL: return "https://github.com/nipo/picore";
    case MODBUS_DEVID_OBJ_PRODUCT_NAME: return "Picore demo";
    case MODBUS_DEVID_OBJ_MODEL_NAME: return "A";
    case MODBUS_DEVID_OBJ_USER_APPLICATION_NAME: return "Demo";
    default: return NULL;
    }
}

static const
struct pr_modbus_handler usb_modbus_handler =
{
    .coil_get = usb_coil_get,
    .coil_set = usb_coil_set,
    .input_register_get = usb_reg_get,
    .holding_register_get = usb_reg_get,
    .holding_register_set = usb_reg_set,
    .devid = usb_devid,
};

static
void on_cdc_event(struct pr_task *cdc_event)
{
    struct app_context *app = app_context_from_cdc_event(cdc_event);

    app->cdc.events = 0;
}

static
void on_modbus_event(struct pr_task *modbus_event)
{
    struct app_context *app = app_context_from_modbus_event(modbus_event);

    app->cdc_modbus.events = 0;
}

struct app_context app[1];

int main(void)
{
    memset(app, 0, sizeof(app));

    set_sys_clock_khz(125000, false);

    pr_task_queue_init(&app->queue);

    stdio_init_all();

    pr_tiny_usb_init(&app->usb, &app->queue);
    pr_task_init(&app->cdc_event, &app->queue, on_cdc_event);
    pr_task_init(&app->modbus_event, &app->queue, on_modbus_event);
    pr_fifo_init(&app->cdc_rx, 2048);
    pr_fifo_init(&app->cdc_tx, 2048);
    pr_fifo_init(&app->modbus_rx, 512);
    pr_fifo_init(&app->modbus_tx, 512);
    
    pr_cdc_interface_init(&app->cdc, &app->queue, 0, &app->cdc_tx, &app->cdc_rx, &app->cdc_event);
    pr_cdc_interface_init(&app->cdc_modbus, &app->queue, 1, &app->modbus_tx, &app->modbus_rx, &app->modbus_event);

    pr_stdio_fifo_driver_bind(&app->cdc_tx, &app->cdc_rx);
    
    printf("Modbus demo\n");

    pr_modbus_init(&app->usb_modbus,
                   &usb_modbus_handler,
                   9,
                   &app->queue,
                   &app->modbus_tx,
                   &app->modbus_rx);
    
    for (;;) {
        pr_task_queue_run_until_empty(&app->queue);
        tight_loop_contents();
    }
    
    return 0;

}
