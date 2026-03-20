#include "CrashCatch.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>

// Test: onCrash callback should see the log file already on disk.
// We set a flag if the file exists when the callback fires.
// The process exits after the crash handler runs, so we write the result
// to a sentinel file that the parent (or manual check) can inspect.

static bool g_fileExistedOnCallback = false;

int main() {
    CrashCatch::Config config;
    config.dumpFolder    = "./test_crash_output/";
    config.dumpFileName  = "test_callback_order";
    config.autoTimestamp = false; // fixed name so we can find it
    config.appVersion    = "test";

    config.onCrash = [](const CrashCatch::CrashContext& ctx) {
        bool exists = std::filesystem::exists(ctx.logFilePath);
        // Write sentinel so we can check after process exits
        std::ofstream sentinel("./test_crash_output/sentinel.txt");
        sentinel << (exists ? "PASS: file existed" : "FAIL: file did NOT exist") << "\n";
        sentinel << "logFilePath: " << ctx.logFilePath << "\n";
        std::cout << "[onCrash] file exists: " << (exists ? "YES (PASS)" : "NO (FAIL)") << "\n";
    };

    CrashCatch::initialize(config);

    // Trigger a crash
    int* ptr = nullptr;
    *ptr = 42;
    return 0;
}
