#include <thread>
#include "CrashCatch.hpp"

void crashInThread() {
    int* ptr = nullptr;
    *ptr = 42;
}
int main() {
    CrashCatch::enable();

    std::thread t(crashInThread);
    t.join();
    return 0;
}