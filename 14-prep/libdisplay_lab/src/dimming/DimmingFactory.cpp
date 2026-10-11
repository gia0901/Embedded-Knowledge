#include "DimmingFactory.h"
#include <cstdlib>
#include <cstring>
#include <memory>
#include "DimmingBackendChipA.h"
#include "GlobalDimming.h"
#include "LocalDimming.h"
#include "shm_types.h"

DimmingType_k detect_dimming_type() {
    const char* m = std::getenv("LIBDISPLAY_MODEL");

    if (m && std::strcmp(m, "local") == 0)
        return DIMMING_LOCAL;
    if (m && std::strcmp(m, "none") == 0)
        return DIMMING_NONE;
    
    return DIMMING_GLOBAL; // default an toàn
}

DimmingFactory::DimmingFactory(DimmingType_k type, GlobalDimmingForShm& gshm,
LocalDimmingForShm& lshm) {
    switch (type) {
    case DIMMING_LOCAL:
        m_pDimmingBackend = std::make_unique<DimmingBackendChipA_Local>();
        m_pDimmingObject = std::make_unique<LocalDimming>(m_pDimmingBackend.get(), lshm);
        break;
    case DIMMING_GLOBAL:
        m_pDimmingBackend = std::make_unique<DimmingBackendChipA_Global>();
        m_pDimmingObject  = std::make_unique<GlobalDimming>(m_pDimmingBackend.get(), gshm);
        break;
    case DIMMING_NONE:
        m_pDimmingObject  = std::make_unique<IDimmingAlgo>(); // NULL OBJECT
        // ko cần backend - vì đang dùng null object không đụng HW
        break;
    }
}
