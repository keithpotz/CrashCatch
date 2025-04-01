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
- appVersion
- buildConfig
- onCrash lambda
- message box

### 🔹 Example_ThreadCrash.cpp
Simulates a crash in a background thread to test thread safety.

### 🔹 Example_divideByZero.cpp
Triggers a divide-by-zero exception to test how CrashCatch handles runtime errors.

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
```

---

Each example will generate `.dmp` and `.txt` crash logs in the `./crash_dumps/` folder when triggered.