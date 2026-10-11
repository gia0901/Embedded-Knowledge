// lib_base.h — idiom IPC: object per-process, state per-system.
#pragma once
#include <semaphore.h>
#include "dimming/shm_types.h"
#include "lib_dimming.h"

struct display_shm_info {
    int32_t             init;
    int32_t             ref_count;      // đếm process đang nạp library
    DimmingForShm       shmDimming;
    GlobalDimmingForShm shmGlobalDimming;
    LocalDimmingForShm  shmLocalDimming;
};

display_shm_info*       get_lib_shm();
int                     get_check_shm();                // 0 = sẵn sàng, -1 = chưa
int                     lib_sem_lock(const char* who);  // named semaphore
void                    lib_sem_unlock(const char* who);
lib_dimming_interface*  get_dimming_instance();         // SERVICE LOCATOR (không phải singleton)