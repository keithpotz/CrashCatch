#pragma once

/*
CrashCatch - A simple cross-platform crash handler
Version 1.6.0
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
#include <vector>
#include <cstdio>

#if defined(_WIN32)
#define CRASHCATCH_PLATFORM_WINDOWS
#include <windows.h>
#include <dbgHelp.h>
#pragma comment(lib, "dbgHelp.lib") //Auto-link debugging support library
#elif defined(__linux__)
#define CRASHCATCH_PLATFORM_LINUX
#elif defined(__APPLE__)
#define CRASHCATCH_PLATFORM_MACOS
#include <mach-o/dyld.h> // For _NSGetExecutablePath
#endif

// Common POSIX include across both macOS and Linux.
#if defined(CRASHCATCH_PLATFORM_LINUX) || defined(CRASHCATCH_PLATFORM_MACOS)
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
        std::filesystem::path dumpFilePath;  // .dmp (Windows) or blank (Linux)
        std::filesystem::path logFilePath;   // .txt summary log
        std::string timestamp;          // Crash timestamp
        int signalOrCode = 0;           // Signal or exception code
    };

    // Configuration structure for CrashCatch behavior
    struct Config {
        std::filesystem::path dumpFolder = "./crash_dumps/";   // Where to save crash files
        std::string dumpFileName = "crash";          // Base name (timestamp added optionally)
        bool enableTextLog = true;                   // Output .txt human-readable crash report
        bool autoTimestamp = true;                   // Auto-append timestamp to filenames
        bool showCrashDialog = false;                // (Windows only) Show MessageBox on crash
        std::function<void(const CrashContext&)> onCrash = nullptr;        // Called on crash (log before exit)
        std::function<void(const CrashContext&)> onCrashUpload = nullptr;  // Optional hook to upload crash report
        std::string appVersion = "unknown";          // Application version string
        std::string buildConfig =
        // NDEBUG is the way to go due to portability, its part of the 
        // C/C++ standard (controls assert()) and CMake defines/undefines it consistently
        // across MSVC, Clang, and GCC based on the CMAKE_BUILD_TYPE. _DEBUG by contrast is 
        // MSVC/CRT specific and won't reflect the real build type on other compilers.
#if !defined(NDEBUG) 
            "Debug";
#else
            "Release";
#endif
        std::string additionalNotes;                 // Optional notes in crash log
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

#elif defined(CRASHCATCH_PLATFORM_MACOS)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size); // first call: returns -1, but writes the required buffer size into 'size'

        std::vector<char> buffer(size);
        if (_NSGetExecutablePath(buffer.data(), &size) != 0){
            return "(unknown)"; // still failed even with the correct size
        }
        return std::string(buffer.data());
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
#elif defined(CRASHCATCH_PLATFORM_MACOS)
    //
    // macOS symbol names from backtrace() are very long.
    // For example look at this:
    // symbol = 3   My Module                                0x0000000196776b98 start + 6076
    // If we break it down we get:
    //          Frame  Module                                Address            Name    Offset
    // In this case the function name is coming from C so it's not managled.
    //
    // The code below parses that into it's components, and unfortunately it's not so easy as it
    // is on Linux, macOS doesn't really give us a nice structured symbol to parse as on Linux
    // they can look like "./app(_Z3foov+0x10) [0x7f...]" and we and find the module name by loo-
    // king what comes after the "/" until the "(". 
    // But on macOS we parse the symbol backwards and work our way towards the module name, 
    // which is needed because the module name may contain spaces.
    //
    inline std::string demangle(const char* symbol) {
        std::string workingSymbol(symbol);

        std::string module;
        std::string offset;

        // Helper function to trim the right-hand side of the symbol.
        const auto trimRight = [&](std::string& s)
        {
            while(!s.empty() && std::isspace((unsigned char) s.back()))
                s.pop_back();
        };

        const auto trimLeft = [&](std::string& s)
        {
            const auto pos = s.find_first_not_of(" \t");
            if (pos == std::string::npos)
                s.clear();
            else
                s.erase(0, pos);
        };

        trimRight(workingSymbol);

        // Parse offset, the offset is next to the '+' character.
        const auto plus = workingSymbol.rfind('+');
        if(plus == std::string::npos)
            return workingSymbol;

        offset = workingSymbol.substr(plus);

        // Now, we'll remove the entire offset from the string...
        workingSymbol = workingSymbol.substr(0, plus);
        // including the space...
        trimRight(workingSymbol);

        // Parse function name.
        std::string symbolStr;
        auto lastSpace = workingSymbol.find_last_of(" \t");
        // If we cannot find the space, then we'll just return the original symbol.
        if(lastSpace == std::string::npos)
            return std::string(symbol);

        symbolStr = std::string(workingSymbol.substr(lastSpace + 1));

        // Now, demangle:
        int status = 0;
        size_t size = 0;
        char* demangled = abi::__cxa_demangle(symbolStr.c_str(), nullptr, &size, &status);

        // Demangling was successful...
        if(status == 0)
            symbolStr = std::string(demangled);

        if(demangled)
            free(demangled);

        // Remove from working symbol.
        workingSymbol = workingSymbol.substr(0, lastSpace);
        // Remove trailing space on the right.
        trimRight(workingSymbol);
        
        // Parse address, we don't care about it but we still need to remove it.
        lastSpace = workingSymbol.find_last_of(" \t");
        workingSymbol = workingSymbol.substr(0, lastSpace);
        trimRight(workingSymbol);

        // Frame number position, we don't care about that.
        const auto frameNumberPosition = workingSymbol.find_first_of(" \t");

        module = workingSymbol.substr(frameNumberPosition);
        while(!module.empty() && std::isspace((unsigned char) module.front()))
            trimLeft(module);

        std::ostringstream oss;
        oss << module << " " << symbolStr << " " << offset;

        return oss.str();
    }
    
    // Uses 'atos' (Xcode Command Line Tools) to resolve file:line for a batch
    // of return addresses in the current process. Called from the forked child,
    // where heap allocation and process spawning are safe.
    // Requires debug info to be present in the binary stripped/Release builds
    // without symbols will just get empty results, which callers should handle
    // gracefully (fall back to module+symbol+offset only). 
    inline std::vector<std::string> resolveFileLines(const std::vector<void*>& addresses){
        std::vector<std::string> results(addresses.size());
        if (addresses.empty()) return results;

        std::ostringstream cmd;
        cmd << "atos -p" << getpid();
        for (auto addr: addresses){
            cmd << " " << addr;
        }

        FILE* pipe = popen(cmd.str().c_str(), "r");
        if (!pipe) return results;

        char lineBuf[1024];
        size_t idx = 0;
        while (idx < results.size() && fgets(lineBuf, sizeof(lineBuf), pipe)){
            std::string line(lineBuf);
            auto lastOpen = line.rfind('(');
            auto lastClose = line.rfind(')');
            if (lastOpen != std::string::npos && lastClose != std::string::npos && lastClose > lastOpen){
                std::string inner = line.substr(lastOpen + 1, lastClose - lastOpen - 1);
                // atos gives "(file.cpp:42)" when debug info is present,
                // or "(in ModuleName)" when it isn't only keep the former.
                if (inner.find(':') != std::string::npos && inner.rfind("in ", 0) != 0){
                    results[idx] = inner;
                }
            }
            ++idx;
        }
        pclose(pipe);
        return results;
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
#elif defined(CRASHCATCH_PLATFORM_MACOS)
        ss << "Platform: macOS\n";
#endif
        ss << "Executable: " << getExecutablePath() << "\n";
        if (!globalConfig.additionalNotes.empty()) {
            ss << "Notes: " << globalConfig.additionalNotes << "\n";
        }
        return ss.str();
    }

    // Write human-readable crash report to .txt file
    inline void writeCrashLog(const std::filesystem::path& logPath, const std::string& timestamp, int signal = 0) {
        std::error_code ec;
        std::filesystem::create_directories(logPath.parent_path(), ec);
        if (ec) return; // can't create output directory — bail out silently

        std::ofstream log(logPath);
        if (!log.is_open()) return;

        log << "Crash Report\n============\n";

#ifdef CRASHCATCH_PLATFORM_LINUX
        log << "Signal: " << strsignal(signal) << " (" << signal << ")\n";
#elif defined(CRASHCATCH_PLATFORM_MACOS)
        // On macOS strsignal() can produce an output like:
        // Segmentation fault: 11
        // So on macOS it doesn't make sense to include the signal code when it's
        // already present.
        log << "Signal: " << strsignal(signal) << "\n";
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

#if defined(CRASHCATCH_PLATFORM_LINUX) || defined(CRASHCATCH_PLATFORM_MACOS)
        if (globalConfig.includeStackTrace) {
            void* callstack[128];
            int frames = backtrace(callstack, 128);
            char** symbols = backtrace_symbols(callstack, frames);
            log << "\nStack Trace:\n";
#ifdef CRASHCATCH_PLATFORM_MACOS
            //Batch-resolve file:line for every frame in one atos call, rather 
            // than shelling out once per fram.
            std::vector<void*> addrVec(callstack, callstack + frames);
            std::vector<std::string> fileLines = resolveFileLines(addrVec);
#endif

            for (int i = 0; i < frames; ++i) {
                log << "  [" << i << "]: " << demangle(symbols[i]);
#ifdef CRASHCATCH_PLATFORM_MACOS
                if (!fileLines[i].empty()){
                    log << " (" << fileLines[i] << ")";
                }
#endif        
                log << "\n";
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

        const std::string timestamp = globalConfig.autoTimestamp ? getTimestamp() : "";
        const std::string base = globalConfig.dumpFileName + (timestamp.empty() ? "" : ("_" + timestamp));
     
        const std::filesystem::path dumpPath = globalConfig.dumpFolder / ( base + ".dmp" );
        const std::filesystem::path logPath = globalConfig.dumpFolder / ( base + ".txt" );

        const std::wstring dumpFilepathStr = dumpPath.wstring();

        std::error_code ec;
        std::filesystem::create_directories(globalConfig.dumpFolder, ec);
        if (ec) return EXCEPTION_EXECUTE_HANDLER; // can't create output directory

        // Point the stack walker at the actual crash site, not the handler frame
        g_crashSiteContext = ep->ContextRecord;

        HANDLE hFile = CreateFileW(dumpFilepathStr.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION dumpInfo = { GetCurrentThreadId(), ep, FALSE };
            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, &dumpInfo, nullptr, nullptr);
            CloseHandle(hFile);

            if (globalConfig.enableTextLog) {
                writeCrashLog(logPath, timestamp);
            }

            if (globalConfig.showCrashDialog) {
                std::wstring msg = L"Crash occurred. Dump written to:\n" + dumpFilepathStr;
                MessageBoxW(nullptr, msg.c_str(), L"Crash Detected", MB_OK | MB_ICONERROR);
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

#if defined(CRASHCATCH_PLATFORM_LINUX) || defined(CRASHCATCH_PLATFORM_MACOS)
    // POSIX signal handler.
    //
    // Signal handlers must only call async-signal-safe functions (see signal-safety(7)).
    // Heap allocation, std::string, file I/O, and C++ exceptions are NOT safe to call
    // directly from a signal handler.
    //
    // Solution: fork() a child process to do all the heavy work (logging, callbacks).
    // The child inherits the parent's memory image but runs in a clean execution context
    // where malloc locks are not held. The parent simply _exit()s immediately.
    inline void posixSignalHandler(int signum) {
        // Build paths before fork using only already-constructed std::strings.
        // These copies are safe because we're single-threaded at the point of the crash
        // signal delivery (the faulting thread is the only one executing here).
        const std::string timestamp = globalConfig.autoTimestamp ? getTimestamp() : "";
        const std::string base = globalConfig.dumpFileName + (timestamp.empty() ? "" : ("_" + timestamp));
        
        const std::filesystem::path logPath = globalConfig.dumpFolder / ( base + ".txt" );

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
#elif defined(CRASHCATCH_PLATFORM_LINUX) || defined(CRASHCATCH_PLATFORM_MACOS)
        signal(SIGSEGV, posixSignalHandler);
        signal(SIGABRT, posixSignalHandler);
        signal(SIGFPE,  posixSignalHandler);
        signal(SIGILL,  posixSignalHandler);
        signal(SIGBUS,  posixSignalHandler);
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
