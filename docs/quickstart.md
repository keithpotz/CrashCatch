# 🚀 Quick Start

CrashCatch is designed to be usable in less than 10 seconds. Choose from one of the quick-start modes below:

## 🟢 Zero Config (Auto-init)
```cpp
#define CRASHCATCH_AUTO_INIT
#include "CrashCatch.hpp"

int main() {
    int* ptr = nullptr;
    *ptr = 42; // CrashCatch catches this automatically
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
#include <iostream>

int main() {
    CrashCatch::Config config;
    config.appVersion        = "2.0.0";
    config.buildConfig       = "Release";
    config.additionalNotes   = "This is a test build.";
    config.dumpFolder        = "./crash_reports/";
    config.includeStackTrace = true;
    config.showCrashDialog   = false;

    config.onCrash = [](const CrashCatch::CrashContext& ctx) {
        // Called after crash files are written — safe to read them here
        std::cout << "Crash detected!\n";
        std::cout << "Log file: " << ctx.logFilePath << "\n";
        std::cout << "Signal/Code: " << ctx.signalOrCode << "\n";
    };

    config.onCrashUpload = [](const CrashCatch::CrashContext& ctx) {
        // Called after onCrash — upload files to your server here
        uploadToMyServer(ctx.dumpFilePath, ctx.logFilePath);
    };

    CrashCatch::initialize(config);

    int* ptr = nullptr;
    *ptr = 42;
}
```

---

After a crash, CrashCatch generates a `.dmp` and `.txt` log in `./crash_dumps/` by default (or wherever `dumpFolder` points).

## 📋 Configuration Reference

| Field | Default | Description |
|---|---|---|
| `dumpFolder` | `./crash_dumps/` | Output directory for crash files |
| `dumpFileName` | `crash` | Base filename |
| `enableTextLog` | `true` | Write `.txt` human-readable report |
| `autoTimestamp` | `true` | Append timestamp to filename |
| `showCrashDialog` | `false` | Windows: show MessageBox on crash |
| `includeStackTrace` | `true` | Include stack trace in `.txt` log |
| `appVersion` | `unknown` | Your application version string |
| `buildConfig` | `Release` | Build configuration label |
| `additionalNotes` | _(empty)_ | Extra notes written into the crash log |
| `onCrash` | `nullptr` | Callback fired after files are written |
| `onCrashUpload` | `nullptr` | Callback for uploading crash files |
