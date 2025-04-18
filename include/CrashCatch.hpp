#pragma once

/*
CrashCatch - A simple cross-platform crash handler
Version 1.2.0
Created by Keith Pottratz
Email: keithpotz@gmail.com
License: MIT
*/

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <filesystem>
#include <functional>

#if defined(_WIN32)
#define CRASHCATCH_PLATFORM_WINDOWS
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib") //Auto-link debugging support library
#elif defined(__linux__)
#define CRASHCATCH_PLATFORM_LINUX
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <limits.h>
#include <cxxabi.h>
#include <string.h>
#endif

namespace CrashCatch {

    // Context data passed to crash callback (onCrash, onCrashUpload)
    struct CrashContext {
        std::string dumpFilePath = "";  // .dmp (Windows) or blank (Linux)
        std::string logFilePath = "";   // .txt summary log
        std::string timestamp = "";     // Crash timestamp
        int signalOrCode = 0;           // Signal or exception code
    };

    // Configuration structure for CrashCatch behavior
    struct Config {
        std::string dumpFolder = "./crash_dumps/";   // Where to save crash files
        std::string dumpFileName = "crash";          // Base name (timestamp added optionally)
        bool enableTextLog = true;                   // Output .txt human-readable crash report
        bool autoTimestamp = true;                   // Auto-append timestamp to filenames
        bool showCrashDialog = false;                // (Windows only) Show MessageBox on crash
        std::function<void(const CrashContext&)> onCrash = nullptr;        // Called on crash (log before exit)
        std::function<void(const CrashContext&)> onCrashUpload = nullptr;  // Optional hook to upload crash report
        std::string appVersion = "unknown";          // Application version string
        std::string buildConfig =
#ifdef _DEBUG
            "Debug";
#else
            "Release";
#endif
        std::string additionalNotes = "";            // Optional notes in crash log
    };

    inline Config globalConfig; // Global configuration

