#include "panel_control.h"
#include "panel_ops.h"

int32_t panel_ctl_set_frequency(int32_t hz) { 
    return drv_panel_core_ioctl(PANEL_IOC_SET_FREQ, hz);
}

int32_t panel_ctl_write_brightness(int32_t level) {
    return drv_panel_core_ioctl(PANEL_IOC_SET_BRIGHTNESS, level);
}