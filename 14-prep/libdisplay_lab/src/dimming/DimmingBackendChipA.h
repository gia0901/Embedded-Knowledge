#pragma once
#include "IDimmingBackend.h"

// Backend cua ChipA, theo tung loai thuat toan: 
// moi lop chi override nhom cua minh.

class DimmingBackendChipA_Global : public IDimmingBackend {
public:
    uint32_t t_Set2DFinalDuty(BackendGd2DFinalDuty_t* p) override;
};

class DimmingBackendChipA_Local : public IDimmingBackend {
public:
    uint32_t t_SetLdFinalDuty(BackendLdFinalDuty_t* p) override;
};