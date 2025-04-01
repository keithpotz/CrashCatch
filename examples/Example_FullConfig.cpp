#include "CrashCatch.hpp"

int main() {
    CrashCatch::Config config;
    config.appVersion = "2.0.0-beta";
    config.buildConfig = "Release";
    config.additionalNotes = "Triggered from full config example";
    config.onCrash = [] {
        std::cout << "Performing cleanup before crash...\n";
    };
    config.showCrashDialog = true;

    CrashCatch::initialize(config);

    int* ptr = nullptr;
    *ptr = 42; // Simulated crash
    return 0;
}