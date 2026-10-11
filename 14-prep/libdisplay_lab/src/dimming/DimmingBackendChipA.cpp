#include "DimmingBackendChipA.h"
#include "panel_control.h"

// Backend KHONG tu goi ioctl: 
// ghi gia tri cuoi qua Panel Control (duy nhat xuong phan cung).

uint32_t DimmingBackendChipA_Global::t_Set2DFinalDuty(BackendGd2DFinalDuty_t* p) {
    return static_cast<uint32_t>(panel_ctl_write_brightness(p->value));
}

uint32_t DimmingBackendChipA_Local::t_SetLdFinalDuty(BackendLdFinalDuty_t* p) {
    int32_t avg = (p->zones[0] + p->zones[1] + p->zones[2] + p->zones[3]) / 4;
    return static_cast<uint32_t>(panel_ctl_write_brightness(avg));
}