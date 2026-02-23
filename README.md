# CrashCatch

*A cross-platform, efficient, and easy-to-integrate crash-reporting library for modern C++ applications.*

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Header-only](https://img.shields.io/badge/Header--only-yes-green)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightblue)
![C++](https://img.shields.io/badge/C++-17%2B-blue)
![Crash Dumps](https://img.shields.io/badge/Dump-.dmp%2C%20.txt-orange)
![Single Header](https://img.shields.io/badge/Single--header-✔️-green)
![CI](https://github.com/keithpotz/CrashCatch/actions/workflows/build.yml/badge.svg)


CrashCatch is a lightweight, single-header C++ crash-reporting library for generating `.dmp` and `.txt` crash logs — complete with stack traces, diagnostics, optional cleanup hooks, and user dialogs.

---

## 🎯 Overview

CrashCatch provides simple and powerful crash diagnostics for C++ applications on **Windows and Linux**, with macOS support planned. Whether you're building GUI apps, system tools, or CLI utilities, CrashCatch helps you catch and log critical failures with minimal setup.

As of **v1.2**, CrashCatch supports rich crash context capture and includes optional post-crash upload integration.


---
## 🚀 Key Features

- ✅ Cross-platform: Windows & Linux (macOS coming soon)
- 🔹 Single-header C++ integration
- 💥 Exception and signal handling (`SIGSEGV`, `SIGABRT`, `SIGFPE`, etc.)
- 🧠 `.dmp` and `.txt` crash report generation
- 🧩 **`onCrash()` & `onCrashUpload()` Hooks** – Run cleanup or upload crash artifacts
- 🔍 Demangled symbols on Linux, symbol resolution on Windows
- 📋 Self-diagnostics: version, build, architecture, exec path
- 🧼 Drop-in, no external libraries required
- ⚙️ Fully configurable output format & location
- 📁 CMake install + `find_package` support
- 🧪 Debug & release builds supported
---

--

## Why CrashCatch?

Most crash reporting solutions for C++ require heavyweight SDKs, mandatory cloud uploads, or platform-specific build gymnastics. CrashCatch is different:

| | CrashCatch | Crashpad | Sentry Native | Backtrace |
|---|---|---|---|---|
| Single header | ✅ | ❌ | ❌ | ❌ |
| No external dependencies | ✅ | ❌ | ❌ | ❌ |
| Offline-first | ✅ | ❌ | ❌ | ❌ |
| Windows + Linux | ✅ | ✅ | ✅ | ✅ |
| onCrash / onUpload hooks | ✅ | ❌ | ✅ | ✅ |
| Free & open source | ✅ | ✅ | Partial | ❌ |

---

---

## Quick Start

### Zero Config — just include and it works

```cpp
#define CRASHCATCH_AUTO_INIT
#include "CrashCatch.hpp"

int main() {
    int* ptr = nullptr;
    *ptr = 42; // CrashCatch catches this and writes a crash report
}
```

### One-Liner Setup

```cpp
#include "CrashCatch.hpp"

int main() {
    CrashCatch::enable();
    // your app code
}
```

### Full Configuration

```cpp
#include "CrashCatch.hpp"

int main() {
    CrashCatch::Config config;
    config.appVersion      = "1.1.0";
    config.buildConfig     = "Release";
    config.additionalNotes = "Internal beta build";
    config.showCrashDialog = true;

    config.onCrash = [] {
        // Runs before process exits — flush logs, close handles, etc.
        std::cout << "Crash detected, cleaning up...\n";
    };

    config.onCrashUpload = [](const CrashCatch::CrashReport& report) {
        // Send to your own backend, S3, or CrashCatch Labs
        uploadReport(report.dumpPath, report.contextJson);
    };

    CrashCatch::initialize(config);
    // your app code
}
```

---

## Supported Platforms

| OS | Status | Crash Handling |
|---|---|---|
| Windows 10 / 11 | ✅ Supported | `SetUnhandledExceptionFilter` + MiniDump |
| Linux | ✅ Supported | POSIX signals + `backtrace()` |
| macOS | 🚧 Planned | POSIX + Mach exceptions |

> **v1.2.0** — Complete Linux support shipped: signal handling, demangled stack traces, and crash context generation.

---

## Installing with CMake

```bash
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=./install
cmake --build . --target install
```

Then in your project:

```cmake
find_package(CrashCatch REQUIRED)
target_link_libraries(MyApp PRIVATE CrashCatch::CrashCatch)
```

Or just copy `CrashCatch.hpp` directly into your project — no build system required.

---

## Examples

Working examples are in the [`/examples`](examples/) folder:

| Example | What it demonstrates |
|---|---|
| `ZeroConfig` | Auto-init with no setup |
| `OneLiner` | `CrashCatch::enable()` minimal setup |
| `FullConfig` | All config options including callbacks |
| `ThreadCrash` | Crash on a non-main thread |
| `DivideByZero` | Arithmetic exception handling |

---

## Requirements

- C++17 or later
- Windows: MSVC (Visual Studio 2019+) or MinGW
- Linux: GCC or Clang with `-rdynamic` for stack traces

---

## 📄 Crash Output

When a crash occurs, CrashCatch generates the following files in the `./crash_dumps/` directory:

- `crash_YYYY-MM-DD_HH-MM-SS.dmp` — Binary MiniDump (viewable in tools like WinDbg)
- `crash_YYYY-MM-DD_HH-MM-SS.txt` — Human-readable crash summary

### 📋 Example `.txt` Includes:

```text
Crash Report (Windows)
============

Timestamp: 2025-04-01_14-23-56
Dump File: ./crash_dumps/crash_2025-04-01_14-23-56.dmp
Exception Code: 0xC0000005
Thread ID: 12345

Stack Trace:
  [0]: MyApp::SomeFunction + 0x15
  [1]: MyApp::MainLoop + 0x2a
  [2]: WinMain + 0x10
  [3]: __tmainCRTStartup + 0x20
  [4]: BaseThreadInitThunk + 0x14
  [5]: RtlUserThreadStart + 0x21

Environment Info:
App Version: 1.0.0
Build Config: Release
Architecture: x64
Executable: C:\Path\To\YourApp.exe
Uptime (s): 182
Notes: Test build
```
```text
Crash Report (Linux)
============
Timestamp: 2025-04-04_15-42-18
Signal: Segmentation fault (11)

Stack Trace:
  [0]: ./CrashCatchTest(+0x1234)
  [1]: libc.so.6(+0xdeadbeef)
  [2]: start_thread
  [3]: __libc_start_main

Environment Info:
App Version: 1.1.0
Build Config: Release
Platform: Linux
Executable: /home/user/CrashCatchTest
```

### The .txt files contains:
- **Timestamp**: The time the crash occurred.
- **Dump File**: The path to the generated MiniDump file.
- **Exception Code**: The exception code that caused the crash.
- **Thread ID**: The ID of the thread that crashed.
- **Stack Trace**: A list of function calls leading up to the crash.
- **Environment Info**: Additional information about the environment in which the crash occurred.
- **Architecture (x86/x64)**: The architecture of the system where the crash occurred.
- **Executable**: The path to the executable that crashed.
- **Uptime (s)**: The duration the application was running when the crash occurred.
-**App version**: The version of the application from config.
- **Build Config**: The configuration of the build (e.g., Release, Debug).
- **Notes**: Any additional notes or comments about the crash.

---
---
## Crash Context API
CrashCatch provides detailed crash metadata via `CrashContext`:
```cpp
struct CrashContext{
  std::string dumpFilePath;
  std::string logFilePath;
  std::string timestamp;
  int singalOrCode;
};
```
You can use both in `onCrash` and `onCrashUpload`
---

## 🗺 Roadmap

CrashCatch is actively being developed with the goal of becoming a robust, cross-platform crash-handling solution.

- [x] Windows crash capture + MiniDump
- [x] Linux signal handling + backtrace
- [x] `onCrash` and `onCrashUpload` hooks
- [x] CMake install support
- [ ] DLL / shared library support
- [ ] Optional stack trace suppression (Windows)
- [ ] macOS support (POSIX + Mach exceptions)
- [ ] vcpkg and Conan package registry support

---

---

## Understand Your Crashes — CrashCatch Labs

CrashCatch generates the report. **[CrashCatch Labs](https://crashcatchlabs.com)** tells you what it means.

CrashCatch Labs is a Windows-first crash analysis tool built specifically for C++ and Unreal Engine developers. Drop in a crash report and get:

- Symbolicated stack traces with engine frames filtered out
- Plain-English root cause explanation *(Explain Mode)*
- Deep technical analysis for engineers *(Engineer Mode)*
- PDF export for sharing with your team

> Currently in development. **[Join the waitlist](https://crashcatchlabs.com/subscribe.html)** to get early access and launch pricing.

---

## 📝 Contributing
We warmly welcome contributions! Check `CONTRIBUTING.md` for more information.

## 📄 License
CrashCatch is licensed under the [MIT License](LICENSE.md). You're free to use, modify, and distribute it within your projects.
