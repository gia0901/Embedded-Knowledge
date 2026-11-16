// transfer: hai thread chuyển tiền qua lại giữa hai tài khoản, mỗi tài khoản một mutex.
// "bad": mỗi thread khóa tài khoản nguồn trước, đích sau -> hai thread khóa ngược thứ tự nhau.
// "ordered": luôn khóa tài khoản có id nhỏ trước. "scoped": std::scoped_lock khóa cả hai cùng lúc.
// Lời giải Lab 5.1 và 5.3. Build: g++ -O2 -g -std=c++20 -pthread -o transfer transfer.cpp
// Chạy:  ./transfer bad|ordered|scoped <số lần chuyển mỗi thread>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <pthread.h>
#include <thread>

struct Account { int id; long balance; std::mutex m; };
static Account g_a{1, 1000}, g_b{2, 1000};
static const char* g_mode;

static void transfer(Account& from, Account& to, long amount) {
    if (!std::strcmp(g_mode, "bad")) {
        std::lock_guard l1(from.m);                       // khóa nguồn...
        std::lock_guard l2(to.m);                         // ...rồi đích: thread kia làm ngược lại
        from.balance -= amount; to.balance += amount;
    } else if (!std::strcmp(g_mode, "ordered")) {
        Account& first = from.id < to.id ? from : to;     // thứ tự toàn cục: id nhỏ trước
        Account& second = from.id < to.id ? to : from;
        std::lock_guard l1(first.m);
        std::lock_guard l2(second.m);
        from.balance -= amount; to.balance += amount;
    } else {
        std::scoped_lock lk(from.m, to.m);                // thuật toán tránh deadlock của thư viện
        from.balance -= amount; to.balance += amount;
    }
}

int main(int argc, char** argv) {
    g_mode = argv[1];
    long n = std::atol(argv[2]);
    std::thread t1([&] { pthread_setname_np(pthread_self(), "a-to-b"); for (long i = 0; i < n; ++i) transfer(g_a, g_b, 1); });
    std::thread t2([&] { pthread_setname_np(pthread_self(), "b-to-a"); for (long i = 0; i < n; ++i) transfer(g_b, g_a, 1); });
    t1.join(); t2.join();
    std::printf("%s: xong %ld lan moi chieu, a=%ld b=%ld tong=%ld\n", g_mode, n, g_a.balance, g_b.balance, g_a.balance + g_b.balance);
}
