#include "CrashCatch.hpp"
#include <iostream>
#include <fstream>

// Simulated upload function
void simulateUpload(const std::string& path) {
    std::cout << "[Uploader] Simulating upload of file: " << path << std::endl;
    std::ifstream file(path);
    if (file.is_open()) {
        std::cout << "[Uploader] File content (first 3 lines):\n";
        std::string line;
        int count = 0;
        while (std::getline(file, line) && count++ < 3) {
            std::cout << "  " << line << "\n";
        }
        file.close();
    } else {
        std::cerr << "[Uploader] Failed to open file: " << path << std::endl;
    }
}

int main() {
    CrashCatch::Config config;
    config.appVersion = "1.2.0";
    config.additionalNotes = "Simulated upload test";

    config.onCrashUpload = [](const CrashCatch::CrashContext& ctx) {
        if (!ctx.dumpFilePath.empty()) {
            simulateUpload(ctx.dumpFilePath);
        }
        if (!ctx.logFilePath.empty()) {
            simulateUpload(ctx.logFilePath);
        }
    };

    CrashCatch::initialize(config);

    // Trigger a crash
    int* ptr = nullptr;
    *ptr = 42;

    return 0;
}
