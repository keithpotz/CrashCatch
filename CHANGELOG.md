# Changelog

All notable changes to this project will be documented in this file.

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

