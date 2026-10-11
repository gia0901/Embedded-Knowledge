// IDimmingBackend — Bridge IMPLEMENTOR: nua "ghi phan cung", theo chip.
#pragma once
#include <cstdint>
#include "IDimmingAlgo.h"

struct BackendGd2DFinalDuty_t {
    int32_t value;
};
struct BackendLdFinalDuty_t {
    int32_t zones[4];
};

class IDimmingBackend {
public:
    virtual ~IDimmingBackend() = default;
    // global dimming backend
    virtual uint32_t t_InitGlobalDimming()                         { return LIB_OK; }
    virtual uint32_t t_Set2DFinalDuty(BackendGd2DFinalDuty_t*)     { return LIB_OK; }  // than mac dinh
    // local dimming backend
    virtual uint32_t t_InitLocalDimming()                          { return LIB_OK; }
    virtual uint32_t t_SetLdFinalDuty(BackendLdFinalDuty_t*)       { return LIB_OK; }
};