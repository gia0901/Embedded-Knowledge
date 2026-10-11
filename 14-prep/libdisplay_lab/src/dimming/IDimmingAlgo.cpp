#include "IDimmingAlgo.h"

uint32_t IDimmingAlgo::SetBacklight(int32_t) {
    return LIB_OK;  // no-op thanh cong
}

int32_t  IDimmingAlgo::GetBacklight() {
    return LIB_OK;  // khong panel: khong co gi de doc
} 

void IDimmingAlgo::t_vSyncCallBack() {

}