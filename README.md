# CrashCatch

*A cross-platform, lightweight, single-header crash-reporting library for modern C++ applications.*

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Header-only](https://img.shields.io/badge/Header--only-yes-green)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightblue)
![C++](https://img.shields.io/badge/C++-17%2B-blue)
![Crash Dumps](https://img.shields.io/badge/Dump-.dmp%2C%20.txt-orange)
![Single Header](https://img.shields.io/badge/Single--header-✔️-green)
![CI](https://github.com/keithpotz/CrashCatch/actions/workflows/build.yml/badge.svg)

> **CrashCatch Analyzer is now in Beta!** A standalone desktop tool for analyzing and understanding your crash reports  symbolicated stack traces, plain-English explanations, and more.
> **[Download / View the Beta on GitHub](https://github.com/keithpotz/Crash-Catch-Analyzer-Release)**

CrashCatch is a lightweight, single-header C++ crash-reporting library that generates `.dmp` and `.txt` crash logs with accurate stack traces, diagnostics, optional cleanup hooks, and user dialogs all with no external dependencies.

---

## Key Features

- Cross-platform: Windows & Linux (macOS planned)
- Single-header integration just `#include "CrashCatch.hpp"`
- Accurate crash-site stack traces Windows stack walk uses the actual crash context, not the handler frame
- `.dmp` MiniDump (Windows) and `.txt` human-readable report (Windows & Linux)
- `onCrash()` and `onCrashUpload()` callbacks fire **after** crash files are written to disk
- Demangled symbols on Linux, `SymFromAddr` with file/line info on Windows
- Thread-safe timestamp generation
- `SymInitialize` called at startup for faster, more reliable symbol resolution
- Fully configurable output path, filename, and format
- DLL / shared library support via `CrashCatchDLL.hpp` for C++11/C++98/C consumers
- CMake install + `find_package` support
- Zero external dependencies

---

## Why CrashCatch?

Most crash reporting solutions require heavyweight SDKs, mandatory cloud uploads, or complex build setups. CrashCatch is different:

| | CrashCatch | Crashpad | Sentry Native | Backtrace |
|---|---|---|---|---|
| Single header | ✅ | ❌ | ❌ | ❌ |
| No external dependencies | ✅ | ❌ | ❌ | ❌ |
| Offline-first | ✅ | ❌ | ❌ | ❌ |
| Windows + Linux | ✅ | ✅ | ✅ | ✅ |
| onCrash / onUpload hooks | ✅ | ❌ | ✅ | ✅ |
| Free & open source | ✅ | ✅ | Partial | ❌ |

---

## Quick Start

### Zero Config

```cpp
#define CRASHCATCH_AUTO_INIT
#include "CrashCatch.hpp"

int main() {
    int* ptr = nullptr;
    *ptr = 42; // CrashCatch catches this and writes a crash report
}
```

### One-Liner

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
#include <iostream>

int main() {
    CrashCatch::Config config;
    config.appVersion        = "2.0.0";
    config.buildConfig       = "Release";
    config.additionalNotes   = "Internal beta build";
    config.dumpFolder        = "./crash_reports/";
    config.includeStackTrace = true;
    config.showCrashDialog   = false; // Windows only

    config.onCrash = [](const CrashCatch::CrashContext& ctx) {
        // Called after crash files are written — safe to read them here
        std::cout << "Crash detected. Log: " << ctx.logFilePath << "\n";
        // flush logs, close handles, etc.
    };

    config.onCrashUpload = [](const CrashCatch::CrashContext& ctx) {
        // Called after onCrash — files are on disk and ready to upload
        uploadToMyServer(ctx.dumpFilePath, ctx.logFilePath);
    };

    CrashCatch::initialize(config);
    // your app code
}
```

---

## Supported Platforms

| OS | Status | Crash Handling |
|---|---|---|
| Windows 10 / 11 | ✅ Supported | `SetUnhandledExceptionFilter` + MiniDump + StackWalk64 |
| Linux | ✅ Supported | POSIX signals + `backtrace()` + `fork()` for safe I/O |
| macOS | Planned | POSIX + Mach exceptions |

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

Or copy `CrashCatch.hpp` directly into your project  no build system required.

---

## Crash Output

When a crash occurs, CrashCatch writes to `./crash_dumps/` by default:

- `crash_YYYY-MM-DD_HH-MM-SS.dmp`  Binary MiniDump (Windows only, viewable in WinDbg)
- `crash_YYYY-MM-DD_HH-MM-SS.txt`  Human-readable crash summary

### Example `.txt` output (Windows)

```text
Crash Report
============
Timestamp: 2026-03-20_09-15-23

Environment Info:
App Version: 2.0.0
Build Config: Release
Platform: Windows
Executable: C:\MyApp\MyApp.exe

Stack Trace:
  [0]: MyApp::GameLoop (C:\MyApp\src\game.cpp:142)
  [1]: main (C:\MyApp\src\main.cpp:28)
  [2]: invoke_main (exe_common.inl:79)
  [3]: __scrt_common_main_seh (exe_common.inl:288)
  [4]: mainCRTStartup (exe_main.cpp:17)
  [5]: BaseThreadInitThunk
  [6]: RtlUserThreadStart
```

### Example `.txt` output (Linux)

```text
Crash Report
============
Signal: Segmentation fault (11)
Timestamp: 2026-03-20_09-15-23

Environment Info:
App Version: 2.0.0
Build Config: Release
Platform: Linux
Executable: /home/user/MyApp

Stack Trace:
  [0]: ./MyApp(MyApp::GameLoop()+0x42) [0x401234]
  [1]: ./MyApp(main+0x1f) [0x401100]
  [2]: libc.so.6(__libc_start_main+0xf3) [0x7f...]
```

> Stack frames on Linux show demangled C++ names when compiled with `-rdynamic`.

---

## Crash Context API

Both `onCrash` and `onCrashUpload` receive a `CrashContext` populated after files are written:

```cpp
struct CrashContext {
    std::string dumpFilePath;  // path to .dmp (Windows) or empty (Linux)
    std::string logFilePath;   // path to .txt crash report
    std::string timestamp;     // YYYY-MM-DD_HH-MM-SS
    int signalOrCode;          // POSIX signal number or Windows exception code
};
```

> Both callbacks fire **after** crash files are on disk. It is safe to open, read, or upload them from within either callback.

---

## DLL / Shared Library Support

For projects that cannot use C++17, `CrashCatchDLL.hpp` provides a plain C interface:

```c
#include "CrashCatchDLL.hpp"

int main() {
    // Zero config
    crashcatch_enable();

    // Or with configuration
    CrashCatch_Config cfg = crashcatch_default_config();
    cfg.app_version        = "2.0.0";
    cfg.include_stack_trace = 1;
    cfg.on_crash = my_crash_callback;
    crashcatch_init(&cfg);
}
```

Build the DLL once with C++17. Consumers link against the compiled binary — no C++17 required on their end.

---

## Configuration Reference

```cpp
struct Config {
    std::string dumpFolder        = "./crash_dumps/"; // output directory
    std::string dumpFileName      = "crash";          // base filename
    bool enableTextLog            = true;             // write .txt report
    bool autoTimestamp            = true;             // append timestamp to filename
    bool showCrashDialog          = false;            // Windows: show MessageBox
    bool includeStackTrace        = true;             // include stack trace in .txt
    std::string appVersion        = "unknown";
    std::string buildConfig       = "Release";        // or "Debug"
    std::string additionalNotes   = "";               // appended to crash report
    std::function<void(const CrashContext&)> onCrash        = nullptr;
    std::function<void(const CrashContext&)> onCrashUpload  = nullptr;
};
```

---

## Examples

Working examples are in the [`/examples`](examples/) folder:

| Example | What it demonstrates |
|---|---|
| `Example_ZeroConfig` | Auto-init with `CRASHCATCH_AUTO_INIT` macro |
| `Example_OneLiner` | `CrashCatch::enable()` minimal setup |
| `Example_FullConfig` | All config options including callbacks |
| `Example_ThreadCrash` | Crash on a non-main thread |
| `Example_divideByZero` | Arithmetic exception handling |
| `Example_UploadCrash` | `onCrashUpload` reading and uploading files |
| `StackTraceExample` | `includeStackTrace` flag |

---

## Requirements

- C++17 or later (or C++11/C++98/C via `CrashCatchDLL.hpp`)
- **Windows:** MSVC (Visual Studio 2019+) or MinGW
- **Linux:** GCC or Clang, link with `-rdynamic` for symbol resolution

---

## Roadmap

- [x] Windows crash capture + MiniDump
- [x] Linux signal handling + backtrace
- [x] `onCrash` and `onCrashUpload` hooks
- [x] CMake install support
- [x] DLL / shared library support (`CrashCatchDLL.hpp`)
- [x] Windows stack trace in `.txt` log with file/line info
- [x] `includeStackTrace` flag
- [x] Accurate crash-site stack context (v1.4.0)
- [x] Async-signal-safe Linux crash handler via `fork()` (v1.4.0)
- [x] Thread-safe timestamp generation (v1.4.0)
- [x] vcpkg and Conan package registry support
- [ ] macOS support (POSIX + Mach exceptions)


---

## Understand Your Crashes — CrashCatch Analyzer

CrashCatch generates the report. **[CrashCatch Analyzer](https://github.com/keithpotz/Crash-Catch-Analyzer-Release)** tells you what it means.

Drop in a crash report and get:

- Symbolicated stack traces
- Plain-English root cause explanation *(Explain Mode)*
- Deep technical analysis for engineers *(Engineer Mode)*
- PDF export for sharing with your team

Currently in Beta. **[View on GitHub](https://github.com/keithpotz/Crash-Catch-Analyzer-Release)** to download.

---

## Contributing

Contributions are welcome. See [CONTRIBUTING.md](C0NTRIBUTING.md) for guidelines.

## License

CrashCatch is licensed under the [MIT License](LICENSE). Free to use, modify, and distribute.
