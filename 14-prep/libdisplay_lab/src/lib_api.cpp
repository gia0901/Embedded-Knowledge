// lib_api.cpp — FACADE: moi ham cung mot khuon  shm -> KHOA -> dieu phoi -> MO KHOA.
#include "lib_api.h"
#include "lib_base.h"
#include "panel_control.h"

#ifdef __cplusplus
    extern "C" {
#endif


int32_t lib_api_set_backlight(int32_t backlight) {
    if (get_check_shm() == -1) 
        return -1;

    int sem_ret = lib_sem_lock(__func__);

    int32_t ret = LIB_OK;
    if (get_dimming_instance() != nullptr)
        ret = static_cast<int32_t>(get_dimming_instance()->SetBacklight(backlight));   // tuyen PQ
    
    if (sem_ret == 0) lib_sem_unlock(__func__);
    
    return ret;
}

int32_t lib_api_get_backlight(void) {
    if (get_check_shm() == -1) 
        return -1;

    int sem_ret = lib_sem_lock(__func__);
    int32_t v = (get_dimming_instance() != nullptr) 
                ? get_dimming_instance()->GetBacklight() : 0;

    if (sem_ret == 0) lib_sem_unlock(__func__);
    
    return v;
}
int32_t lib_api_set_frequency(int32_t hz) {
    if (get_check_shm() == -1) 
        return -1;
    int sem_ret = lib_sem_lock(__func__);
    int32_t ret = panel_ctl_set_frequency(hz); // tuyen Panel Control
    if (sem_ret == 0) lib_sem_unlock(__func__);

    return ret;
}


#ifdef __cplusplus
    }
#endif