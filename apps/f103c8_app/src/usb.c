#include <usb/usb_device.h>
#include <zephyr.h>

static void fn_usb_dc_status_callback(enum usb_dc_status_code cb_status, const uint8_t *param)
{
    printk("cb_status: %d\n", cb_status);
}

int usb_init(const struct device *dev)
{
    int ret = 0;
    printk("Hello World! %s\n", CONFIG_BOARD);
    ret = usb_enable(fn_usb_dc_status_callback);
    if (ret)
    {
        printk("usb enable failed\n");
        return ret;
    }
    return ret;
}

SYS_INIT(usb_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);