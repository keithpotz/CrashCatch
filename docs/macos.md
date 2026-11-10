# 🍎 macOS Support

*New in v1.6.0.*

CrashCatch handles crashes in native macOS C++ applications using POSIX signals and `backtrace()`, on both Intel and Apple Silicon Macs.

It writes a human-readable `.txt` report with demangled stack traces, environment info and, when debug symbols are available, file and line numbers. All of it comes from the same single header used on Windows and Linux.

---

## 🔥 Supported Signals

| Signal     | Description                              |
|------------|------------------------------------------|
| `SIGSEGV`  | Segmentation fault (invalid memory access) |
| `SIGABRT`  | Aborted process (e.g. `abort()`)           |
| `SIGFPE`   | Floating point exception (e.g. integer divide by zero on Intel) |
| `SIGILL`   | Illegal instruction                        |
| `SIGBUS`   | Bus error (e.g. misaligned memory access)  |

> All signals trigger a crash log and invoke the optional `onCrash()` and `onCrashUpload()` callbacks with full crash context.

!!! note "Apple Silicon and divide by zero"
    On Apple Silicon (ARM64), integer divide by zero does **not** raise `SIGFPE`. ARM64 doesn't trap on it in hardware, so the program continues with an undefined result and no crash report is written. This is a platform limitation, not a CrashCatch bug. On Intel Macs it raises `SIGFPE` as on Linux.

---

## 🧠 macOS Features

- ✅ Signal name logged in crash reports (via `strsignal()`, e.g. `Segmentation fault: 11`)
- ✅ Async-signal-safe handler using `fork()`, the same design as Linux
- ✅ Demangled C++ symbol names in stack traces
- ✅ File/line numbers via `atos`, resolved in one batched call for all frames (optional)
- ✅ Executable path detection via `_NSGetExecutablePath`
- ✅ Crash context includes signal, timestamp, paths, and notes
- ✅ Callbacks fire after crash files are written
- ✅ Thread-safe timestamp generation via `localtime_r`
- ✅ `CrashCatch::shutdown()` restores the previous signal handlers *(v1.6.0)*

---

## ⚡ How the Handler Works

macOS uses the same handler as Linux. Signal handlers may only call **async-signal-safe** functions, and calling `malloc`, `std::string` or file I/O directly from one can deadlock if the crash happened mid-heap-operation.

CrashCatch avoids this with `fork()`:

1. The signal fires and the handler builds the output path
2. `fork()` is called and the child process inherits the parent's memory image
3. The **child** writes the report, resolves the stack trace, and runs your callbacks
4. The **parent** waits for the child to finish, then calls `_exit()`

### File and line numbers

macOS's `backtrace_symbols()` gives a module, symbol and offset for each frame, but no source location. To add one, the child process runs `atos -p <pid>` once with every frame address, while the crashed parent process is still alive to be inspected.

- If `atos` returns `file:line` for a frame, it's appended to that frame: `(main.cpp:12)`.
- If it doesn't, because there are no debug symbols, `atos` isn't installed, or the frame is in a system library, the frame is written without it. Nothing fails.

---

## 📋 Output Format

When a crash occurs, CrashCatch creates a `.txt` log in `./crash_dumps/` (configurable) containing:

- Timestamp
- Signal name
- Stack trace with module, demangled symbol, offset and (optionally) file:line
- Application version and build config
- Executable path
- Any additional notes set in config

### 🧾 Example Output

```text
Crash Report
============
Signal: Segmentation fault: 11
Timestamp: 2026-03-20_09-15-23

Environment Info:
App Version: 2.0.0
Build Config: Release
Platform: macOS
Executable: /Users/user/MyApp

Stack Trace:
  [0]: MyApp CrashCatch::writeCrashLog(...) + 676 (CrashCatch.hpp:408)
  [1]: MyApp CrashCatch::posixSignalHandler(int) + 304 (CrashCatch.hpp:510)
  [2]: libsystem_platform.dylib _sigtramp + 29
  [3]: MyApp main + 52
  [4]: dyld start + 6076
```

Each frame reads `module  symbol + offset (file:line)`.

> **Reading the trace:** the frames above `_sigtramp` are CrashCatch's own handler. The frame right after `_sigtramp` is where your program crashed (here, `main`).

> System library frames (`dyld`, `libsystem_platform.dylib`, ...) never get file/line, because Apple doesn't ship debug info for them.

---

## Testing macOS Crashes

### Null Pointer
```cpp
#include "CrashCatch.hpp"

int main() {
    CrashCatch::enable();
    int* ptr = nullptr;
    *ptr = 42; // SIGSEGV
    return 0;
}
```

### Abort
```cpp
#include <cstdlib>
#include "CrashCatch.hpp"

int main() {
    CrashCatch::enable();
    std::abort(); // SIGABRT
}
```

### Threaded Fault
```cpp
#include <thread>
#include "CrashCatch.hpp"

void crashFunc() {
    int* p = nullptr;
    *p = 5; // SIGSEGV from background thread
}

int main() {
    CrashCatch::enable();
    std::thread t(crashFunc);
    t.join();
}
```

> Use a null pointer or `abort()` for testing on Apple Silicon. The divide-by-zero test won't crash there (see the note above).

---

## Requirements and Build Setup

- **Compiler:** Clang via Xcode Command Line Tools:

    ```bash
    xcode-select --install
    ```

- **C++17** or later.
- **For file/line numbers:** `atos` (included with Command Line Tools) and a build with debug info (`-g`).

Using the compiler directly:

```bash
clang++ -std=c++17 -g main.cpp -o myapp
```

With CMake, build a configuration that includes debug info, such as `Debug` or `RelWithDebInfo`:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

or add it to your target:

```cmake
target_compile_options(MyApp PRIVATE -g)
```

Unlike Linux, macOS doesn't need `-rdynamic` for function names to appear in `backtrace_symbols()` output.

---

## 🚧 Known Limitations

- Crash logs are `.txt` only. The `.dmp` MiniDump format is Windows only.
- `showCrashDialog` has no effect on macOS. A native message box would need an Objective-C dependency, which conflicts with the single-header goal.
- Mach exceptions are not intercepted, only POSIX signals. Crashes that never become a signal won't be caught.
- Integer divide by zero does not crash on Apple Silicon (see above).
- File/line numbers need debug symbols and `atos`. Stripped or Release builds without debug info show `module symbol + offset` only.
