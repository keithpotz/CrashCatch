#pragma once

/*
CrashCatch - A simple cross-platform crash handler
Version 1.4.0
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
#include <windows.h>
#include <dbgHelp.h>
#pragma comment(lib, "dbgHelp.lib") //Auto-link debugging support library
#elif defined(__linux__)
#define CRASHCATCH_PLATFORM_LINUX
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <limits.h>
#include <cxxabi.h>
#include <string.h>
#include <sys/wait.h>
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
        bool includeStackTrace = true;               // Output stack trace in .txt log (Windows + Linux)
    };

    inline Config globalConfig; // Global configuration

#ifdef CRASHCATCH_PLATFORM_WINDOWS
    // Set by UnhandledExceptionHandler so writeCrashLog can walk the crash-site stack,
    // not the handler's own stack. Only valid during crash handling.
    inline CONTEXT* g_crashSiteContext = nullptr;
#endif

    // Generate timestamp string (YYYY-MM-DD_HH-MM-SS)
    inline std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm_result = {};
#ifdef CRASHCATCH_PLATFORM_WINDOWS
        localtime_s(&tm_result, &time);  // thread-safe on Windows
#else
        localtime_r(&time, &tm_result);  // thread-safe on POSIX
#endif
        std::stringstream ss;
        ss << std::put_time(&tm_result, "%Y-%m-%d_%H-%M-%S");
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
    // Demangle a C++ symbol name.
    // backtrace_symbols() returns strings like "./app(_Z3foov+0x10) [0x7f...]".
    // We extract just the mangled name between '(' and '+' before demangling.
    inline std::string demangle(const char* symbol) {
        std::string sym(symbol);

        // Extract mangled name: between '(' and '+'
        auto parenOpen  = sym.find('(');
        auto plusSign   = sym.find('+', parenOpen);
        if (parenOpen != std::string::npos && plusSign != std::string::npos && plusSign > parenOpen + 1) {
            std::string mangled = sym.substr(parenOpen + 1, plusSign - parenOpen - 1);
            size_t size = 0;
            int status  = 0;
            char* demangled = abi::__cxa_demangle(mangled.c_str(), nullptr, &size, &status);
            if (status == 0 && demangled) {
                std::string result = sym.substr(0, parenOpen + 1)
                                   + demangled
                                   + sym.substr(plusSign);
                free(demangled);
                return result;
            }
            if (demangled) free(demangled);
        }
        return sym; // return original if no mangled segment found or demangle failed
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
        std::error_code ec;
        std::filesystem::create_directories(std::filesystem::path(logPath).parent_path(), ec);
        if (ec) return; // can't create output directory — bail out silently

        std::ofstream log(logPath);
        if (!log.is_open()) return;

        log << "Crash Report\n============\n";

#ifdef CRASHCATCH_PLATFORM_LINUX
        log << "Signal: " << strsignal(signal) << " (" << signal << ")\n";
#endif
        log << "Timestamp: " << (timestamp.empty() ? "N/A" : timestamp) << "\n\n";
        log << "Environment Info:\n" << getDiagnosticsInfo() << "\n";

#ifdef CRASHCATCH_PLATFORM_WINDOWS
        if (globalConfig.includeStackTrace) {
            // Walk the stack using DbgHelp (already linked via pragma comment)
            // SymInitialize was called at CrashCatch::initialize() time
            HANDLE process = GetCurrentProcess();
            HANDLE thread  = GetCurrentThread();

            // Use crash-site context if available (set by exception handler),
            // otherwise fall back to capturing here (e.g. called standalone).
            CONTEXT localContext = {};
            CONTEXT& context = g_crashSiteContext ? *g_crashSiteContext : localContext;
            if (!g_crashSiteContext) {
                localContext.ContextFlags = CONTEXT_FULL;
                RtlCaptureContext(&localContext);
            }

            STACKFRAME64 frame = {};
#if defined(_M_X64)
            DWORD machineType       = IMAGE_FILE_MACHINE_AMD64;
            frame.AddrPC.Offset     = context.Rip;
            frame.AddrPC.Mode       = AddrModeFlat;
            frame.AddrFrame.Offset  = context.Rbp;
            frame.AddrFrame.Mode    = AddrModeFlat;
            frame.AddrStack.Offset  = context.Rsp;
            frame.AddrStack.Mode    = AddrModeFlat;
#elif defined(_M_IX86)
            DWORD machineType       = IMAGE_FILE_MACHINE_I386;
            frame.AddrPC.Offset     = context.Eip;
            frame.AddrPC.Mode       = AddrModeFlat;
            frame.AddrFrame.Offset  = context.Ebp;
            frame.AddrFrame.Mode    = AddrModeFlat;
            frame.AddrStack.Offset  = context.Esp;
            frame.AddrStack.Mode    = AddrModeFlat;
#else
            DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
#endif
            // Symbol buffer
            const int MAX_SYM_NAME_LEN = 256;
            char symBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME_LEN * sizeof(char)];
            SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symBuffer);
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen   = MAX_SYM_NAME_LEN;

            log << "\nStack Trace:\n";
            int frameIndex = 0;

            while (StackWalk64(machineType, process, thread, &frame, &context,
                               nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) {
                if (frame.AddrPC.Offset == 0) break;

                log << "  [" << frameIndex++ << "]: ";

                DWORD64 displacement = 0;
                if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol)) {
                    log << symbol->Name;

                    // Try to get file/line info
                    IMAGEHLP_LINE64 line = {};
                    line.SizeOfStruct    = sizeof(IMAGEHLP_LINE64);
                    DWORD lineDisp       = 0;
                    if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &lineDisp, &line)) {
                        log << " (" << line.FileName << ":" << line.LineNumber << ")";
                    }
                } else {
                    // No symbol — fall back to raw address
                    log << "0x" << std::hex << frame.AddrPC.Offset << std::dec;
                }
                log << "\n";

                // Avoid runaway stack walks
                if (frameIndex > 64) break;
            }

        }
#endif

#ifdef CRASHCATCH_PLATFORM_LINUX
        if (globalConfig.includeStackTrace) {
            void* callstack[128];
            int frames = backtrace(callstack, 128);
            char** symbols = backtrace_symbols(callstack, frames);
            log << "\nStack Trace:\n";
            for (int i = 0; i < frames; ++i) {
                log << "  [" << i << "]: " << demangle(symbols[i]) << "\n";
            }
            free(symbols);
        }
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

        std::error_code ec;
        std::filesystem::create_directories(globalConfig.dumpFolder, ec);
        if (ec) return EXCEPTION_EXECUTE_HANDLER; // can't create output directory

        // Point the stack walker at the actual crash site, not the handler frame
        g_crashSiteContext = ep->ContextRecord;

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

        // Build context once, after files are written, so callbacks can access them
        CrashContext context{ dumpPath, logPath, timestamp, static_cast<int>(code) };

        if (globalConfig.onCrash) {
            globalConfig.onCrash(context);
        }

        if (globalConfig.onCrashUpload) {
            globalConfig.onCrashUpload(context);
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }
#endif

#ifdef CRASHCATCH_PLATFORM_LINUX
    // POSIX signal handler (Linux only).
    //
    // Signal handlers must only call async-signal-safe functions (see signal-safety(7)).
    // Heap allocation, std::string, file I/O, and C++ exceptions are NOT safe to call
    // directly from a signal handler.
    //
    // Solution: fork() a child process to do all the heavy work (logging, callbacks).
    // The child inherits the parent's memory image but runs in a clean execution context
    // where malloc locks are not held. The parent simply _exit()s immediately.
    inline void linuxSignalHandler(int signum) {
        // Build paths before fork using only already-constructed std::strings.
        // These copies are safe because we're single-threaded at the point of the crash
        // signal delivery (the faulting thread is the only one executing here).
        std::string timestamp = globalConfig.autoTimestamp ? getTimestamp() : "";
        std::string base = globalConfig.dumpFileName + (timestamp.empty() ? "" : ("_" + timestamp));
        std::string logPath = globalConfig.dumpFolder + base + ".txt";

        pid_t pid = fork();
        if (pid == 0) {
            // Child process: safe to use heap, file I/O, std::string, etc.
            writeCrashLog(logPath, timestamp, signum);

            // Build context once, after file is written, so callbacks can access it
            CrashContext context{ "", logPath, timestamp, signum };

            if (globalConfig.onCrash) {
                globalConfig.onCrash(context);
            }

            if (globalConfig.onCrashUpload) {
                globalConfig.onCrashUpload(context);
            }

            _exit(0);
        } else if (pid > 0) {
            // Parent: wait for child to finish writing the report, then exit
            waitpid(pid, nullptr, 0);
        }
        // pid < 0 means fork failed — fall through and exit anyway
        _exit(1);
    }
#endif

    // Initialize CrashCatch with user configuration
    inline bool initialize(const Config& config = Config()) {
        globalConfig = config;
#ifdef CRASHCATCH_PLATFORM_WINDOWS
        // Load symbols now so they're ready when a crash occurs
        SymInitialize(GetCurrentProcess(), nullptr, TRUE);
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

