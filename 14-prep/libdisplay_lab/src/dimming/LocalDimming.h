#pragma once
#include "IDimmingAlgo.h"
#include "IDimmingBackend.h"
#include "shm_types.h"

// Thuat toan LOCAL (rut gon): chia muc sang cho 4 vung, ghi ngay.
class LocalDimming : public IDimmingAlgo {
public:
    LocalDimming(IDimmingBackend* pDimmingBackend, LocalDimmingForShm& shm);
    uint32_t SetBacklight(int32_t backlight) override;
    int32_t  GetBacklight() override;

protected:
    LocalDimmingForShm& Shm; // shm CỦA LOCAL
    IDimmingBackend*     m_pDimmingBackend;
};