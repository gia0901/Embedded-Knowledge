#include "GlobalDimming.h"

GlobalDimming::GlobalDimming(IDimmingBackend* pDimmingBackend, GlobalDimmingForShm& shm)
    : Shm(shm), m_pDimmingBackend(pDimmingBackend) {
    m_pDimmingBackend->t_InitGlobalDimming();
}

uint32_t GlobalDimming::SetBacklight(int32_t backlight) {
    if (backlight < 0)   backlight = 0;
    if (backlight > 100) backlight = 100;
    Shm.target = backlight; // chỉ set target, vsync sẽ tự điều khiển
    return LIB_OK;
}

int32_t GlobalDimming::GetBacklight() {
    return Shm.current;
}

void GlobalDimming::t_vSyncCallBack() { // ⚠️ cham Shm KHONG qua khoa (diem yeu #5)
    if (Shm.current == Shm.target)
        return;
    
    int32_t step = (Shm.target > Shm.current) ? 10 : -10;
    Shm.current += step;
    if ((step > 0 && Shm.current > Shm.target) || (step < 0 && Shm.current < Shm.target))
        Shm.current = Shm.target;
    
    ++Shm.steps;
    BackendGd2DFinalDuty_t duty{Shm.current};
    
    m_pDimmingBackend->t_Set2DFinalDuty(&duty);  // ghi HW qua con trỏ backend
}