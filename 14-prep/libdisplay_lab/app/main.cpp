// App chi biet lib_api.h — khong biet PQ, Panel Control, chip nao.
//   ./display_demo          đặt freq + brightness, cho vsync tiến dần, rồi đọc lại
//   ./display_demo --read   CHỈ doc: chứng minh state nằm ở shm, process nào cũng thấy cùng số
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>
#include "lib_api.h"

int main(int argc, char** argv) {
    // 1. chỉ đọc (--read)
    if (argc > 1 && std::strcmp(argv[1], "--read") == 0) {
        std::printf("backlight=%d\n", lib_api_get_backlight());
        return 0;
    }

    // 2. đặt freq + brightness
    std::printf("set_frequency(120) -> %d\n", lib_api_set_frequency(120));
    std::printf("set_backlight(60)  -> %d\n", lib_api_set_backlight(60));
        // chờ vsync tự chạy
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // đọc lại giá trị mới
    std::printf("backlight=%d\n", lib_api_get_backlight());


    return 0;
}
