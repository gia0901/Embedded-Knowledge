#include "LocalDimming.h"

LocalDimming::LocalDimming(IDimmingBackend* pDimmingBackend, LocalDimmingForShm& shm)
    : Shm(shm), m_pDimmingBackend(pDimmingBackend) {
    m_pDimmingBackend->t_InitLocalDimming();
}

uint32_t LocalDimming::SetBacklight(int32_t backlight) {
    BackendLdFinalDuty_t d{{backlight, backlight, backlight / 2, backlight / 2}};  // vung duoi toi hon
    Shm.level = backlight;
    for (int i = 0; i < 4; ++i) Shm.zones[i] = d.zones[i];   // muc tung vung cung nam o shm: process khac doc duoc
    return m_pDimmingBackend->t_SetLdFinalDuty(&d);
}
int32_t LocalDimming::GetBacklight() {
    return Shm.level;
}