    // Generate timestamp string (YYYY-MM-DD_HH-MM-SS)
    inline std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");
        return ss.str();
    }

    // Return full path to current executable
    inline std::string getExecutablePath() {
#ifdef CRASHCATCH_PLATFORM_WINDOWS
        char buffer[MAX_PATH];
        GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        return std::string(buffer);
#elif defined(CRASHCATCH_PLATFORM_LINUX)
        char path[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (len != -1) {
            path[len] = '\0';
            return std::string(path);
        }
        return "(unknown)";
#endif
    }

#ifdef CRASHCATCH_PLATFORM_LINUX
    // Demangle C++ symbol names from backtrace
    inline std::string demangle(const char* symbol) {
        size_t size;
        int status;
        char* demangled = abi::__cxa_demangle(symbol, nullptr, &size, &status);
        std::string result = (status == 0) ? demangled : symbol;
        free(demangled);
        return result;
    }
#endif

    // Collect system/app info for inclusion in crash logs
    inline std::string getDiagnosticsInfo() {
        std::stringstream ss;
        ss << "App Version: " << globalConfig.appVersion << "\n";
        ss << "Build Config: " << globalConfig.buildConfig << "\n";
#ifdef CRASHCATCH_PLATFORM_WINDOWS
        ss << "Platform: Windows\n";
#elif defined(CRASHCATCH_PLATFORM_LINUX)
        ss << "Platform: Linux\n";
#endif
        ss << "Executable: " << getExecutablePath() << "\n";
        if (!globalConfig.additionalNotes.empty()) {
            ss << "Notes: " << globalConfig.additionalNotes << "\n";
        }
        return ss.str();
    }

    // Write human-readable crash report to .txt file
    inline void writeCrashLog(const std::string& logPath, const std::string& timestamp, int signal = 0) {
        std::ofstream log(logPath);
        if (!log.is_open()) return;

        log << "Crash Report\n============\n";

#ifdef CRASHCATCH_PLATFORM_LINUX
        // Stack trace for Linux
        log << "Signal: " << strsignal(signal) << " (" << signal << ")\n";
#endif
        log << "Timestamp: " << (timestamp.empty() ? "N/A" : timestamp) << "\n\n";
        log << "Environment Info:\n" << getDiagnosticsInfo() << "\n";

#ifdef CRASHCATCH_PLATFORM_LINUX
        // Output stack trace
        void* callstack[128];
        int frames = backtrace(callstack, 128);
        char** symbols = backtrace_symbols(callstack, frames);
        log << "\nStack Trace:\n";
        for (int i = 0; i < frames; ++i) {
            log << "  [" << i << "]: " << demangle(symbols[i]) << "\n";
        }
        free(symbols);
#endif

        log.close();
    }

#ifdef CRASHCATCH_PLATFORM_WINDOWS
    // Windows unhandled exception handler
    inline LONG WINAPI UnhandledExceptionHandler(EXCEPTION_POINTERS* ep) {
        DWORD code = ep->ExceptionRecord->ExceptionCode;
        if (code == DBG_PRINTEXCEPTION_C || code == DBG_CONTROL_C) {
            return EXCEPTION_CONTINUE_SEARCH;
        }

        std::string timestamp = globalConfig.autoTimestamp ? getTimestamp() : "";
        std::string base = globalConfig.dumpFileName + (timestamp.empty() ? "" : ("_" + timestamp));
        std::string dumpPath = globalConfig.dumpFolder + base + ".dmp";
        std::string logPath = globalConfig.dumpFolder + base + ".txt";

        std::filesystem::create_directories(globalConfig.dumpFolder);

        if (globalConfig.onCrash) {
            CrashContext context{ dumpPath, logPath, timestamp, static_cast<int>(code) };
            globalConfig.onCrash(context);
        }

        HANDLE hFile = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION dumpInfo = { GetCurrentThreadId(), ep, FALSE };
            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, &dumpInfo, nullptr, nullptr);
            CloseHandle(hFile);

            if (globalConfig.enableTextLog) {
                writeCrashLog(logPath, timestamp);
            }

            if (globalConfig.showCrashDialog) {
                std::string msg = "Crash occurred. Dump written to:\n" + dumpPath;
                MessageBoxA(nullptr, msg.c_str(), "Crash Detected", MB_OK | MB_ICONERROR);
            }
        }

        if (globalConfig.onCrashUpload) {
            CrashContext context{ dumpPath, logPath, timestamp, static_cast<int>(code) };
            globalConfig.onCrashUpload(context);
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }
#endif

#ifdef CRASHCATCH_PLATFORM_LINUX
    // POSIX signal handler (Linux only)
    inline void linuxSignalHandler(int signum) {
        std::string timestamp = globalConfig.autoTimestamp ? getTimestamp() : "";
        std::string base = globalConfig.dumpFileName + (timestamp.empty() ? "" : ("_" + timestamp));
        std::string logPath = globalConfig.dumpFolder + base + ".txt";

        std::filesystem::create_directories(globalConfig.dumpFolder);

        if (globalConfig.onCrash) {
            CrashContext context{ "", logPath, timestamp, signum };
            globalConfig.onCrash(context);
        }

        writeCrashLog(logPath, timestamp, signum);

        if (globalConfig.onCrashUpload) {
            CrashContext context{ "", logPath, timestamp, signum };
            globalConfig.onCrashUpload(context);
        }

        _exit(1);
    }
#endif

    // Initialize CrashCatch with user configuration
    inline bool initialize(const Config& config = Config()) {
        globalConfig = config;
#ifdef CRASHCATCH_PLATFORM_WINDOWS
        SetUnhandledExceptionFilter(UnhandledExceptionHandler);
#elif defined(CRASHCATCH_PLATFORM_LINUX)
        signal(SIGSEGV, linuxSignalHandler);
        signal(SIGABRT, linuxSignalHandler);
        signal(SIGFPE, linuxSignalHandler);
        signal(SIGILL, linuxSignalHandler);
        signal(SIGBUS, linuxSignalHandler);
#endif
        return true;
    }

    // Shorthand: use default configuration
    inline bool enable() { return initialize(Config{}); }

    // Auto-initialize when included (optional)
#ifdef CRASHCATCH_AUTO_INIT
    namespace {
        const bool _autoInit = CrashCatch::enable();
    }
#endif

} // namespace CrashCatch
