# Changelog

All notable changes to this project will be documented in this file.

---

## [1.4.0] - 2026-03-20

### Fixed

- **Windows stack trace pointed at the handler, not the crash site**  `writeCrashLog` previously called `RtlCaptureContext` from inside the exception handler, capturing the handler's own stack. The actual crash context (`ep->ContextRecord`) is now passed through and used for the stack walk. Frame `[0]` now correctly shows the function that crashed.
- **`onCrash` and `onCrashUpload` fired before files existed**  Both callbacks previously received file paths that had not yet been written to disk, making it impossible to safely read or upload them from within the callback. Callbacks now fire after all files are fully written.
- **`SymInitialize` called inside the crash handler**  Symbol initialization was deferred until crash time, which is slow and can conflict with other DbgHelp users. `SymInitialize` is now called once during `CrashCatch::initialize()` at startup.
- **Linux signal handler was not async-signal-safe**  The handler called heap-allocating functions (`std::string`, `std::ofstream`, `backtrace_symbols`, `abi::__cxa_demangle`) directly from a POSIX signal handler. These are not on the async-signal-safe list and can deadlock or crash again if the heap is corrupt. The handler now uses `fork()`: the child process performs all I/O and callback work; the parent calls `_exit()`. This is the same pattern used by Crashpad and Breakpad.
- **Linux symbol demangling applied to full backtrace string instead of mangled name**  `backtrace_symbols()` returns strings like `./app(_Z3foov+0x10) [0x...]`. Passing the whole string to `__cxa_demangle` always failed. The mangled name is now extracted from between `(` and `+` before demangling, producing readable output like `./app(MyClass::myMethod()+0x10) [0x...]`.
- **`localtime` is not thread-safe** Replaced with `localtime_s` (Windows) and `localtime_r` (Linux), both of which write into a caller-supplied struct rather than returning a pointer to shared static data.
- **`create_directories` result was silently ignored** Failure to create the output directory now causes early exit rather than writing to an invalid path. Uses `std::error_code` overload to avoid exceptions.
- **`CrashContext` was constructed twice per crash** One instance was built for `onCrash` and a second identical one for `onCrashUpload`. A single context object is now built once and shared between both callbacks.
- **Version comment in `CrashCatch.hpp` was out of date** Header comment said `Version 1.2.0`; updated to `1.3.0` and now `1.4.0`.

### Added

- `tests/test_callback_order.cpp` Verifies that the crash log file exists on disk when `onCrash` fires.
- `tests/test_stack_context.cpp` Verifies that the stack trace frame `[0]` is the actual crash-site function, not an internal handler frame.

---

## [1.3.0] - 2025-05-01

### Added

- **Windows stack trace in `.txt` log**  `StackWalk64` + `SymFromAddr` + `SymGetLineFromAddr64` provide function names and file/line numbers directly in the human-readable crash report.
- **`includeStackTrace` config flag**  Set to `false` to suppress stack trace output on both Windows and Linux.
- **`CrashCatchDLL.hpp`**  Plain C interface (`extern "C"`) so C++11, C++98, and C projects can use CrashCatch by linking against a pre-compiled DLL or shared library. Consumers do not need C++17.
- **`crashcatch_version()`**  Returns the version string from the DLL (`"1.3.0"`).
- **`crashcatch_default_config()`** Returns a `CrashCatch_Config` populated with sensible defaults.

---

## [1.2.0] - 2025-04-17

### ✨ Added
- ✅ **Linux Support Finalized** CrashCatch now fully supports Linux crash logging via POSIX signals.
- ✅ **CrashContext Struct**  Provides metadata like timestamp, file paths, and signal/exception codes to `onCrash` and `onCrashUpload`.
- ✅ **`onCrashUpload` Callback**  Optional function to handle post-crash uploads (e.g., to a custom server).
- ✅ **Symbol Resolution on Linux**  Now includes demangled stack traces for improved readability.
- ✅ **Executable Path Detection** Added `getExecutablePath()` for both Windows and Linux platforms.
- ✅ **Linux Support Documentation** New `linux.md` file and documentation updates.
- ✅ **Improved Logging** Crash logs include platform, architecture, and optional diagnostic notes.

### 🔧 Improved
- 🧠 `onCrash` now passes a `CrashContext` object instead of taking no arguments.
- 🛡️ Better signal filtering on Linux (`SIGSEGV`, `SIGABRT`, `SIGFPE`, `SIGILL`, `SIGBUS`).
- 🧰 Code refactored for clarity and maintainability.
- ✅ Added extensive inline comments across the codebase.

### 🐛 Fixed
- 🐧 Addressed compatibility issue on GCC 7.5.0 (OpenSUSE) where missing headers caused build failure.
- 🛑 Resolved crash on dereferencing null pointers on Linux not logging output due to missing signal handler.

### 📚 Docs
- 📝 Updated README with new examples (`onCrashUpload`, `CrashContext` usage).
- 📷 Screenshots added for examples.
- 📁 Auto-generated `docs/` from `mkdocs-material` now reflects Linux support and all v1.2 changes.

---



---

## [v1.1.0] — 2025-04-04

### 🚀 Added
- **Linux support!** CrashCatch now handles signals like `SIGSEGV`, `SIGABRT`, and `SIGFPE` using POSIX `signal()` and `backtrace()`.
- **Symbol resolution on Linux** using `backtrace_symbols()`.
- **Executable path detection** added for Linux.
- New crash log output format includes platform, architecture, and environment diagnostics.
- Crash logs are now thread-safe and tested across multiple crashing threads.
- Added support for detecting and handling division-by-zero crashes on Linux.
- Improved header detection for use in both CLI and GUI applications.

### 🐛 Fixed
- **Resolved a bug on Linux** where `CrashCatch.hpp` failed to compile due to unconditional inclusion of `<Windows.h>`.
  - CrashCatch is now fully platform-aware and conditionally includes headers.
  - Thanks to the community for reporting the issue on **OpenSUSE 7.5.0**.
  
### ♻️ Changed
- `CrashCatch.hpp` refactored to reduce OS-specific overhead.
- Platform-specific logic is conditionally compiled for cleaner cross-platform compatibility.
- Crash log structure unified across platforms for consistency.

### ✅ Compatibility
- Windows: Tested on Windows 10/11 (x64)
- Linux: Tested on Ubuntu 22.04 (x64) and WSL2

---

