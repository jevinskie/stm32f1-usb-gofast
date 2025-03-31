
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/usb/usb_ch9.h>
#include <zephyr/usb/usb_device.h>

LOG_MODULE_REGISTER(usb);

enum usb_req_type {
    GET_SPI_MAGIC  = 0x60,
    SET_SPI_CONFIG = 0x61,
    GET_SPI_CONFIG = 0x62,
};

#define LOOPBACK_OUT_EP_ADDR 0x01
#define LOOPBACK_IN_EP_ADDR  0x81

#define LOOPBACK_OUT_EP_IDX  0
#define LOOPBACK_IN_EP_IDX   1

static uint8_t copi_buf_raw[1024];
static uint8_t cipo_buf_raw[sizeof(copi_buf_raw)];

/* usb.rst config structure start */
struct usb_loopback_config {
    struct usb_if_descriptor if0;
    struct usb_ep_descriptor if0_out_ep;
    struct usb_ep_descriptor if0_in_ep;
} __packed;

USBD_CLASS_DESCR_DEFINE(primary, 0)
struct usb_loopback_config loopback_cfg = {
    /* Interface descriptor 0 */
    .if0 =
        {
            .bLength            = sizeof(struct usb_if_descriptor),
            .bDescriptorType    = USB_DESC_INTERFACE,
            .bInterfaceNumber   = 0,
            .bAlternateSetting  = 0,
            .bNumEndpoints      = 2,
            .bInterfaceClass    = USB_BCC_VENDOR,
            .bInterfaceSubClass = 0,
            .bInterfaceProtocol = 0,
            .iInterface         = 0,
        },

    /* Data Endpoint OUT */
    .if0_out_ep =
        {
            .bLength          = sizeof(struct usb_ep_descriptor),
            .bDescriptorType  = USB_DESC_ENDPOINT,
            .bEndpointAddress = LOOPBACK_OUT_EP_ADDR,
            .bmAttributes     = USB_EP_TYPE_BULK,
            .wMaxPacketSize   = sys_cpu_to_le16(64),
            .bInterval        = 0x00,
        },

    /* Data Endpoint IN */
    .if0_in_ep =
        {
            .bLength          = sizeof(struct usb_ep_descriptor),
            .bDescriptorType  = USB_DESC_ENDPOINT,
            .bEndpointAddress = LOOPBACK_IN_EP_ADDR,
            .bmAttributes     = USB_EP_TYPE_BULK,
            .wMaxPacketSize   = sys_cpu_to_le16(64),
            .bInterval        = 0x00,
        },
};
/* usb.rst config structure end */

static void loopback_out_cb(uint8_t ep, enum usb_dc_ep_cb_status_code ep_status) {
    uint32_t sz;
    usb_read(ep, NULL, 0, &sz);
    LOG_DBG("ep 0x%x, bytes to read %d", ep, sz);
}

static void loopback_in_cb(uint8_t ep, enum usb_dc_ep_cb_status_code ep_status) {
    LOG_DBG("loopback_in_cb ep 0x%x status: %d", ep, ep_status);
}

/* usb.rst endpoint configuration start */
static struct usb_ep_cfg_data ep_cfg[] = {
    {
        .ep_cb   = loopback_out_cb,
        .ep_addr = LOOPBACK_OUT_EP_ADDR,
    },
    {
        .ep_cb   = loopback_in_cb,
        .ep_addr = LOOPBACK_IN_EP_ADDR,
    },
};
/* usb.rst endpoint configuration end */

static void loopback_status_cb(struct usb_cfg_data *cfg, enum usb_dc_status_code status,
                               const uint8_t *param) {
    ARG_UNUSED(cfg);

    switch (status) {
    case USB_DC_INTERFACE:
        loopback_in_cb(ep_cfg[LOOPBACK_IN_EP_IDX].ep_addr, 0);
        LOG_DBG("USB interface configured");
        break;
    case USB_DC_SET_HALT:
        LOG_DBG("Set Feature ENDPOINT_HALT");
        break;
    case USB_DC_CLEAR_HALT:
        LOG_DBG("Clear Feature ENDPOINT_HALT");
        if (*param == ep_cfg[LOOPBACK_IN_EP_IDX].ep_addr) {
            loopback_in_cb(ep_cfg[LOOPBACK_IN_EP_IDX].ep_addr, 0);
        }
        break;
    default:
        break;
    }
}

/* usb.rst vendor handler start */
static int loopback_vendor_handler(struct usb_setup_packet *setup, int32_t *len, uint8_t **data) {
    LOG_DBG("Class request: bRequest 0x%x bmRequestType 0x%x len %d", setup->bRequest,
            setup->bmRequestType, *len);

    if (USB_REQTYPE_GET_RECIPIENT(setup->bmRequestType) != USB_REQTYPE_RECIPIENT_INTERFACE) {
        return -ENOTSUP;
    }

    if (USB_REQTYPE_GET_DIR(setup->bmRequestType) == USB_REQTYPE_DIR_TO_HOST &&
        setup->bRequest == GET_SPI_MAGIC) {
        LOG_DBG("Device-to-Host, GET_SPI_CONFIG returning deadbeef");
        uint32_t db = 0xdeadbeefU;
        memcpy(*data, &db, MIN(sizeof(db), setup->wLength));
        return 0;
    } else if (USB_REQTYPE_GET_DIR(setup->bmRequestType) == USB_REQTYPE_DIR_TO_DEVICE &&
               setup->bRequest == SET_SPI_CONFIG) {
        LOG_DBG("Host-to-Device, SET_SPI_CONFIG len: %d", *len);
        return 0;
    } else if ((USB_REQTYPE_GET_DIR(setup->bmRequestType) == USB_REQTYPE_DIR_TO_HOST) &&
               (setup->bRequest == GET_SPI_CONFIG)) {
        for (uint32_t i = 0; i < setup->wLength; ++i) {
            (*data)[i] = i + 0x80;
        }
        LOG_DBG("Device-to-Host, GET_SPI_CONFIG len: %d", *len);
        return 0;
    }

    return -ENOTSUP;
}
/* usb.rst vendor handler end */

static void loopback_interface_config(struct usb_desc_header *head, uint8_t bInterfaceNumber) {
    ARG_UNUSED(head);

    loopback_cfg.if0.bInterfaceNumber = bInterfaceNumber;
}

/* usb.rst device config data start */
USBD_DEFINE_CFG_DATA(loopback) = {
    .usb_device_description = NULL,
    .interface_config       = loopback_interface_config,
    .interface_descriptor   = &loopback_cfg.if0,
    .cb_usb_status          = loopback_status_cb,
    .interface =
        {
            .class_handler  = NULL,
            .custom_handler = NULL,
            .vendor_handler = loopback_vendor_handler,
        },
    .num_endpoints = ARRAY_SIZE(ep_cfg),
    .endpoint      = ep_cfg,
};
/* usb.rst device config data end */
