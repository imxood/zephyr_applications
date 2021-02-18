
#define LOG_LEVEL CONFIG_LOG_MAX_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(test_hid);

#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>

#define REPORT_ID_1 0x01
#define REPORT_ID_2 0x02

int usb_waitForConfigured(k_timeout_t timeout);
int usb_waitForSuspended(k_timeout_t timeout);
int usb_waitForReadReady(k_timeout_t timeout);

static const struct device *hid0_dev;
static K_SEM_DEFINE(hid0_out_ready, 0, 1);

static struct usb_status
{
    enum usb_dc_status_code cur_status;
    enum usb_dc_status_code pre_status;
    enum usb_dc_status_code status;
    struct k_sem usb_configured;
    struct k_sem usb_suppended;
} usb_status;

/* Some HID sample Report Descriptor */
static const uint8_t hid0_report_desc[] = {
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

static inline void usb_init_status()
{
    usb_status.cur_status = USB_DC_UNKNOWN;
    usb_status.pre_status = USB_DC_UNKNOWN;
    k_sem_init(&usb_status.usb_configured, 0, 1);
    k_sem_init(&usb_status.usb_suppended, 0, 1);
}

static inline void usb_set_status(enum usb_dc_status_code cb_status)
{
    if (cb_status == USB_DC_CONFIGURED)
    {
        usb_status.pre_status = usb_status.cur_status;
        usb_status.cur_status = USB_DC_CONFIGURED;
        k_sem_give(&usb_status.usb_configured);
    }
    else if (cb_status == USB_DC_SUSPEND)
    {
        if (usb_status.cur_status == USB_DC_CONFIGURED)
        {
            k_sem_give(&usb_status.usb_suppended);
        }
		usb_status.pre_status = usb_status.cur_status;
        usb_status.cur_status = USB_DC_SUSPEND;
    }
}

static void usb_dc_status_cb(enum usb_dc_status_code cb_status, const uint8_t *param)
{
    usb_set_status(cb_status);
}

int usb_waitForConfigured(k_timeout_t timeout) {
	return k_sem_take(&usb_status.usb_configured, timeout);
}

int usb_waitForSuspended(k_timeout_t timeout) {
	return k_sem_take(&usb_status.usb_suppended, timeout);
}

int usb_waitForReadReady(k_timeout_t timeout) {
	return k_sem_take(&hid0_out_ready, timeout);
}

static int debug_cb(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
    LOG_DBG("Debug callback");
    return -ENOTSUP;
}

static int set_idle_cb(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
    LOG_DBG("Set Idle callback");
    return 0;
}

static int get_report_cb(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
    LOG_DBG("Get report callback");
    return 0;
}

static void hid0_out_ready_cb(const struct device *dev)
{
    LOG_INF("usb hid0 out ready!");
    k_sem_give(&hid0_out_ready);
}

static struct hid_ops ops = {.get_report = get_report_cb,
                             .get_idle = debug_cb,
                             .get_protocol = debug_cb,
                             .set_report = debug_cb,
                             .set_idle = set_idle_cb,
                             .set_protocol = debug_cb,
                             .int_out_ready = hid0_out_ready_cb};

static int sys_init_usb(const struct device *dev)
{
    usb_init_status();

    int ret = usb_enable(usb_dc_status_cb);
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
    hid0_dev = device_get_binding("HID_0");
    if (hid0_dev == NULL)
    {
        LOG_ERR("Cannot get HID_0 Device");
        return -1;
    }

    usb_hid_register_device(hid0_dev, hid0_report_desc, sizeof(hid0_report_desc), &ops);
    usb_hid_init(hid0_dev);
    LOG_INF("usb hid registered");
    return 0;
}

static void hid0_read_thread(void *p1, void *p2, void *p3)
{
    uint8_t report[64];
    int ret, read;

    if (hid0_dev == NULL)
    {
        LOG_ERR("HID_0 Device is NULL, read hid read thread failed");
        return;
    }

    while (true)
    {
        LOG_INF("wait for hid0 out ready ..");
        memset(report, 0, sizeof(report));
		usb_waitForReadReady(K_FOREVER);

        printk("hid0 out msg receoved!\n");
        ret = hid_int_ep_read(hid0_dev, report, sizeof(report), &read);
        LOG_DBG("Wrote %d bytes with ret %d", read, ret);
        LOG_HEXDUMP_INF(report, sizeof(report), "read hid data");
    }
}

static K_THREAD_STACK_DEFINE(usb_stack, 1024);

static void usb_thread(void *p1, void *p2, void *p3)
{
    static struct k_thread hid0_read_tid;
    while (1)
    {
        /* 等待 usb setup */
		usb_waitForConfigured(K_FOREVER);
        k_thread_create(&hid0_read_tid, usb_stack, 1024, hid0_read_thread, NULL, NULL, NULL, 1, 0, K_NO_WAIT);
        /* 拔出 usb设备 或 usb设备 suppend时 删除线程, 等待 usb 重新 setup */
		usb_waitForSuspended(K_FOREVER);
        k_thread_abort(&hid0_read_tid);
    }
}

SYS_INIT(sys_init_usb_hid, POST_KERNEL, 0);
SYS_INIT(sys_init_usb, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);

K_THREAD_DEFINE(hid_thread_id, 1024, usb_thread, NULL, NULL, NULL, 1, 0, 0);
