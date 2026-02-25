#pragma once

/*
CrashCatchDLL - C interface wrapper for CrashCatch
Version 1.3.0
Created by Keith Pottratz
License: MIT

─────────────────────────────────────────────────────────────────────────────
PURPOSE
─────────────────────────────────────────────────────────────────────────────
CrashCatch.hpp requires C++17. This file provides a plain C interface so that
programs built with C++11, C++98, or plain C can use CrashCatch by linking
against a pre-compiled DLL (Windows) or shared library (Linux).

The DLL itself is compiled once with C++17. Consumers only need to include
this header and link against the compiled binary — no C++17 required on
their end.

─────────────────────────────────────────────────────────────────────────────
BUILDING THE DLL
─────────────────────────────────────────────────────────────────────────────
Create a single .cpp file in your DLL project:

    // CrashCatchDLL.cpp
    #define CRASHCATCH_DLL_EXPORTS
    #include "CrashCatchDLL.hpp"

Compile with C++17. This produces:
    Windows : CrashCatch.dll + CrashCatch.lib
    Linux   : libCrashCatch.so

─────────────────────────────────────────────────────────────────────────────
USING THE DLL (C++11 / C++98 / C consumer)
─────────────────────────────────────────────────────────────────────────────
    #include "CrashCatchDLL.hpp"

    int main() {
        crashcatch_enable();        // zero-config
        // -- or --
        CrashCatch_Config cfg = crashcatch_default_config();
        cfg.show_crash_dialog = 1;
        cfg.include_stack_trace = 1;
        crashcatch_init(&cfg);
    }
─────────────────────────────────────────────────────────────────────────────
*/

// ── Export / import macros ────────────────────────────────────────────────
#if defined(_WIN32)
  #if defined(CRASHCATCH_DLL_EXPORTS)
    #define CRASHCATCH_API __declspec(dllexport)
  #else
    #define CRASHCATCH_API __declspec(dllimport)
  #endif
#else
  // GCC/Clang visibility
  #define CRASHCATCH_API __attribute__((visibility("default")))
#endif

// ── C linkage — no name mangling, usable from C++98 / C++11 / C ──────────
#ifdef __cplusplus
extern "C" {
#endif

// ── Plain-C configuration struct ─────────────────────────────────────────
// Mirrors CrashCatch::Config but uses only C-compatible types.
typedef struct {
    const char* dump_folder;        // Directory to write crash files (default: "./crash_dumps/")
    const char* dump_file_name;     // Base filename (default: "crash")
    int         enable_text_log;    // Write .txt crash report (1 = yes, 0 = no)
    int         auto_timestamp;     // Append timestamp to filename (1 = yes)
    int         show_crash_dialog;  // Windows only: show MessageBox on crash
    int         include_stack_trace;// Write stack trace to .txt log (1 = yes)
    const char* app_version;        // Application version string
    const char* build_config;       // "Debug" or "Release"
    const char* additional_notes;   // Optional notes included in crash log

    // Optional callbacks — set to NULL if not needed.
    // context_json: a JSON-formatted string of crash context fields.
    void (*on_crash)(const char* dump_path, const char* log_path,
                     const char* timestamp, int signal_or_code);
    void (*on_crash_upload)(const char* dump_path, const char* log_path,
                            const char* timestamp, int signal_or_code);
} CrashCatch_Config;

// ── API ───────────────────────────────────────────────────────────────────

/**
 * Returns a CrashCatch_Config populated with sensible defaults.
 * Always call this first, then override only the fields you need.
 *
 * Example:
 *   CrashCatch_Config cfg = crashcatch_default_config();
 *   cfg.app_version = "2.0.0";
 *   crashcatch_init(&cfg);
 */
CRASHCATCH_API CrashCatch_Config crashcatch_default_config(void);

/**
 * Initialize CrashCatch with a configuration struct.
 * Returns 1 on success, 0 on failure.
 *
 * Example:
 *   CrashCatch_Config cfg = crashcatch_default_config();
 *   cfg.show_crash_dialog = 1;
 *   crashcatch_init(&cfg);
 */
CRASHCATCH_API int crashcatch_init(const CrashCatch_Config* config);

/**
 * Initialize CrashCatch with all defaults. Equivalent to CrashCatch::enable().
 * Returns 1 on success, 0 on failure.
 *
 * Example:
 *   crashcatch_enable();
 */
CRASHCATCH_API int crashcatch_enable(void);

/**
 * Returns the CrashCatch version string, e.g. "1.3.0".
 */
CRASHCATCH_API const char* crashcatch_version(void);

#ifdef __cplusplus
} // extern "C"
#endif


// ── Implementation (compiled into the DLL only) ───────────────────────────
// This block is only compiled when building the DLL itself.
// Consumers who only #include this header never see the C++17 code.
#if defined(CRASHCATCH_DLL_EXPORTS)

#include "CrashCatch.hpp"   // C++17 — only compiled inside the DLL project
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

CRASHCATCH_API CrashCatch_Config crashcatch_default_config(void) {
    CrashCatch_Config cfg;
    cfg.dump_folder         = "./crash_dumps/";
    cfg.dump_file_name      = "crash";
    cfg.enable_text_log     = 1;
    cfg.auto_timestamp      = 1;
    cfg.show_crash_dialog   = 0;
    cfg.include_stack_trace = 1;
    cfg.app_version         = "unknown";
    cfg.build_config        = "Release";
    cfg.additional_notes    = "";
    cfg.on_crash            = nullptr;
    cfg.on_crash_upload     = nullptr;
    return cfg;
}

CRASHCATCH_API int crashcatch_init(const CrashCatch_Config* cfg) {
    if (!cfg) return 0;

    CrashCatch::Config cppConfig;

    if (cfg->dump_folder)       cppConfig.dumpFolder        = cfg->dump_folder;
    if (cfg->dump_file_name)    cppConfig.dumpFileName      = cfg->dump_file_name;
    if (cfg->app_version)       cppConfig.appVersion        = cfg->app_version;
    if (cfg->build_config)      cppConfig.buildConfig       = cfg->build_config;
    if (cfg->additional_notes)  cppConfig.additionalNotes   = cfg->additional_notes;

    cppConfig.enableTextLog     = cfg->enable_text_log     != 0;
    cppConfig.autoTimestamp     = cfg->auto_timestamp      != 0;
    cppConfig.showCrashDialog   = cfg->show_crash_dialog   != 0;
    cppConfig.includeStackTrace = cfg->include_stack_trace != 0;

    // Wrap C callbacks into C++ std::function lambdas
    if (cfg->on_crash) {
        auto cb = cfg->on_crash; // capture raw pointer — safe, DLL lifetime
        cppConfig.onCrash = [cb](const CrashCatch::CrashContext& ctx) {
            cb(ctx.dumpFilePath.c_str(),
               ctx.logFilePath.c_str(),
               ctx.timestamp.c_str(),
               ctx.signalOrCode);
        };
    }

    if (cfg->on_crash_upload) {
        auto cb = cfg->on_crash_upload;
        cppConfig.onCrashUpload = [cb](const CrashCatch::CrashContext& ctx) {
            cb(ctx.dumpFilePath.c_str(),
               ctx.logFilePath.c_str(),
               ctx.timestamp.c_str(),
               ctx.signalOrCode);
        };
    }

    return CrashCatch::initialize(cppConfig) ? 1 : 0;
}

CRASHCATCH_API int crashcatch_enable(void) {
    return CrashCatch::enable() ? 1 : 0;
}

CRASHCATCH_API const char* crashcatch_version(void) {
    return "1.3.0";
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CRASHCATCH_DLL_EXPORTS
