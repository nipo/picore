#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pr/modbus.h>
#include <pr/endian.h>
#include "modbus_crc16.h"
#include "modbus_protocol.h"

#define TIMEOUT_MS 50

#define dprintf(...) do{}while(0)
//#define dprintf(...) printf(__VA_ARGS__)

static void modbus_clear(struct pr_modbus *bus);
static void modbus_rx_try(struct pr_modbus *bus);
static void modbus_tx_try(struct pr_modbus *bus);

static size_t modbus_req_frame_len(const uint8_t *data, size_t size)
{
    if (size < 2)
        return 0;

    switch (data[1]) {
    case MODBUS_READ_COILS:
    case MODBUS_READ_DISCRETE_INPUTS:
    case MODBUS_READ_HOLDING_REGISTERS:
    case MODBUS_READ_INPUT_REGISTERS:
    case MODBUS_WRITE_SINGLE_COIL:
    case MODBUS_WRITE_SINGLE_REGISTER:
        return 8;
        
    case MODBUS_WRITE_MULTIPLE_REGISTERS:
    case MODBUS_WRITE_MULTIPLE_COILS: {
        if (size < 7)
            return 0;
        uint16_t byte_count = data[6];
        return byte_count + 9;
    }
        
    case MODBUS_READ_DEVICE_IDENTIFICATION:
        return 7;

    case MODBUS_READ_FILE_RECORD:
    case MODBUS_WRITE_FILE_RECORD:
    case MODBUS_READ_WRITE_REGISTERS:
    default:
        return 0;
    }
}

static size_t modbus_rsp_frame_len(const uint8_t *data, size_t size)
{
    if (size < 2)
        return 0;

    if (data[1] & 0x80) {
        return 5;
    }

    switch (data[1]) {
    case MODBUS_READ_COILS:
    case MODBUS_READ_DISCRETE_INPUTS:
    case MODBUS_READ_HOLDING_REGISTERS:
    case MODBUS_READ_INPUT_REGISTERS: {
        if (size < 5)
            return 0;
        uint16_t byte_count = data[2];
        return byte_count + 5;
    }

    case MODBUS_WRITE_SINGLE_COIL:
    case MODBUS_WRITE_SINGLE_REGISTER:
    case MODBUS_WRITE_MULTIPLE_REGISTERS:
    case MODBUS_WRITE_MULTIPLE_COILS:
        return 8;
        
    case MODBUS_READ_DEVICE_IDENTIFICATION: {
        if (size < 7)
            return 0;
        size_t point = 8;
        uint8_t object_count = data[7];

        for (uint8_t index = 0; index < object_count; ++index) {
            if (size < point + 1)
                return 0;
            point += data[point+1] + 2;
        }
        return point;
    }
        
    case MODBUS_READ_FILE_RECORD:
    case MODBUS_WRITE_FILE_RECORD:
    case MODBUS_READ_WRITE_REGISTERS:
    default:
        return 0;
    }
}

static
enum modbus_error bus_req_coils_get_many(struct pr_modbus *bus,
                                           uint16_t addr, uint16_t count,
                                           modbus_bool_getter_func_t getter)
{
    size_t rsp_bytes = (count + 7) / 8;
    if (rsp_bytes > 250)
        return MODBUS_SERVER_DEVICE_FAILURE;

    if (!getter)
        return MODBUS_ILLEGAL_FUNCTION;

    bus->buffer[bus->ptr++] = rsp_bytes;

    memset(&bus->buffer[bus->ptr], 0, rsp_bytes);
    
    for (uint16_t off = 0; off < count; ++off) {
        bool value;
        enum modbus_error err = getter(bus, addr + off, &value);
        if (err)
            return err;

        if (value)
            bus->buffer[bus->ptr + off/8] |= 1 << (off % 8);
    }

    bus->ptr += rsp_bytes;

    return 0;
}

static
enum modbus_error bus_req_regs_get_many(struct pr_modbus *bus,
                                        uint16_t addr, uint16_t count,
                                        modbus_reg_getter_func_t getter)
{
    size_t rsp_bytes = count * 2;
    if (rsp_bytes > 250)
        return MODBUS_SERVER_DEVICE_FAILURE;

    if (!getter)
        return MODBUS_ILLEGAL_FUNCTION;

    bus->buffer[bus->ptr++] = rsp_bytes;
        
    for (uint16_t off = 0; off < count; ++off) {
        uint16_t value;
        enum modbus_error err = getter(bus, addr + off, &value);
        if (err)
            return err;

        pr_be16_na_write(&bus->buffer[bus->ptr], value);
        bus->ptr += 2;
    }

    return 0;
}

