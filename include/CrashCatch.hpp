/*
CrashCatch - A simple C++ crash handler for Windows
Version 1.0
Created by Keith Pottratz
Email: keithpotz@gmail.com
Copywrite (c) 2025 Keith Pottratz
License: MIT
*/



#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <functional>

#pragma comment(lib, "DbgHelp.lib")  //Automatically lin DbgHelp.lib

namespace CrashCatch {
    //Configuration structure for crash handling behavior
    struct Config {
        std::string dumpFolder = "./crash_dumps/";  //Where to write .dmp and .txt files
		std::string dumpFileName = "crash";         //Base name for Crash Files (timestamp will be appended)
		bool enableTextLog = true;                  //Write a .txt file with crash info
		bool autoTimestamp = true;  		        //Append timestamp to crash files 
		bool showCrashDialog = false;              //Show a message box when a crash occurs 
		std::function<void()> onCrash = nullptr;	//Callback function to run when a crash occurs

        //  Self-diagnostics
        std::string appVersion = "unknown";  //Set your app version
        std::string buildConfig =
#ifdef _DEBUG
            "Debug";
#else
            "Release";
#endif
		std::string additionalNotes = ""; //Additional notes to include in the crash log
    };

    inline Config globalConfig;  //Internal global config used by the handler

    // Generate current local timestamp (formatted)
    inline std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");
        return ss.str();
    }
    // Resolve an address to a symbol name (function + offset)
    inline std::string getSymbolFromAddress(void* address) {
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
        SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;
        DWORD64 displacement = 0;

        if (SymFromAddr(GetCurrentProcess(), (DWORD64)address, &displacement, symbol)) {
            std::stringstream ss;
            ss << symbol->Name << " + 0x" << std::hex << displacement;
            return ss.str();
        }

        return "(symbol not found)";
    }
    // Get the full path to the executable
    inline std::string getExecutablePath() {
        char buffer[MAX_PATH];
        GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        return std::string(buffer);
    }
    //formate and retrun slef-diagnostic info (version, build, uptime ,etc)
    inline std::string getDiagnosticsInfo() {
        std::stringstream ss;
        ss << "App Version: " << globalConfig.appVersion << "\n";
        ss << "Build Config: " << globalConfig.buildConfig << "\n";
#ifdef _M_X64
        ss << "Architecture: x64\n";
#elif defined(_M_IX86)
        ss << "Architecture: x86\n";
#else
        ss << "Architecture: unknown\n";
#endif
        ss << "Executable: " << getExecutablePath() << "\n";
        ss << "Uptime (s): " << GetTickCount64() / 1000 << "\n";
        if (!globalConfig.additionalNotes.empty()) {
            ss << "Notes: " << globalConfig.additionalNotes << "\n";
        }
        return ss.str();
    }
    // Write human readable .txt crash summary (Strack trace + diagnostics)
    inline void writeCrashLog(const std::string& logPath, const std::string& timestamp, const std::string& dumpPath, EXCEPTION_POINTERS* ep) {
        std::ofstream log(logPath);
        if (!log.is_open()) return;

        log << "Crash Report\n============\n";
        log << "Timestamp: " << (timestamp.empty() ? "N/A" : timestamp) << "\n";
        log << "Dump File: " << dumpPath << "\n";
        log << "Exception Code: 0x" << std::hex << ep->ExceptionRecord->ExceptionCode << "\n";
        log << "Thread ID: " << GetCurrentThreadId() << "\n";

        // Stack trace generation using StackWalk64
        SymInitialize(GetCurrentProcess(), nullptr, TRUE);
        CONTEXT* context = ep->ContextRecord;
        STACKFRAME64 stack = {};
        DWORD machineType = 0;

#ifdef _M_X64
        machineType = IMAGE_FILE_MACHINE_AMD64;
        stack.AddrPC.Offset = context->Rip;
        stack.AddrFrame.Offset = context->Rbp;
        stack.AddrStack.Offset = context->Rsp;
#elif defined(_M_IX86)
        machineType = IMAGE_FILE_MACHINE_I386;
        stack.AddrPC.Offset = context->Eip;
        stack.AddrFrame.Offset = context->Ebp;
        stack.AddrStack.Offset = context->Esp;
#else
        machineType = 0;
#endif

        stack.AddrPC.Mode = AddrModeFlat;
        stack.AddrFrame.Mode = AddrModeFlat;
        stack.AddrStack.Mode = AddrModeFlat;

        log << "\nStack Trace:\n";
        for (int i = 0; i < 10; ++i) {
            if (!StackWalk64(machineType, GetCurrentProcess(), GetCurrentThread(), &stack, context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
                break;
            log << "  [" << i << "]: " << getSymbolFromAddress((void*)stack.AddrPC.Offset) << "\n";
        }
        SymCleanup(GetCurrentProcess());

        // Append Self-diagnostics
        log << "\nEnvironment Info:\n";
        log << getDiagnosticsInfo();

        log.close();
    }
    // Main unhandled excception filter (called on crash)
    inline LONG WINAPI UnhandledExceptionHandler(EXCEPTION_POINTERS* ep) {
        if (globalConfig.onCrash) globalConfig.onCrash();
        std::filesystem::create_directories(globalConfig.dumpFolder);

        std::string timestamp = globalConfig.autoTimestamp ? getTimestamp() : "";
        std::string base = globalConfig.dumpFileName.empty() ? "crash" : globalConfig.dumpFileName;
        std::string fileBase = base + (timestamp.empty() ? "" : ("_" + timestamp));
        std::string dumpPath = globalConfig.dumpFolder + fileBase + ".dmp";

        HANDLE hFile = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION dumpInfo = {
                GetCurrentThreadId(),
                ep,
                FALSE
            };

            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, &dumpInfo, nullptr, nullptr);
            CloseHandle(hFile);
            // Write crash log if enabled
            if (globalConfig.enableTextLog) {
                std::string logPath = globalConfig.dumpFolder + fileBase + ".txt";
                writeCrashLog(logPath, timestamp, dumpPath, ep);
            }
			// Show crash dialog if enabled
            if (globalConfig.showCrashDialog) {
                std::string msg = "The application has crashed.\n\nA crash report has been saved to:\n" + dumpPath;
                MessageBoxA(nullptr, msg.c_str(), "Crash Detected", MB_OK | MB_ICONERROR);
            }
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }
	//inialize the crash handler with default configuration
    inline bool initialize(const Config& config = Config()) {
        globalConfig = config;
        SetUnhandledExceptionFilter(UnhandledExceptionHandler);
        return true;
    }
    // One-liner: enable crash handler with default config
    inline bool enable() {
        return initialize(Config{});
    }
    // Auto-init: enable handler on include if macro is set
#ifdef CRASHCATCH_AUTO_INIT
    namespace {
        const bool _autoInit = CrashCatch::enable();
    }
#endif

} // namespace CrashCatch
