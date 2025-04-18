#include "CrashCatch.hpp"

int main() {
    CrashCatch::Config config;
    config.appVersion = "2.0.0-beta";
    config.buildConfig = "Release";
    config.additionalNotes = "Triggered from full config example";

    config.onCrash = [](const CrashCatch::CrashContext& context) {
        std::cout << "Performing cleanup before crash...\n";
        std::cout << "Dump file: " << context.dumpFilePath << "\n";
        std::cout << "Log file: " << context.logFilePath << "\n";
        std::cout << "Signal/Code: " << context.signalOrCode << "\n";
    };

    config.showCrashDialog = true;

    CrashCatch::initialize(config);

    int* ptr = nullptr;
    *ptr = 42; // Simulated crash
    return 0;
}
