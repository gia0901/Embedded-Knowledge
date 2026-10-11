// shm_types.h — state CHUNG moi process. 
// Layout nay la ABI giua cac process.
#pragma once
#include <cstdint>

struct DimmingForShm {
    int32_t m_iPlatformType;
    int32_t requested;
};

struct GlobalDimmingForShm {
    int32_t target;
    int32_t current;
    int32_t steps;
};

struct LocalDimmingForShm {
    int32_t level;
    int32_t zones[4];
};