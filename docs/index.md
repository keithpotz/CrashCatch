# CrashCatch

💥 _A modern, single-header crash reporting library for C++ on Windows and Linux._

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Header-only](https://img.shields.io/badge/Header--only-yes-brightgreen)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightblue)
![C++17](https://img.shields.io/badge/C%2B%2B-17%2B-blue)
![CI](https://github.com/keithpotz/CrashCatch/actions/workflows/build.yml/badge.svg)

---

## 🚀 What is CrashCatch?
CrashCatch is a zero-dependency, header-only crash reporting library for C++ applications.  
It captures crashes, generates `.dmp` (Windows) or `.txt` (Windows and Linux) files, includes stack traces, and optionally shows a GUI-friendly message box — all from a single header.
* Works great in embedded systems, CLI tools, and GUI apps alike.


As of **v1.1.0**, CrashCatch supports both **Windows** and **Linux** out of the box.
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

---

## 🧪 Examples
Explore working examples in the GitHub repo:
- ZeroConfig
- OneLiner
- FullConfig
- ThreadCrash
- DivideByZero

[View Examples Folder](../examples/)

---

## 📸 Screenshots

![ZeroConfig](img/ZeroConfig.png)
![OneLiner](img/OneLiner.png)
![FullConfig](img/FullConfig1.png)
![CrashDialog](img/screen1.png)

---

## 🛠 Features
- ✅ Header-only — drop-in, no dependencies
- ✅ Windows `.dmp` and `.txt` crash logs
- ✅ Linux .txt logs with stack trace and signal info
- ✅ Thread safety
- ✅ Symbol resolution for stack trace
- ✅ GUI message box support (Windows)
- ✅ CMake + CI friendly
- 🔜 macOS and remote reporting coming soon

---

## 📄 License
MIT License — created and maintained by **Keith Pottratz**  
[GitHub Repo](https://github.com/keithpotz/CrashCatch)


Created by **Keith Pottratz**  
MIT Licensed
