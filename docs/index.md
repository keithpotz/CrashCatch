# CrashCatch

_A lightweight, single-header C++ crash reporting library for modern applications._

---

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Header-only](https://img.shields.io/badge/Header--only-yes-green)
![Platform](https://img.shields.io/badge/Platform-Windows-lightblue)

CrashCatch lets you add crash reporting to your C++ app in under 10 seconds.

---

## 🚀 Quick Start

### 🟢 Zero Config (Auto-init)
```cpp
#define CRASHCATCH_AUTO_INIT
#include "CrashCatch.hpp"

int main() {
    int* ptr = nullptr;
    *ptr = 42; // simulated crash
}
```

### 🟡 One-Liner
```cpp
#include "CrashCatch.hpp"

int main() {
    CrashCatch::enable();

    int* ptr = nullptr;
    *ptr = 42;
}
```

### 🔧 Full Config
```cpp
#include "CrashCatch.hpp"

int main() {
    CrashCatch::Config config;
    config.appVersion = "1.0.0";
    config.buildConfig = "Release";
    config.additionalNotes = "Beta test build";
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

## 📄 What Happens On Crash?

CrashCatch will generate:
- `.dmp` file with MiniDumpWriteDump (viewable in WinDbg)
- `.txt` file with stack trace + diagnostics

Everything goes into `./crash_dumps/` by default.

---

## 🧪 Examples

Real examples are available in the [examples folder](../examples/):
- Zero Config
- One Liner
- Full Config + Dialog
- Background Thread Crash
- Divide By Zero

---

## 📦 Installation

You only need **one file**:

```text
include/CrashCatch.hpp
```

No libraries to link. No build steps. Just `#include` and go.

---

## 🔗 Resources

- [GitHub Repository](https://github.com/yourusername/CrashCatch)
- [README + Roadmap](../README.md)
- [License](../LICENSE.md)

---

Created by **Keith Pottratz**  
MIT Licensed
