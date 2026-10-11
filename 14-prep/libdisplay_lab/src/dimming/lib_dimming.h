#pragma once
#include <memory>
#include "DimmingFactory.h"
#include "shm_types.h"

// Hợp đồng module (DIP) + ủy nhiệm. Một implementation nên chưa phải strategy ở tầng này
class lib_dimming_interface {
public:
    virtual ~lib_dimming_interface() = default;
    virtual uint32_t SetBacklight(int32_t v) = 0;
    virtual int32_t  GetBacklight() = 0;
    virtual void     OnVSync() = 0; // chạy bởi thread vsync
};

class lib_dimming : public lib_dimming_interface {
public:
    lib_dimming(DimmingType_k type, GlobalDimmingForShm& gshm, LocalDimmingForShm& lshm)
        : m_factory(type, gshm, lshm) {}

    uint32_t SetBacklight(int32_t v) override {
        return m_factory.GetDimmingObject()->SetBacklight(v); // algo->SetBacklight(v)
    }

    int32_t GetBacklight() override {
        return m_factory.GetDimmingObject()->GetBacklight();
    }

    void OnVSync() override { // chạy bởi thread vsync
        m_factory.GetDimmingObject()->t_vSyncCallBack();
    }

private:
    DimmingFactory m_factory;
};