static void bus_req_handle(struct pr_modbus *bus, size_t size)
{
    uint8_t addr = bus->buffer[0];
    uint8_t func = bus->buffer[1];
    enum modbus_error err;

    bus->ptr = 0;
    bus->rsp_size = 0;

    dprintf("q %d,%d:", func, size);
    for (size_t i = 0; i < size; ++i)
        dprintf("%02x", bus->buffer[i]);
    dprintf("\n");

    if (addr != bus->addr)
        return;

    bus->buffer[bus->ptr++] = bus->addr;
    bus->buffer[bus->ptr++] = func;
    
    switch (func) {
    case MODBUS_READ_COILS: {
        uint16_t address = pr_be16_na_read(bus->buffer + 2);
        uint16_t count = pr_be16_na_read(bus->buffer + 4);
        err = bus_req_coils_get_many(bus, address, count, bus->handler->coil_get);
        if (err)
            goto error;
        goto respond;
    }

    case MODBUS_READ_DISCRETE_INPUTS: {
        uint16_t address = pr_be16_na_read(bus->buffer + 2);
        uint16_t count = pr_be16_na_read(bus->buffer + 4);
        err = bus_req_coils_get_many(bus, address, count, bus->handler->discrete_input_get);
        if (err)
            goto error;
        goto respond;
    }

    case MODBUS_READ_HOLDING_REGISTERS: {
        uint16_t address = pr_be16_na_read(bus->buffer + 2);
        uint16_t count = pr_be16_na_read(bus->buffer + 4);
        err = bus_req_regs_get_many(bus, address, count, bus->handler->holding_register_get);
        if (err)
            goto error;
        goto respond;
    }

    case MODBUS_READ_INPUT_REGISTERS: {
        uint16_t address = pr_be16_na_read(bus->buffer + 2);
        uint16_t count = pr_be16_na_read(bus->buffer + 4);
        err = bus_req_regs_get_many(bus, address, count, bus->handler->input_register_get);
        if (err)
            goto error;
        goto respond;
    }

    case MODBUS_WRITE_SINGLE_REGISTER: {
        uint16_t address = pr_be16_na_read(bus->buffer + 2);
        uint16_t value = pr_be16_na_read(bus->buffer + 4);
        if (bus->handler->holding_register_set)
            err = bus->handler->holding_register_set(bus, address, value);
        else
            err = MODBUS_ILLEGAL_FUNCTION;
        if (err)
            goto error;

        pr_be16_na_write(&bus->buffer[bus->ptr], address);
        pr_be16_na_write(&bus->buffer[bus->ptr + 2], value);
        bus->ptr += 4;

        goto respond;
    }
        
    case MODBUS_WRITE_SINGLE_COIL: {
        uint16_t address = pr_be16_na_read(bus->buffer + 2);
        uint16_t rvalue = pr_be16_na_read(bus->buffer + 4);
        bool value = !!rvalue;

        if (bus->handler->coil_set)
            err = bus->handler->coil_set(bus, address, value);
        else
            err = MODBUS_ILLEGAL_FUNCTION;

        dprintf("write single coil %d: %04x: %d\n", address, rvalue, err);

        if (err)
            goto error;

        pr_be16_na_write(&bus->buffer[bus->ptr], address);
        pr_be16_na_write(&bus->buffer[bus->ptr + 2], value ? 0xff00 : 0);
        bus->ptr += 4;

        goto respond;
    }

    case MODBUS_WRITE_MULTIPLE_REGISTERS: {
        uint16_t address = pr_be16_na_read(bus->buffer + 2);
        uint16_t count = pr_be16_na_read(bus->buffer + 4);
        uint16_t byte_count = bus->buffer[6];

        if (count != byte_count / 2) {
            err = MODBUS_ILLEGAL_DATA_VALUE;
            goto error;
        }

        if (byte_count + 9 != size) {
            err = MODBUS_ILLEGAL_DATA_VALUE;
            goto error;
        }
        
        for (uint16_t off = 0; off < count; ++off) {
            uint16_t value = pr_be16_na_read(bus->buffer + 7 + off * 2);
            if (bus->handler->holding_register_set)
                err = bus->handler->holding_register_set(bus, address + off, value);
            else
                err = MODBUS_ILLEGAL_FUNCTION;
            if (err)
                goto error;

        }

        pr_be16_na_write(&bus->buffer[bus->ptr], address);
        pr_be16_na_write(&bus->buffer[bus->ptr + 2], count);
        bus->ptr += 4;
        goto respond;
    }

    case MODBUS_WRITE_MULTIPLE_COILS: {
        uint16_t address = pr_be16_na_read(bus->buffer + 2);
        uint16_t count = pr_be16_na_read(bus->buffer + 4);
        uint16_t byte_count = bus->buffer[6];

        if ((count + 7) / 8 != byte_count) {
            err = MODBUS_ILLEGAL_DATA_VALUE;
            goto error;
        }

        if (byte_count + 9 != size) {
            err = MODBUS_ILLEGAL_DATA_VALUE;
            goto error;
        }

        for (uint16_t off = 0; off < count; ++off) {
            bool value = (bus->buffer[7 + off / 8] >> (off % 8)) & 1;
            if (bus->handler->coil_set)
                err = bus->handler->coil_set(bus, address + off, value);
            else
                err = MODBUS_ILLEGAL_FUNCTION;
            if (err)
                goto error;
        }

        pr_be16_na_write(&bus->buffer[bus->ptr], address);
        pr_be16_na_write(&bus->buffer[bus->ptr + 2], count);
        bus->ptr += 4;
        goto respond;
    }
        
    case MODBUS_READ_DEVICE_IDENTIFICATION: {
        uint8_t mei = bus->buffer[2];
        uint8_t code = bus->buffer[3];
        uint8_t object_id = bus->buffer[4];
        uint8_t last_object_id = 0;

        dprintf("Read DID mei %d code %d obj %d\n",
               mei, code, object_id);
        
        if (mei != 0xe) {
            err = MODBUS_ILLEGAL_DATA_VALUE;
            goto error;
        }

        switch (code) {
        case 1: last_object_id = 0x02; break;
        case 2: last_object_id = 0x7f; break;
        case 3: last_object_id = 0xff; break;
        default:
            err = MODBUS_ILLEGAL_DATA_VALUE;
            goto error;
        }

        bus->buffer[2] = mei;
        bus->buffer[3] = code;
        bus->buffer[4] = 0x83;
        bus->buffer[5] = 0; // More
        bus->buffer[6] = 0; // Next ID
        bus->buffer[7] = 0; // Count

        bus->ptr = 8;
        for (uint8_t i = object_id; i <= last_object_id; ++i) {
            size_t left = sizeof(bus->buffer) - 4 - bus->ptr;
            const char *data = NULL;
            if (bus->handler->devid)
                data = bus->handler->devid(bus, i);

            bus->buffer[6] = i+1;

            if (!data)
                continue;

            size_t len = strlen(data);

            if (len > 0x40)
                len = 0x40;
                
            if (left < len) {
                bus->buffer[6] = i;
                break;
            }

            bus->buffer[bus->ptr] = i;
            bus->buffer[bus->ptr+1] = len;
            memcpy(bus->buffer + bus->ptr + 2, data, len);

            bus->ptr += len + 2;
            bus->buffer[7] += 1;
        }

        if (bus->buffer[6] >= last_object_id) {
            bus->buffer[5] = 0;
            bus->buffer[6] = 0;
        } else {
            bus->buffer[5] = 0xff;
        }
        goto respond;
    }

    case MODBUS_READ_FILE_RECORD:
    case MODBUS_WRITE_FILE_RECORD:
    case MODBUS_READ_WRITE_REGISTERS:
    default:
        err = MODBUS_ILLEGAL_FUNCTION;
        goto error;
    }

    dprintf("req %d -> none\n", func);

    bus->ptr = 0;
    bus->rsp_size = 0;
    return;

  error:

    dprintf("req %d -> err %d\n", func, err);

    bus->buffer[1] |= 0x80;
    bus->buffer[2] = err;
    bus->ptr = 3;

  respond:;
    modbus_crc16_state_t st = modbus_crc16_update(modbus_crc16_init, bus->buffer, bus->ptr);
    modbus_crc16_serialize(bus->buffer + bus->ptr, modbus_crc16_finalize(st));
    bus->rsp_size = bus->ptr + 2;
    bus->ptr = 0;
    
    dprintf("r %d;%d:", func, bus->rsp_size);
    for (size_t i = 0; i < bus->rsp_size; ++i)
        dprintf("%02x", bus->buffer[i]);
    dprintf("\n");
    
    modbus_tx_try(bus);
}

