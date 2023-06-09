#include <tusb.h>
#include <pr/tiny_usb.h>

PR_TINY_USB_IMPL

#define DESC_STR_MAX 20

#define USBD_VID 0x1234
#define USBD_PID 0x5e01

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)
#define USBD_MAX_POWER_MA 250

enum usbd_intf_e
{
    INTF_NO_SERIAL = 0,
    INTF_NO_SERIAL_DATA,
    INTF_NO_TOTAL
};

#define USBD_SERIAL_0_EP_CMD 0x81
#define USBD_SERIAL_0_EP_OUT 0x01
#define USBD_SERIAL_0_EP_IN 0x82
#define USBD_SERIAL_CMD_MAX_SIZE 8
#define USBD_SERIAL_IN_OUT_MAX_SIZE 64

enum usbd_str_e
{
    STR_MANUF = 0x01,
    STR_PRODUCT,
    STR_CONFIG,
    STR_SERIAL_INTF,
    // Keep serial last
    STR_SERIAL,
};

static const tusb_desc_device_t usbd_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = STR_MANUF,
    .iProduct = STR_PRODUCT,
    .iSerialNumber = STR_SERIAL,
    .bNumConfigurations = 1,
};

static const uint8_t usbd_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, INTF_NO_TOTAL, STR_CONFIG, USBD_DESC_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, USBD_MAX_POWER_MA),

    TUD_CDC_DESCRIPTOR(INTF_NO_SERIAL, STR_SERIAL_INTF, USBD_SERIAL_0_EP_CMD,
                       USBD_SERIAL_CMD_MAX_SIZE, USBD_SERIAL_0_EP_OUT, USBD_SERIAL_0_EP_IN,
                       USBD_SERIAL_IN_OUT_MAX_SIZE),
};

static const char *const usbd_desc_str[] = {
    [STR_MANUF] = "Picore",
    [STR_PRODUCT] = "USB Loopback demo",
    [STR_CONFIG] = "Main",
    [STR_SERIAL_INTF] = "",
};

const uint8_t *tud_descriptor_device_cb(void)
{
    return (const uint8_t *) &usbd_desc_device;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
    return usbd_desc_cfg;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    static uint16_t desc_str[DESC_STR_MAX];
    uint8_t len;

    if (index == 0) {
        desc_str[1] = 0x0409;
        len = 1;
    } else if (index == STR_SERIAL) {
        return pr_usb_serial_number();
    } else {
        const char *str;

        if (index >= sizeof(usbd_desc_str) / sizeof(usbd_desc_str[0]))
            return NULL;

        str = usbd_desc_str[index];

        if (!str)
            return NULL;

        for (len = 0; len < DESC_STR_MAX - 1 && str[len]; ++len)
            desc_str[1 + len] = str[len];
    }

    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * len + 2);

    return desc_str;
}
