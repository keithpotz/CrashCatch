#define CRASHCATCH_AUTO_INIT
#include "CrashCatch.hpp"
#include <iostream>

int main() {
    CrashCatch::Config config;
    config.appVersion = "1.2.0";
    config.buildConfig = "Debug";
    config.onCrash = [](const CrashCatch::CrashContext& ctx) {
        std::cout << "Custom crash handler triggered!\n";
        std::cout << "Dump File: " << ctx.dumpFilePath << "\n";
        std::cout << "Log File: " << ctx.logFilePath << "\n";
        std::cout << "Exception Code: " << std::hex << ctx.signalOrCode << "\n";
    };
    CrashCatch::initialize(config);

    // Simulate crash
    int* ptr = nullptr;
    *ptr = 42; // CRASH!
}