static void bus_rsp_handle(struct pr_modbus *bus, size_t size)
{
    bus->ptr = 0;
    bus->rsp_size = 0;
}

// Returns whether something useful was processed
static bool modbus_req_try(struct pr_modbus *bus)
{
    size_t frame_len = modbus_req_frame_len(bus->buffer, bus->ptr);
    if (!frame_len)
        return false;

    dprintf("req try: %d/%d\n", bus->ptr, frame_len);
    
    if (frame_len > sizeof(bus->buffer)) {
        bus->ptr = 0;
        return true;
    }

    if (frame_len > bus->ptr)
        return false;

    modbus_crc16_state_t st = modbus_crc16_update(modbus_crc16_init, bus->buffer, frame_len);
    dprintf("crc %04x\n", st);
    if (st != modbus_crc16_check_state)
        return false;
    
    bus_req_handle(bus, frame_len);
    return true;
}

// Returns whether something useful was processed
static bool modbus_rsp_try(struct pr_modbus *bus)
{
    size_t frame_len = modbus_rsp_frame_len(bus->buffer, bus->ptr);
    if (!frame_len)
        return false;

    dprintf("rsp try: %d/%d\n", bus->ptr, frame_len);

    if (frame_len > sizeof(bus->buffer)) {
        bus->ptr = 0;
        return true;
    }

    if (frame_len > bus->ptr)
        return false;

    modbus_crc16_state_t st = modbus_crc16_update(modbus_crc16_init, bus->buffer, frame_len);
    if (st != modbus_crc16_check_state)
        return false;
    
    bus_rsp_handle(bus, frame_len);
    return true;
}

