#pragma once
#include "IDimmingAlgo.h"
#include "IDimmingBackend.h"
#include "shm_types.h"

// Thuat toan GLOBAL: mot gia tri cho ca panel; 
// tien dan toi target moi khung hinh.
class GlobalDimming : public IDimmingAlgo {
public:
    GlobalDimming(IDimmingBackend* pDimmingBackend, GlobalDimmingForShm& shm);
    uint32_t SetBacklight(int32_t backlight) override;
    int32_t  GetBacklight() override;
    void     t_vSyncCallBack() override;

protected:
    GlobalDimmingForShm& Shm;                 // reference member: gan dung MOT vung state
    IDimmingBackend*     m_pDimmingBackend;   // Bridge sang phần cứng (Backend)
};