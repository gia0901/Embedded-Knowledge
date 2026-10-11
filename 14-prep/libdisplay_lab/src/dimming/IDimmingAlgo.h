// IDimmingAlgo — STRATEGY (nhin tu lib_dimming) + ABSTRACTION cua Bridge (nhin sang backend).
// Moi method base trả LIB_OK => chinh base class la NULL OBJECT (platform khong panel).
#pragma once
#include <cstdint>
#include "lib_api.h"

class IDimmingAlgo {
public:
    virtual ~IDimmingAlgo() = default;
    virtual uint32_t SetBacklight(int32_t backlight);
    virtual void     t_vSyncCallBack(); // vong vsync goi moi khung hinh
    virtual int32_t  GetBacklight(); // doc tu STATE của thuật toán đang dùng 
};
