# CrashCatch

💥 _A modern, single-header crash reporting library for C++ on Windows and Linux._

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Header-only](https://img.shields.io/badge/Header--only-yes-brightgreen)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightblue)

![C++17](https://img.shields.io/badge/C%2B%2B-17%2B-blue)
![CI](https://github.com/keithpotz/CrashCatch/actions/workflows/build.yml/badge.svg)
[![Stars](https://img.shields.io/github/stars/keithpotz/CrashCatch?style=social)](https://github.com/keithpotz/CrashCatch/stargazers)

---

## 🚀 What is CrashCatch?

CrashCatch is a modern, single-header crash reporting library for C++ applications — supporting both **Windows** and **Linux**.

It automatically captures crashes, logs diagnostic information, generates `.dmp` (Windows) or `.txt` (Windows & Linux) reports, and includes stack traces and environment metadata. From minimal CLI tools to full desktop apps, CrashCatch fits right in.

Key highlights:
- No external dependencies — just include the header
- Full crash context (timestamp, platform, executable path, version, etc.)
- Symbol resolution and demangling (platform-specific)
- Configurable `onCrash()` and `onCrashUpload()` hooks
- Optional crash dialog support (Windows GUI apps)

> As of **v1.2.0**, CrashCatch offers complete Linux support with signal handling, demangled stack traces, and crash context generation.

---

## 🤔 Why CrashCatch?

Most crash reporting solutions for C++ require heavyweight SDKs, mandatory cloud uploads, or days of wiring up Crashpad just to get a stack trace. CrashCatch is different:

| | CrashCatch | Crashpad | Sentry Native | Backtrace |
|---|---|---|---|---|
| Single header | ✅ | ❌ | ❌ | ❌ |
| No external dependencies | ✅ | ❌ | ❌ | ❌ |
| Offline-first | ✅ | ❌ | ❌ | ❌ |
| Windows + Linux | ✅ | ✅ | ✅ | ✅ |
| onCrash / onUpload hooks | ✅ | ❌ | ✅ | ✅ |
| Free & open source | ✅ | ✅ | Partial | ❌ |

---

## ✅ Supported Platforms

| OS      | Supported | Crash Handling Method   |
|---------|-----------|--------------------------|
| Windows | ✅ Yes    | `SetUnhandledExceptionFilter` + MiniDump |
| Linux   | ✅ Yes    | POSIX signals (`signal()`) + backtrace |
| macOS   | 🚧 Planned | POSIX + Mach exceptions

---

## ⚡ Quick Start

### Zero Config (Auto-init)
```cpp
#define CRASHCATCH_AUTO_INIT
#include "CrashCatch.hpp"

int main() {
    int* ptr = nullptr;
    *ptr = 42; // simulated crash
}
```

### One-Liner Setup
```cpp
#include "CrashCatch.hpp"

int main() {
    CrashCatch::enable();
    int* ptr = nullptr;
    *ptr = 42;
}
```

### Full Config Example
```cpp
#include "CrashCatch.hpp"

int main() {
    CrashCatch::Config config;
    config.appVersion = "1.1.0";
    config.buildConfig = "Release";
    config.additionalNotes = "Test build";
    config.showCrashDialog = true;
    config.onCrash = [] {
        std::cout << "Cleaning up before crash...\n";
    };

    CrashCatch::initialize(config);

    int* ptr = nullptr;
    *ptr = 42;
}
```

---

## 📦 Installing with CMake

```bash
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=./install
cmake --build . --target install
```

Then in another project:

```cmake
find_package(CrashCatch REQUIRED)
target_link_libraries(MyApp PRIVATE CrashCatch::CrashCatch)
```

Or just copy `CrashCatch.hpp` directly into your project — no build system required.

---

## 🧪 Examples

Explore working examples in the GitHub repo:

| Example | What it demonstrates |
|---|---|
| `ZeroConfig` | Auto-init with no setup |
| `OneLiner` | `CrashCatch::enable()` minimal setup |
| `FullConfig` | All config options including callbacks |
| `ThreadCrash` | Crash on a non-main thread |
| `DivideByZero` | Arithmetic exception handling |

[View Examples Folder](../examples/)

---

## 📸 Screenshots

![ZeroConfig](img/ZeroConfig.png)
![OneLiner](img/OneLiner.png)
![FullConfig](img/FullConfig1.png)
![CrashDialog](img/screen1.png)

---

## 🛠 Features

- ✅ **Header-only** — single `.hpp`, no external dependencies
- ✅ **Cross-platform** — Windows & Linux support out of the box
- ✅ **Automatic Crash Capture** — via `SetUnhandledExceptionFilter` (Windows) or POSIX signals (Linux)
- ✅ **Crash Context Info** — includes executable path, build config, version, and notes
- ✅ **Crash Reporting**:
  - `.dmp` (Windows) or `.txt` (Linux/Windows) crash files
  - Detailed stack traces and environment info
- ✅ **Symbol Resolution**:
  - Top frame symbols (Windows)
  - Demangled symbols (Linux)
- ✅ **onCrash Callback** — run custom cleanup or logging logic with full crash context
- ✅ **Optional Crash Dialog** — user-friendly message box (Windows only)
- ✅ **Configurable Dump Location**, filename prefix, and timestamping
- ✅ **onCrashUpload Hook** — pass report data to your custom uploader
- ✅ **CMake + CI Friendly** — drop-in installation and build support

---

## 📋 Requirements

- C++17 or later
- **Windows**: MSVC (Visual Studio 2019+) or MinGW
- **Linux**: GCC or Clang with `-rdynamic` for stack traces

---

## 🗺 Roadmap

- [x] Windows crash capture + MiniDump
- [x] Linux signal handling + backtrace
- [x] `onCrash` and `onCrashUpload` hooks
- [x] CMake install support
- [ ] DLL / shared library support
- [ ] Optional stack trace suppression (Windows)
- [ ] macOS support (POSIX + Mach exceptions)
- [ ] vcpkg and Conan package registry support

---

## 🔬 Understand Your Crashes — CrashCatch Labs

CrashCatch generates the report. **[CrashCatch Labs](https://crashcatchlabs.com)** tells you what it means.

CrashCatch Labs is a Windows-first crash analysis tool built specifically for C++ and Unreal Engine developers. Drop in a crash report and get:

- Symbolicated stack traces with engine frames filtered out
- Plain-English root cause explanation *(Explain Mode)*
- Deep technical analysis for engineers *(Engineer Mode)*
- PDF export for sharing with your team

> Currently in development. **[Join the waitlist](https://crashcatchlabs.com/subscribe.html)** to get early access and launch pricing.

---

## 📄 License

MIT License — created and maintained by **Keith Pottratz**  
[GitHub Repo](https://github.com/keithpotz/CrashCatch)

Created by **Keith Pottratz**  
MIT Licensed
