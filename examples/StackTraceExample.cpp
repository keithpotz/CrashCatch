/*
 * CrashCatch — StackTraceExample
 *
 * Demonstrates the includeStackTrace config option added in v1.3.0.
 *
 * Run with CRASHCATCH_SHOW_STACK=1 to see stack trace in the .txt log,
 * or CRASHCATCH_SHOW_STACK=0 to suppress it.
 *
 * Build:
 *   Windows : cl /std:c++17 /EHsc StackTraceExample.cpp
 *   Linux   : g++ -std=c++17 -g -rdynamic -o StackTraceExample StackTraceExample.cpp
 */

#include <iostream>
#include "CrashCatch.hpp"

int main() {
    CrashCatch::Config config;
    config.appVersion        = "1.3.0";
    config.buildConfig       = "Release";
    config.additionalNotes   = "StackTrace example — v1.3.0";
    config.dumpFolder        = "./crash_dumps/";

    // ── NEW in v1.3.0 ────────────────────────────────────────────────────
    // Set to true  → stack trace is written to the .txt crash log
    // Set to false → .txt log contains environment info only (no trace)
    // Defaults to true — existing code is unaffected.
    config.includeStackTrace = true;
    // ─────────────────────────────────────────────────────────────────────

    config.onCrash = [](const CrashCatch::CrashContext& ctx) {
        std::cout << "\n[CrashCatch] Crash captured!\n";
        std::cout << "  Timestamp : " << ctx.timestamp << "\n";
        std::cout << "  Log file  : " << ctx.logFilePath << "\n";
#ifdef CRASHCATCH_PLATFORM_WINDOWS
        std::cout << "  Dump file : " << ctx.dumpFilePath << "\n";
#endif
        std::cout << "  Signal    : " << ctx.signalOrCode << "\n";
    };

    CrashCatch::initialize(config);

    std::cout << "CrashCatch v1.3.0 — StackTrace Example\n";
    std::cout << "includeStackTrace = " << (config.includeStackTrace ? "true" : "false") << "\n";
    std::cout << "Triggering null pointer dereference...\n\n";

    // Trigger crash — CrashCatch catches this and writes the report
    int* ptr = nullptr;
    *ptr = 42;

    return 0;
}
