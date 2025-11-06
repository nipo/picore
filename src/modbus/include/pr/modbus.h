/**
 * @file modbus.h
 * @defgroup pr_modbus Modbus RTU Protocol
 * @brief Modbus RTU protocol implementation with FIFO integration
 *
 * The modbus library implements the Modbus RTU serial protocol for embedded
 * systems. It provides a server (slave) implementation with callback-based
 * handlers for different register types and supports standard Modbus functions.
 *
 * Supported Modbus functions:
 * - Read Coils (0x01)
 * - Read Discrete Inputs (0x02)
 * - Read Holding Registers (0x03)
 * - Read Input Registers (0x04)
 * - Write Single Coil (0x05)
 * - Write Single Register (0x06)
 * - Read Device Identification (0x2B/0x0E)
 *
 * Key features:
 * - Callback-based register access
 * - Automatic CRC handling
 * - FIFO-based serial communication
 * - Task-based message processing
 * - Device identification support
 *
 * Typical usage pattern:
 * @code
 * #include <pr/modbus.h>
 * #include <pr/fifo.h>
 * #include <pr/task.h>
 *
 * struct pr_modbus modbus;
 * struct pr_fifo rx_fifo, tx_fifo;
 * struct pr_task_queue queue;
 *
 * // Register access callbacks
 * enum modbus_error coil_get(struct pr_modbus *bus, uint16_t addr, bool *val) {
 *     // Read coil state from application
 *     return MODBUS_OK;
 * }
 *
 * enum modbus_error holding_get(struct pr_modbus *bus, uint16_t addr, uint16_t *val) {
 *     // Read holding register from application
 *     return MODBUS_OK;
 * }
 *
 * const struct pr_modbus_handler handler = {
 *     .coil_get = coil_get,
 *     .holding_register_get = holding_get,
 *     // ... other callbacks
 * };
 *
 * int main(void) {
 *     pr_task_queue_init(&queue);
 *     pr_fifo_init(&rx_fifo, 256);
 *     pr_fifo_init(&tx_fifo, 256);
 *
 *     // Initialize Modbus with address 1
 *     pr_modbus_init(&modbus, &handler, 1, &queue, &tx_fifo, &rx_fifo);
 *
 *     // Connect FIFOs to UART or other transport
 * }
 * @endcode
 *
 * @{
 */

#ifndef PR_MODBUS_H_
#define PR_MODBUS_H_

#include <stdbool.h>
#include <stdint.h>
#include <pr/task.h>
#include <pr/fifo.h>

/**
 * @brief Modbus error codes
 *
 * Standard Modbus exception codes plus success indicator.
 */
enum modbus_error {
    MODBUS_OK = 0,                      /**< Success (no error) */
    MODBUS_ILLEGAL_FUNCTION = 1,        /**< Function code not supported */
    MODBUS_ILLEGAL_DATA_ADDRESS = 2,    /**< Invalid register/coil address */
    MODBUS_ILLEGAL_DATA_VALUE = 3,      /**< Invalid data value */
    MODBUS_SERVER_DEVICE_FAILURE = 4,   /**< Server device failure */
};

/**
 * @brief Modbus device identification object IDs
 *
 * Standard Modbus device identification objects.
 */
enum modbus_devid_obj
{
    MODBUS_DEVID_OBJ_VENDOR_NAME,           /**< Vendor name string */
    MODBUS_DEVID_OBJ_PRODUCT_CODE,          /**< Product code string */
    MODBUS_DEVID_OBJ_MAJOR_MINOR_REVISION,  /**< Firmware version string */
    MODBUS_DEVID_OBJ_VENDOR_URL,            /**< Vendor URL string */
    MODBUS_DEVID_OBJ_PRODUCT_NAME,          /**< Product name string */
    MODBUS_DEVID_OBJ_MODEL_NAME,            /**< Model name string */
    MODBUS_DEVID_OBJ_USER_APPLICATION_NAME, /**< Application name string */
};

struct pr_modbus;

/**
 * @brief Callback function type for reading boolean values (coils/discrete inputs)
 */
typedef enum modbus_error (*modbus_bool_getter_func_t)(struct pr_modbus *bus, uint16_t address, bool *value);

