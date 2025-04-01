# 🚀 Quick Start

CrashCatch is designed to be usable in less than 10 seconds. Choose from one of the quick-start modes below:

## 🟢 Zero Config (Auto-init)
```cpp
#define CRASHCATCH_AUTO_INIT
#include "CrashCatch.hpp"

int main() {
    int* ptr = nullptr;
    *ptr = 42; // simulated crash
}
```

## 🟡 One-Liner Setup
```cpp
#include "CrashCatch.hpp"

int main() {
    CrashCatch::enable();
    int* ptr = nullptr;
    *ptr = 42;
}
```

## 🔧 Full Configuration
```cpp
#include "CrashCatch.hpp"

int main() {
    CrashCatch::Config config;
    config.appVersion = "1.0.0";
    config.buildConfig = "Release";
    config.additionalNotes = "This is a test build.";
    config.showCrashDialog = true;
    config.onCrash = [] {
        std::cout << "Performing cleanup before crash...\n";
    };

    CrashCatch::initialize(config);

    int* ptr = nullptr;
    *ptr = 42;
}
```

---

After a crash, CrashCatch generates a `.dmp` and `.txt` log in `./crash_dumps/` by default.
