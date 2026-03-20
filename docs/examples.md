# 🧪 Examples

CrashCatch comes with real-world usage examples to demonstrate how easy it is to integrate.

All examples are located in the [`examples/`](../examples/) folder.

---

## 📁 Included Examples

### 🔹 Example_ZeroConfig.cpp
Uses the `#define CRASHCATCH_AUTO_INIT` macro to auto-initialize CrashCatch with no config — just include and it works.

### 🔹 Example_OneLiner.cpp
Initializes CrashCatch at runtime with a single call to `CrashCatch::enable()`.

### 🔹 Example_FullConfig.cpp
Demonstrates full customization:
- `appVersion` and `buildConfig` metadata
- `additionalNotes` in the crash report
- `onCrash` callback with full crash context
- `showCrashDialog` Windows message box

### 🔹 Example_ThreadCrash.cpp
Simulates a crash in a background thread to verify that the handler catches crashes from non-main threads correctly.

### 🔹 Example_divideByZero.cpp
Triggers a divide-by-zero exception to test arithmetic fault handling — `SIGFPE` on Linux, exception code `0xC0000094` on Windows.

### 🔹 Example_UploadCrash.cpp
Demonstrates the `onCrashUpload` callback. Shows how to read and upload crash files after they have been written to disk. Both `.dmp` and `.txt` files are passed via the `CrashContext`.

### 🔹 Example_StackTrace.cpp
Demonstrates the `includeStackTrace` config flag added in v1.3.0 toggle stack trace output in the `.txt` log on or off.

### 🔹 TestCrash (onCrash.cpp)
Demonstrates the `onCrash(const CrashContext&)` callback how to access crash file paths, timestamps, and error codes inside the callback.

---

## 🧰 Build the Examples (via CMake)

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Each example is compiled as its own executable:
```
Example_ZeroConfig.exe
Example_OneLiner.exe
Example_FullConfig.exe
Example_ThreadCrash.exe
Example_divideByZero.exe
Example_UploadCrash.exe
Example_StackTrace.exe
TestCrash.exe
```

---

Each example will generate `.dmp` and `.txt` crash logs in the `./crash_dumps/` folder when triggered.

> **Note:** `onCrash` and `onCrashUpload` callbacks receive file paths that are guaranteed to exist on disk by the time the callback fires it is safe to open, read, or upload them from within either callback.
