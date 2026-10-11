// panel_control.h — Panel Control: ham tu do, MOT lenh -> ioctl. Co y KHONG dung pattern:
// bien the theo chip da nam o panel driver (kernel_sim/drv_panel_chip*).
#pragma once
#include <cstdint>


int32_t panel_ctl_set_frequency(int32_t hz);
int32_t panel_ctl_write_brightness(int32_t level);    // noi bo: chi backend cua PQ goi