#include "CrashCatch.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>

// noinline: without it, Release-mode optimizers are free to inline this straight
// into main(), which would erase the frame this test is checking for.
#if defined(_MSC_VER)
#define CRASHCATCH_TEST_NOINLINE __declspec(noinline)
#else
#define CRASHCATCH_TEST_NOINLINE __attribute__((noinline))
#endif

// This function is the actual crash site. Its name should appear in the stack trace.
// Give it a unique, obviously-named function so we can grep for it.
static volatile std::uintptr_t gCrashAddress = 0;
CRASHCATCH_TEST_NOINLINE void crashSiteFunction() {
    auto* ptr = reinterpret_cast<volatile int*>(gCrashAddress);
    *ptr = 42; // crash here
}

int main() {
    CrashCatch::Config config;
    config.dumpFolder    = "./test_stack_output/";
    config.dumpFileName  = "test_stack";
    config.autoTimestamp = false; // fixed name so we can find it
    config.appVersion    = "test";
    config.includeStackTrace = true;

    // Written after files are on disk, so we can read the log back here.
    config.onCrash = [](const CrashCatch::CrashContext& ctx) {
        std::ifstream logFile(ctx.logFilePath);
        std::stringstream buffer;
        buffer << logFile.rdbuf();
        std::string contents = buffer.str();

        auto traceStart = contents.find("Stack Trace:");
        bool present = traceStart != std::string::npos &&
                       contents.find("crashSiteFunction", traceStart) != std::string::npos;

#if defined(_WIN32)
        // Windows points the stack walk at the actual crash-site register context
        // (v1.4.0 fix), so frame [0] must be the crashing function itself.
        auto frame0 = contents.find("[0]:");
        bool pass = present && frame0 != std::string::npos &&
                    contents.find("crashSiteFunction", frame0) < contents.find('\n', frame0);
        const char* label = "frame [0] is crashSiteFunction";
#else
        // Linux captures the backtrace from inside the fork()ed child, after
        // linuxSignalHandler/writeCrashLog have already added their own frames,
        // so the crash site shows up further down the trace, not at [0].
        bool pass = present;
        const char* label = "crashSiteFunction present in trace";
#endif

        std::ofstream sentinel("./test_stack_output/sentinel.txt");
        sentinel << (pass ? "PASS: " : "FAIL: ") << label << "\n";
        sentinel << "logFilePath: " << ctx.logFilePath << "\n";
        std::cout << "[onCrash] " << label << ": " << (pass ? "YES (PASS)" : "NO (FAIL)") << "\n";
    };

    CrashCatch::initialize(config);
    crashSiteFunction();
    return 0;
}
