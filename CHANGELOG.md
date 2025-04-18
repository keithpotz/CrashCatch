# Changelog

All notable changes to this project will be documented in this file.

# Changelog

## [1.2.0] - 2025-04-17

### ✨ Added
- ✅ **Linux Support Finalized** — CrashCatch now fully supports Linux crash logging via POSIX signals.
- ✅ **CrashContext Struct** — Provides metadata like timestamp, file paths, and signal/exception codes to `onCrash` and `onCrashUpload`.
- ✅ **`onCrashUpload` Callback** — Optional function to handle post-crash uploads (e.g., to a custom server).
- ✅ **Symbol Resolution on Linux** — Now includes demangled stack traces for improved readability.
- ✅ **Executable Path Detection** — Added `getExecutablePath()` for both Windows and Linux platforms.
- ✅ **Linux Support Documentation** — New `linux.md` file and documentation updates.
- ✅ **Improved Logging** — Crash logs include platform, architecture, and optional diagnostic notes.

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

