# Changelog

All notable changes to this project will be documented in this file.

---

## [1.6.0] - Unreleased

### Added

- **macOS support** — POSIX signal handling (`SIGSEGV`, `SIGABRT`, `SIGFPE`, `SIGILL`, `SIGBUS`) with the same `fork()`-based handler as Linux, `backtrace()` stack traces with demangled symbols, and optional file/line resolution via `atos` (Xcode Command Line Tools + debug symbols; falls back to `module symbol + offset` without them). Intel and Apple Silicon.
- **`CrashCatch::shutdown()`** — restores the unhandled-exception filter (Windows) or signal handlers (Linux/macOS) that were active before `initialize()`, and releases DbgHelp symbol state via `SymCleanup`. Silences leak reports from tools such as Valgrind. Returns `false` and does nothing if CrashCatch is not initialized, so it is safe to call more than once.
- **`crashcatch_shutdown()`** — C API equivalent in `CrashCatchDLL.hpp`.

### Changed

- **Calling `initialize()` again while already initialized now only updates the configuration** instead of re-installing handlers. Re-installing recorded CrashCatch's own handler as the "previous" one, so `shutdown()` could never restore the application's original handler.
- Internal cleanup (`free`, `pclose`, `CloseHandle`) now uses a small RAII helper (`details::AtScopeExit`) instead of manual calls on each return path.

### Fixed

- **Windows symbol buffer was misaligned** — the `SYMBOL_INFO` buffer was a plain `char` array cast to `SYMBOL_INFO*`, which is undefined behavior. It is now `alignas(SYMBOL_INFO)`.
- **`GetModuleFileNameA` failure was ignored** — the executable path is now reported as `(unknown)` instead of reading an uninitialized buffer.
- **`.dmp` file stayed locked while the crash dialog was open** — the dump handle is now closed immediately after `MiniDumpWriteDump`, so the file can be opened while the `showCrashDialog` message box is displayed.
- **`MessageBoxW` link error in bare builds** — `user32.lib` is now auto-linked alongside `dbgHelp.lib`.
- **macOS: `demangle()` could throw or hang inside the crash handler** on backtrace lines without a module column (`substr(npos)` threw `std::out_of_range`) or with unusual leading whitespace (infinite loop).
- **macOS: `atos` command missing a space** — the command was built as `atos -p<pid>`; it is now `atos -p <pid>`.
- **`crashcatch_version()` returned `"1.5.0"`** — now `"1.6.0"`.

### Credits

- `shutdown()`, RAII cleanup, symbol-buffer alignment, and `GetModuleFileNameA` fixes via PR #17.

---

## [1.5.0] - 2026-07-29

### Changed

- **BREAKING: `Config::dumpFolder` and `CrashContext::dumpFilePath` / `logFilePath` are now `std::filesystem::path` instead of `std::string`.** Code that consumed these fields as strings needs to call `.string()` or accept `const std::filesystem::path&` instead. `examples/Example_UploadCrash.cpp` updated to match.

### Fixed

- **`dumpFolder` required a trailing path separator** — output paths were built via raw string concatenation, so a `dumpFolder` without a trailing `/`/`\` produced a malformed path. Path composition now uses `std::filesystem::path`'s `/` operator.
- **Filenames containing a dot could be truncated** — an intermediate fix used `replace_extension()`, which strips everything after the last dot in the whole path (e.g. `"MyApp.Build1"` → `MyApp.dmp`). Final fix builds the full filename as one string before appending it as a single path component.
- **Non-ASCII crash file paths were mangled on Windows** — `CreateFileA`/`MessageBoxA` replaced with `CreateFileW`/`MessageBoxW`.
- **`CrashCatchDLL.hpp` (C API) Unicode support** — `on_crash`/`on_crash_upload` now receive UTF-8 encoded paths (`path::u8string()` instead of `path::c_str()`, which returned `wchar_t*` on Windows and didn't compile); `CrashCatch_Config::dump_folder` is now interpreted as UTF-8 (`std::filesystem::u8path()`) instead of the local codepage. Callback signatures remain `const char*` on both platforms.

### Credits

- `std::filesystem::path` migration and Unicode path fixes via PR #9.

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

