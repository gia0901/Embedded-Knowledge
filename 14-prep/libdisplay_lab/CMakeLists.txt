# ============================================================================
# CÁCH BUILD — chạy từ thư mục gốc libdisplay_lab/
#   -S <nguồn> -B <thư mục build>: cmake tự tạo thư mục build nếu chưa có.
#   Đổi option (-D...) thì chạy lại lệnh cmake -S/-B; muốn build sạch thì xoá thư mục build.
#
# 1) Build thường (Debug, có unit test — lần đầu cần mạng để tải GoogleTest):
#      cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
#      cmake --build build -j8                       # phải 0 warning
#      ./build/bin/display_demo                      # đặt tần số + độ sáng, đọc lại
#      ./build/bin/display_demo --read               # process thứ hai: chỉ đọc state trong shm
#      (cd build && ctest --output-on-failure)       # unit test + integration test
#    Không có mạng / không cần test: thêm -DLIBDISPLAY_TESTS=OFF
#
# 2) Coverage (gcovr: pip3 install --user gcovr):
#      cmake -S . -B build_cov -DLIBDISPLAY_COVERAGE=ON && cmake --build build_cov -j8
#      (cd build_cov && ctest) && gcovr --root . --filter src/ --filter kernel_sim/ build_cov
#
# 3) ThreadSanitizer — bắt race của vòng vsync:
#      cmake -S . -B build_tsan -DLIBDISPLAY_TSAN=ON -DLIBDISPLAY_TESTS=OFF && cmake --build build_tsan -j8
#      setarch -R ./build_tsan/bin/display_demo      # -R tắt ASLR: TSan của gcc 11 cần trên kernel 6.x
#
# 4) ASan + UBSan — lỗi bộ nhớ / UB lúc chạy:
#      F="-fsanitize=address,undefined -fno-omit-frame-pointer -g"
#      cmake -S . -B build_asan -DCMAKE_CXX_FLAGS="$F" -DCMAKE_C_FLAGS="$F" \
#            -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" \
#            -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=address,undefined"
#      cmake --build build_asan -j8 && (cd build_asan && ctest --output-on-failure)
#
# 5) Cả pipeline 5 cổng: ./run_ci.sh   (06-build-systems/unit-test-and-code-quality.md §6)
#
# Biến môi trường khi chạy: LIBDISPLAY_MODEL=global|local|none · LIBDISPLAY_SHM=/tên_khác
# State lạ từ lần chạy trước (shm sống lâu hơn process):
#      rm /dev/shm/libdisplay_lab /dev/shm/sem.libdisplay_lab_sem
# ============================================================================
cmake_minimum_required(VERSION 3.16)
project(libdisplay_lab CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

option(LIBDISPLAY_TESTS     "Build unit test (GoogleTest via FetchContent)" ON)
option(LIBDISPLAY_COVERAGE  "Build with --coverage (gcov)"                  OFF)
option(LIBDISPLAY_TSAN      "Build with ThreadSanitizer"                    OFF)

if(LIBDISPLAY_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

if(LIBDISPLAY_TSAN)
    add_compile_options(-fsanitize=thread -g -O1)
    add_link_options(-fsanitize=thread)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- "kernel" gia lap o user-space: drv_panel_core + panel_ops + drv_panel_chipA
#      OBJECT library: neu la STATIC, linker se BO panel_chipA.o (khong ai tham chieu
#      toi no, chi co constructor) => driver chip khong bao gio tu dang ky.
add_library(panel_sim OBJECT 
    kernel_sim/drv_panel_core.c
    kernel_sim/drv_panel_chipA.c)
set_target_properties(panel_sim PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(panel_sim PUBLIC kernel_sim)

# Core (lõi) C++ của library: PQ + Panel control (unit test link thẳng vào đây)
add_library(display_core STATIC
    src/panel_control.cpp
    src/dimming/IDimmingAlgo.cpp
    src/dimming/GlobalDimming.cpp
    src/dimming/LocalDimming.cpp
    src/dimming/DimmingBackendChipA.cpp
    src/dimming/DimmingFactory.cpp
    src/dimming/lib_dimming.cpp)
set_target_properties(display_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(display_core PUBLIC include src src/dimming)
target_link_libraries(display_core PUBLIC panel_sim)


# libdisplay.so: mặt tiền C + shm/semaphore + constructor + vsync
add_library(display SHARED src/lib_api.cpp src/lib_base.cpp $<TARGET_OBJECTS:panel_sim>)
target_link_libraries(display PRIVATE display_core pthread rt)
target_include_directories(display PUBLIC include)

# APP: chỉ thấy lib_api.h
add_executable(display_demo app/main.cpp)
target_link_libraries(display_demo PRIVATE display)
set_target_properties(display_demo PROPERTIES BUILD_RPATH "$ORIGIN")

# TEST: TBD

