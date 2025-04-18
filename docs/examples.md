# 🧪 Examples

CrashCatch comes with real-world usage examples to demonstrate how easy it is to integrate.

All examples are located in the [`examples/`](../examples/) folder.

---

## 📁 Included Examples

### 🔹 Example_ZeroConfig.cpp  
Uses the `#define CRASHCATCH_AUTO_INIT` macro to auto-initialize CrashCatch with no config.

### 🔹 Example_OneLiner.cpp  
Initializes CrashCatch at runtime with `CrashCatch::enable()`.

### 🔹 Example_FullConfig.cpp  
Demonstrates full customization:
- `appVersion`
- `buildConfig`
- `onCrash` lambda
- `showCrashDialog`

### 🔹 Example_ThreadCrash.cpp  
Simulates a crash in a background thread to test thread safety and handler stability.

### 🔹 Example_divideByZero.cpp  
Triggers a divide-by-zero exception to test signal handling for `SIGFPE` (Linux) or exception codes (Windows).

### 🔹 Example_onCrash.cpp  
Demonstrates usage of the `onCrash(const CrashContext&)` callback to perform pre-exit cleanup or logging.  
Shows how to access crash file paths, timestamps, and error codes inside the callback.

### 🔹 Example_UploadCrash.cpp  
Simulates sending a crash report `.txt` or `.dmp` file to a remote server.  
Demonstrates how to use the `onCrash` callback to call a user-defined uploader function after a crash occurs.

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
Example_onCrash.exe
```

---

Each example will generate `.dmp` and `.txt` crash logs in the `./crash_dumps/` folder when triggered.