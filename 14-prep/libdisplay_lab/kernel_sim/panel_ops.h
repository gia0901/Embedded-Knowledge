/* panel_ops.h — "kernel" gia lap: bang con tro ham = vtable viet tay bang C.
 * Luat ABI: CHI them o CUOI struct, va kiem 'size' luc dang ky. */
#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct panel_ops {
    size_t size;  /* = sizeof(struct panel_ops) bên điền bảng */
    int (*init)(void); 
    int (*set_freq)(int hz);
    int (*set_brightness)(int level);
};

enum {
    PANEL_IOC_SET_FREQ = 1,
    PANEL_IOC_SET_BRIGHTNESS = 2,
};

int panel_register_ops(const struct panel_ops* ops); // driver chip gọi lúc được insmod
int drv_panel_core_ioctl(int cmd, int arg); // thay cho ioctl thật

#ifdef __cplusplus
}
#endif