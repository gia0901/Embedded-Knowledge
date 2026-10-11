#include "lib_base.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>

namespace {
display_shm_info*      g_shm = nullptr;
sem_t*                 g_sem = SEM_FAILED;
lib_dimming_interface* dimming = nullptr;     // gan MOT lan trong lib_init_modules()
std::atomic<bool>      g_vsync_stop{false};
std::thread            g_vsync;

std::string shm_name() {
    const char* n = std::getenv("LIBDISPLAY_SHM");
    return n ? n : "/libdisplay_lab";
}

// tao/mo shm + semaphore dung chung moi process
void lib_init_device() {
    const std::string name = shm_name();

    int fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0600);
    if (fd < 0) {
        perror("shm_open"); return;
    }

    if (ftruncate(fd, sizeof(display_shm_info)) != 0) {
        perror("ftruncate"); close(fd); return;
    }

    void* p = mmap(nullptr, sizeof(display_shm_info), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (p == MAP_FAILED) {
        perror("mmap"); return;
    }

    g_shm = static_cast<display_shm_info*>(p);
    g_sem = sem_open((name + "_sem").c_str(), O_CREAT, 0600, 1);
    if (g_sem == SEM_FAILED) {
        perror("sem_open"); return;
    }

    lib_sem_lock(__func__);
    if (!g_shm->init) { 
        std::memset(g_shm, 0, sizeof(*g_shm)); g_shm->init = 1; // process dau tien
    }   
    ++g_shm->ref_count;
    lib_sem_unlock(__func__);
}

// COMPOSITION ROOT: dung moi object, mot lan
void lib_init_modules() {                     
    DimmingType_k type = detect_dimming_type();
    g_shm->shmDimming.m_iPlatformType = type;
    dimming = new lib_dimming(type, g_shm->shmGlobalDimming, g_shm->shmLocalDimming);
}

void vsync_loop() { // ⚠️ KHONG lay semaphore (diem yeu #5 cua A1)
    auto next = std::chrono::steady_clock::now();
    while (!g_vsync_stop) {
        next += std::chrono::milliseconds(16);
        std::this_thread::sleep_until(next);
        if (dimming) dimming->OnVSync();
    }
}
} // namespace

display_shm_info* get_lib_shm() {
    return g_shm;
}

int get_check_shm() {
    return (g_shm && g_sem != SEM_FAILED) ? 0 : -1;

}
lib_dimming_interface* get_dimming_instance() {
    if (!dimming) std::fprintf(stderr, "dimming is NULL\n");
    return dimming;
}

int  lib_sem_lock(const char*)   { return sem_wait(g_sem); }
void lib_sem_unlock(const char*) { sem_post(g_sem); }

// Tu chay khi process nap libdisplay.so 
// TRUOC khi main() tao thread nao => khong race khoi tao.
__attribute__((constructor)) static void lib_init() {
    lib_init_device();
    if (get_check_shm() != 0) return;
    lib_init_modules();
    g_vsync = std::thread(vsync_loop);
}

__attribute__((destructor)) static void lib_base_final() {
    g_vsync_stop = true;
    
    if (g_vsync.joinable())
        g_vsync.join();
    
    if (g_shm && g_sem != SEM_FAILED) {
        lib_sem_lock(__func__);
        --g_shm->ref_count;
        lib_sem_unlock(__func__);
    }
}