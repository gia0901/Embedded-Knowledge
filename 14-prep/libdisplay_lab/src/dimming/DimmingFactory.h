#pragma once
#include <memory>
#include "IDimmingAlgo.h"
#include "IDimmingBackend.h"
#include "shm_types.h"

// ABSTRACT FACTORY: tao CẶP algo + backend khớp nhau. Bản này là cho ChipA
// (CMake chỉ build thư mục chip đích). Cặp sai vẫn compile sạch - nên chỉ tạo ở một chỗ
enum DimmingType_k {
    DIMMING_NONE,
    DIMMING_GLOBAL,
    DIMMING_LOCAL
};

// đọc model lúc chạy (qua biến env LIBDISPLAY_MODEL)
DimmingType_k detect_dimming_type();

class DimmingFactory {
public:
    // Mỗi algo nhận đúng vùng shm của nó - factory là nơi duy nhất ghép cặp
    DimmingFactory(DimmingType_k type, GlobalDimmingForShm& gshm, LocalDimmingForShm& lshm);

    IDimmingAlgo* GetDimmingObject() {
        return m_pDimmingObject.get();
    }

private:
    std::unique_ptr<IDimmingBackend>    m_pDimmingBackend; // khai báo trước: nó sống lâu hơn algo
    std::unique_ptr<IDimmingAlgo>       m_pDimmingObject;
};
