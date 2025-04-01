#include "CrashCatch.hpp"

int main() {
    CrashCatch::enable();

    int x = 1;
    int y = 0;
    int z = x / y; // Division by zero
    return 0;
}