static void modbus_rx_try(struct pr_modbus *bus)
{
    size_t rx_len = pr_fifo_read(bus->rx, bus->buffer + bus->ptr, sizeof(bus->buffer) - bus->ptr);

    dprintf("mb rx at %d rx %d\n", bus->ptr, rx_len);
    
    bus->ptr += rx_len;

    if (modbus_req_try(bus) || modbus_rsp_try(bus))
        return;

    if (bus->ptr)
        pr_task_exec_in_ms(&bus->clear, TIMEOUT_MS);
}

static void modbus_tx_try(struct pr_modbus *bus)
{
    size_t tx_len = pr_fifo_write(bus->tx, bus->buffer + bus->ptr, bus->rsp_size - bus->ptr);
    bus->ptr += tx_len;

    if (bus->ptr == bus->rsp_size) {
        bus->rsp_size = 0;
        bus->ptr = 0;

        if (!pr_fifo_is_empty(bus->rx))
            pr_task_exec(&bus->worker);
    }
}

static void modbus_clear(struct pr_modbus *bus)
{
    dprintf("clear\n");

    bus->ptr = 0;
    bus->rsp_size = 0;

    if (!pr_fifo_is_empty(bus->rx))
        pr_task_exec(&bus->worker);
}

static void on_clear(struct pr_task *task)
{
    struct pr_modbus *bus = pr_modbus_from_clear(task);

    modbus_clear(bus);
}

static void on_work(struct pr_task *task)
{
    struct pr_modbus *bus = pr_modbus_from_worker(task);

    if (bus->rsp_size)
        modbus_tx_try(bus);
    else
        modbus_rx_try(bus);
}

void pr_modbus_init(struct pr_modbus *bus,
                    const struct pr_modbus_handler *handler,
                    uint8_t addr,
                    struct pr_task_queue *queue,
                    struct pr_fifo *tx,
                    struct pr_fifo *rx)
{
    bus->handler = handler;
    bus->rx = rx;
    bus->tx = tx;
    pr_task_init(&bus->worker, queue, on_work);
    pr_task_init(&bus->clear, queue, on_clear);
    pr_fifo_consumer_set(rx, &bus->worker);
    pr_fifo_producer_set(tx, &bus->worker);
    bus->addr = addr;
    bus->ptr = 0;
    bus->rsp_size = 0;
}
