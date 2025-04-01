# CrashCatch

*A cross-platform, efficient, and easy-to-integrate crash-reporting library for modern C++ applications.*

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Header-only](https://img.shields.io/badge/Header--only-yes-green)
![Platform](https://img.shields.io/badge/Platform-Windows-lightblue)
![C++](https://img.shields.io/badge/C++-17%2B-blue)
![Crash Dumps](https://img.shields.io/badge/Dump-.dmp%2C%20.txt-orange)
![Single Header](https://img.shields.io/badge/Single--header-✔️-green)
![CI](https://github.com/keithpotz/CrashCatch/actions/workflows/build.yml/badge.svg)


CrashCatch is a lightweight, single-header C++ crash-reporting library for generating `.dmp` and `.txt` crash logs — complete with stack traces, diagnostics, optional cleanup hooks, and user dialogs.

---

## 🎯 Overview

CrashCatch provides simple yet powerful crash-reporting and diagnostics for C++ applications across Windows, Linux, and macOS. Spend less time tracking down bugs and more time developing great software.

The current version supports Windows and uses MiniDumpWriteDump to generate crash reports. Linux and macOS support are planned.

---

## 🚀 Key Features

- ✅ Cross-platform design: Windows (available), Linux/macOS (planned)
- 🔹 Single-header C++ integration
- 💥 Stack trace on unhandled exceptions
- 🧠 Automatic `.dmp` + `.txt` crash report generation
- 📋 Self-diagnostics in plain-text (version, uptime, config, etc.)
- 🧼 Minimal setup: drop-in header or one-liner initialization
- ⚙️ Configurable: version tagging, notes, message box, cleanup
- 🛡️ Future-ready: secure upload, anonymization, viewer app
- 🧪 Debug & release support
- 🧵 Symbol resolution for top stack frames (x86 / x64 aware)

---

## 📁 Project Structure

```text
CrashCatch/
├── include/
│   └── CrashCatch.hpp       # Single-header crash handler
├── examples/
│   └── CrashCatchExample.cpp
├── LICENSE                  # MIT License
├── README.md                # Project documentation
└── CONTRIBUTING.md          # Contribution guidelines
```
---

## 🔧 Installation
CrashCatch is 100% header-only.

To integrate:

1. Copy `include/CrashCatch.hpp` into your project
2. Add `#include "CrashCatch.hpp"` in your code
3. (Optional) Use `CrashCatch::enable()` or the `CRASHCATCH_AUTO_INIT` macro

> ✅ No build steps.  
> ❌ No `.lib`, `.dll`, or `.so`.  
> 🔒 Just one `.hpp` file, ready to use.

Copy `CrashCatch.hpp` into your project and include it:

```cpp
#include "CrashCatch.hpp"

## QuickStart
### Zero Config (Auto-init)
```cpp
#define CRASHCATCH_AUTO_INIT
#include "CrashCatch.hpp"

int main(){
    int* ptr = nullptr;
    *ptr = 42; // simulated crash
}
```
### One-Liner Initialization
```cpp
#include "CrashCatch.hpp"
int main(){
    CrashCatch::enable(); //Initalizes with default config

    int* ptr = nullptr;
    *ptr = 42; // simulated crash
}
```
### Full Configuration
```cpp
#include "CrashCatch.hpp"

int main(){
    CrashCatch::Config config:
    config.appVersion = "1.0.0";
    config.buildConfig = "Release";
    config.additionalNotes = "This is a test build.";
    config.onCrash = []{
        std::cout << "Cleaning up before crash...\n"/
    };
    config.showCrashDialog = true;

    CrashCatch::initalize(config);

    int* ptr = nullptr;
    *ptr = 42; // simulated crash
}
```
---

## 📄 Crash Output

When a crash occurs, CrashCatch generates the following files in the `./crash_dumps/` directory:

- `crash_YYYY-MM-DD_HH-MM-SS.dmp` — Binary MiniDump (viewable in tools like WinDbg)
- `crash_YYYY-MM-DD_HH-MM-SS.txt` — Human-readable crash summary

### 📋 Example `.txt` Includes:

```text
Crash Report
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

## 🗺 Roadmap

CrashCatch is actively being developed with the goal of becoming a robust, cross-platform crash-handling solution.

### ✅ Completed

- [x] Initial Windows prototype with `.dmp` and `.txt` support
- [x] One-liner setup (`CrashCatch::enable()`)
- [x] Macro-based auto-initialization (`CRASHCATCH_AUTO_INIT`)
- [x] Stack trace generation (x86 / x64 aware)
- [x] Human-readable crash logs with diagnostics
- [x] Optional message box (GUI-friendly)
- [x] `onCrash` callback hook for cleanup
- [x] Configurable output folder, filename, and formatting

---

### 🔜 Planned

- [ ] **Cross-Platform Support**  
  Linux and macOS crash handling via POSIX signals and core dumps

- [ ] **Unified API**  
  One consistent interface for all supported platforms

- [ ] **Remote Uploads**  
  Optional HTTPS endpoint for uploading crash reports (TLS encrypted)

- [ ] **JSON Crash Logs**  
  Structured `.json` report format alongside `.txt`

- [ ] **Symbol Upload Support**  
  Integration with symbol servers for full symbol resolution

- [ ] **Crash Viewer App**  
  GUI tool to browse `.dmp` / `.txt` reports locally

- [ ] **Package Manager Support**  
  Distribute via vcpkg, Conan, Homebrew, etc.

- [ ] **CI/CD Integration**  
  GitHub Actions for test builds, release pipelines

- [ ] **Auto-Cleanup of Old Reports**  
  Limit or rotate crash logs by count, size, or age

- [ ] **Optional `.zip` Compression**  
  Combine `.dmp` and `.txt` into a zip archive for easier sharing

- [ ] **Auto-Open Crash Log**  
  Developer mode option to open `.txt` in Notepad after crash

- [ ] **Full Documentation Site**  
  GitHub Pages or static `/docs` with setup, usage, API reference

## 📝 Contributing
We warmly welcome contributions! Detailed contribution guidelines will soon be added to `CONTRIBUTING.md`.

## 📄 License
CrashCatch is licensed under the [MIT License](LICENSE.md). You're free to use, modify, and distribute it within your projects.
