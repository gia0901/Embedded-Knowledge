/* drv_panel_core — driver nen: KHONG biet ten chip, chi goi qua bang ops. */
#include <errno.h>
#include <stdio.h>
#include "panel_ops.h"

/* bảng ops đang dùng - do driver chip tự điền vào */
static const struct panel_ops *g_ops;

/* driver chipX dùng hàm này để truyền ops của nó vào g_ops của driver nền */
int panel_register_ops(const struct panel_ops* ops) {
    if (!ops || ops->size < sizeof(struct panel_ops)) {
        // bảng truyền vào cũ/ngắn hơn => reject
        fprintf(stderr, "[core] tu choi panel_ops: size=%zu < %zu\n",
                ops ? ops->size : 0, sizeof(struct panel_ops));
        return -EINVAL;
    }

    g_ops = ops;
    return g_ops->init ? g_ops->init() : 0;
}

int drv_panel_core_ioctl(int cmd, int arg) {
    if (!g_ops) return -ENODEV;

    switch (cmd) {
    case PANEL_IOC_SET_FREQ:
        return g_ops->set_freq ? g_ops->set_freq(arg) : -ENOTSUP;
    case PANEL_IOC_SET_BRIGHTNESS:
        return g_ops->set_brightness ? g_ops->set_brightness(arg) : -ENOTSUP;
    default:
        return -EINVAL;
    }
}
