
#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(test_hid);

#include <zephyr.h>

#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>

#define REPORT_ID_1 0x01
#define REPORT_ID_2 0x02

static const struct device *hid_dev;
static K_SEM_DEFINE(usb_configured, 0, 1);

/* Some HID sample Report Descriptor */
static const uint8_t hid_report_desc[] = {
    /* 0x05, 0x01,		USAGE_PAGE (Generic Desktop)		*/
    HID_GI_USAGE_PAGE,
    USAGE_GEN_DESKTOP,
    /* 0x09, 0x00,		USAGE (Undefined)			*/
    HID_LI_USAGE,
    USAGE_GEN_DESKTOP_UNDEFINED,
    /* 0xa1, 0x01,		COLLECTION (Application)		*/
    HID_MI_COLLECTION,
    COLLECTION_APPLICATION,
    /* 0x15, 0x00,			LOGICAL_MINIMUM one-byte (0)	*/
    HID_GI_LOGICAL_MIN(1),
    0x00,
    /* 0x26, 0xff, 0x00,		LOGICAL_MAXIMUM two-bytes (255)	*/
    HID_GI_LOGICAL_MAX(2),
    0xFF,
    0x00,
    /* 0x85, 0x01,			REPORT_ID (1)			*/
    HID_GI_REPORT_ID,
    REPORT_ID_1,
    /* 0x75, 0x08,			REPORT_SIZE (8) in bits		*/
    HID_GI_REPORT_SIZE,
    0x08,
    /* 0x95, 0x01,			REPORT_COUNT (1)		*/
    HID_GI_REPORT_COUNT,
    0x01,
    /* 0x09, 0x00,			USAGE (Undefined)		*/
    HID_LI_USAGE,
    USAGE_GEN_DESKTOP_UNDEFINED,
    /* v0x81, 0x82,			INPUT (Data,Var,Abs,Vol)	*/
    HID_MI_INPUT,
    0x82,
    /* 0x85, 0x02,			REPORT_ID (2)			*/
    HID_GI_REPORT_ID,
    REPORT_ID_2,
    /* 0x75, 0x08,			REPORT_SIZE (8) in bits		*/
    HID_GI_REPORT_SIZE,
    0x08,
    /* 0x95, 0x01,			REPORT_COUNT (1)		*/
    HID_GI_REPORT_COUNT,
    0x01,
    /* 0x09, 0x00,			USAGE (Undefined)		*/
    HID_LI_USAGE,
    USAGE_GEN_DESKTOP_UNDEFINED,
    /* 0x91, 0x82,			OUTPUT (Data,Var,Abs,Vol)	*/
    HID_MI_OUTPUT,
    0x82,
    /* 0xc0			END_COLLECTION			*/
    HID_MI_COLLECTION_END,
};

int debug_cb(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
    LOG_DBG("Debug callback");
    return -ENOTSUP;
}

int set_idle_cb(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
    LOG_DBG("Set Idle callback");
    return 0;
}

int get_report_cb(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
    LOG_DBG("Get report callback");
    return 0;
}

static struct hid_ops ops = {
    .get_report = get_report_cb,
    .get_idle = debug_cb,
    .get_protocol = debug_cb,
    .set_report = debug_cb,
    .set_idle = set_idle_cb,
    .set_protocol = debug_cb,
};

static void hid_thread(void *p1, void *p2, void *p3)
{
    uint8_t report_1[2] = {REPORT_ID_1, 0x00};
    int ret, wrote;

	k_sem_take(&usb_configured, K_FOREVER);

    LOG_DBG("Starting hid loop");

    while (true)
    {
        k_sleep(K_SECONDS(1));

        report_1[1]++;

        ret = hid_int_ep_write(hid_dev, report_1, sizeof(report_1), &wrote);
        LOG_DBG("Wrote %d bytes with ret %d", wrote, ret);
    }
}

static void fn_usb_dc_status_callback(enum usb_dc_status_code cb_status, const uint8_t *param)
{
    LOG_INF("cb_status: %d\n", cb_status);
	if (cb_status == USB_DC_CONFIGURED) {
		LOG_INF("usb dc configured");
		k_sem_give(&usb_configured);
	}
}

static int sys_init_usb(const struct device *dev)
{
    int ret = usb_enable(fn_usb_dc_status_callback);
    if (ret)
    {
        LOG_ERR("usb enable failed\n");
		return -1;
    }
	LOG_INF("usb enable success\n");
    return 0;
}

static int sys_init_usb_hid(const struct device *dev)
{
    hid_dev = device_get_binding("HID_0");
    if (hid_dev == NULL)
    {
        LOG_ERR("Cannot get HID_0 Device");
        return -1;
    }

    usb_hid_register_device(hid_dev, hid_report_desc, sizeof(hid_report_desc), &ops);
    usb_hid_init(hid_dev);
	LOG_INF("usb hid registered");
    return 0;
}

K_THREAD_DEFINE(hid_thread_id, 1024, hid_thread, NULL, NULL, NULL, 1, 0, 0);

SYS_INIT(sys_init_usb_hid, POST_KERNEL, 0);
SYS_INIT(sys_init_usb, POST_KERNEL, 1);