/**
 * @brief Callback function type for writing boolean values (coils)
 */
typedef enum modbus_error (*modbus_bool_setter_func_t)(struct pr_modbus *bus, uint16_t address, bool value);

/**
 * @brief Callback function type for reading register values
 */
typedef enum modbus_error (*modbus_reg_getter_func_t)(struct pr_modbus *bus, uint16_t address, uint16_t *value);

/**
 * @brief Callback function type for writing register values
 */
typedef enum modbus_error (*modbus_reg_setter_func_t)(struct pr_modbus *bus, uint16_t address, uint16_t value);

/**
 * @brief Callback function type for device identification
 */
typedef const char *(*modbus_devid_func_t)(struct pr_modbus *bus, enum modbus_devid_obj obj);

/**
 * @brief Modbus handler callback structure
 *
 * Application provides callbacks for accessing different register types.
 * Set callback pointers to NULL for unsupported register types.
 */
struct pr_modbus_handler
{
    modbus_bool_getter_func_t coil_get;              /**< Read coil state */
    modbus_bool_getter_func_t discrete_input_get;    /**< Read discrete input state */
    modbus_bool_setter_func_t coil_set;              /**< Write coil state */
    modbus_reg_getter_func_t holding_register_get;   /**< Read holding register */
    modbus_reg_getter_func_t input_register_get;     /**< Read input register */
    modbus_reg_setter_func_t holding_register_set;   /**< Write holding register */
    modbus_devid_func_t devid;                       /**< Get device identification */
#if 0
    enum modbus_error (*file_record_read)(struct pr_modbus *bus, uint16_t address, );
    enum modbus_error (*file_record_write)(struct pr_modbus *bus, uint16_t address, );
#endif
};

/**
 * @brief Modbus RTU instance structure
 *
 * Manages Modbus protocol state and communication. All fields are internal.
 */
struct pr_modbus
{
    struct pr_fifo *rx;                     /**< Receive FIFO (from transport) */
    struct pr_fifo *tx;                     /**< Transmit FIFO (to transport) */
    struct pr_task worker;                  /**< Worker task for processing messages */
    struct pr_task clear;                   /**< Task for clearing timeouts */
    uint8_t addr;                           /**< Modbus device address (1-247) */
    const struct pr_modbus_handler *handler; /**< Application callbacks */
    uint8_t buffer[256];                    /**< Message buffer */
    uint16_t ptr;                           /**< Current buffer position */
    uint16_t rsp_size;                      /**< Response size */
};

/**
 * @brief Helper macros for composing Modbus structures
 *
 * Generate functions to retrieve the parent pr_modbus structure from
 * embedded task members.
 */
PR_TASK_STRUCT_COMPOSE(pr_modbus, worker);
PR_TASK_STRUCT_COMPOSE(pr_modbus, clear);

/**
 * @brief Convert Modbus error code to string
 *
 * Returns a human-readable string for a Modbus error code.
 *
 * @param error Modbus error code
 * @return Pointer to error string (never NULL)
 */
const char* modbus_strerror(enum modbus_error error);

/**
 * @brief Initialize a Modbus RTU instance
 *
 * Sets up Modbus protocol handling with the specified address and callbacks.
 * The instance will process incoming requests from the RX FIFO and send
 * responses to the TX FIFO.
 *
 * @param bus Pointer to Modbus instance structure
 * @param handler Pointer to application callback handler structure
 * @param addr Modbus device address (1-247, 0 is unsupported)
 * @param queue Task queue for message processing
 * @param tx Transmit FIFO (responses sent here)
 * @param rx Receive FIFO (requests received from here)
 *
 * @note The handler callbacks are called from the worker task context
 * @note Address 0 is for broadcast messages (unsupported)
 * @note The FIFOs should be connected to a UART or other serial transport
 * @note Message processing happens automatically via tasks
 */
void pr_modbus_init(struct pr_modbus *bus,
                    const struct pr_modbus_handler *handler,
                    uint8_t addr,
                    struct pr_task_queue *queue,
                    struct pr_fifo *tx,
                    struct pr_fifo *rx);

/** @} */ // end of pr_modbus group

#endif
