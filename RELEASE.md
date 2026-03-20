# CrashCatch v1.4.0 Release Notes

**Released:** 2026-03-20

This release is a correctness-focused update. No public API has changed existing code compiles and runs without modification. All changes are internal fixes to long-standing bugs in the crash handling implementation.

---

## What Changed

### Windows: Stack trace now shows the actual crash location

Previously, `writeCrashLog` called `RtlCaptureContext` from inside the exception handler. This captured the handler's own call stack, meaning frame `[0]` showed internal CrashCatch functions rather than the code that crashed.

The Windows exception handler now passes `ep->ContextRecord` (the CPU state frozen at the moment of the fault) directly into the stack walker. Frame `[0]` now correctly identifies the crashing function, file, and line number.

**Before:**
```text
Stack Trace:
  [0]: writeCrashLog
  [1]: UnhandledExceptionHandler
  [2]: RtlDispatchException
  ...
```

**After:**
```text
Stack Trace:
  [0]: MyApp::GameLoop (src\game.cpp:142)
  [1]: main (src\main.cpp:28)
  [2]: invoke_main (exe_common.inl:79)
  ...
```

---

### Callbacks now fire after crash files are written

`onCrash` and `onCrashUpload` previously received file paths before those files existed on disk. Any code inside the callback that tried to read, parse, or upload the crash report would fail silently or open an empty file.

Both callbacks now fire after all files are fully written and closed. The file paths in `CrashContext` are guaranteed to be readable when either callback is invoked.

---

### Linux signal handler is now async-signal-safe

Calling heap-allocating functions (`std::string`, `std::ofstream`, `malloc`, `backtrace_symbols`, `abi::__cxa_demangle`) directly from a POSIX signal handler is undefined behavior. If the crash occurred mid-`malloc` (e.g. heap corruption), the handler would deadlock.

The Linux signal handler now uses `fork()`. The child process performs all file I/O and executes callbacks it is safe to use the heap there. The parent calls `_exit()` immediately. This is the same pattern used by Google's Crashpad and Breakpad.

---

### Linux: Demangled symbols now actually demangle

`backtrace_symbols()` returns full strings like `./myapp(_Z3foov+0x10) [0x7f...]`. The previous `demangle()` function passed the entire string to `abi::__cxa_demangle`, which expects a raw mangled name and always returned the original string unchanged.

The mangled symbol is now extracted from between `(` and `+` before demangling, producing readable output:

**Before:**
```text
  [0]: ./myapp(_Z8myMethodv+0x42) [0x401234]
```

**After:**
```text
  [0]: ./myapp(myMethod()+0x42) [0x401234]
```

---

### Thread-safe timestamps

`std::localtime` returns a pointer to a shared static struct and is not thread-safe. A crash on a background thread while the main thread was also formatting a timestamp could produce a corrupted timestamp string.

Replaced with `localtime_s` (Windows) and `localtime_r` (Linux), both of which write into a caller-supplied struct.

---

### Symbol loading moved to startup

`SymInitialize` was previously called inside the exception handler at crash time. This is slow (~100ms on large binaries) and will fail if another part of the application has already initialized DbgHelp symbols.

`SymInitialize` is now called once during `CrashCatch::initialize()`, so symbols are loaded and ready before any crash occurs.

---

### Directory creation failures are now handled

If `create_directories` failed (bad path, missing permissions), the error was silently ignored and subsequent file writes would fail with no diagnostic. The `std::error_code` overload is now used if the output directory cannot be created, the handler exits early rather than attempting to write to an invalid path.

---

## Internal Improvements

- `CrashContext` is now constructed once per crash and shared between `onCrash` and `onCrashUpload`, rather than being constructed twice.
- Version string in `CrashCatch.hpp` header comment corrected to match the release version.

---

## New Tests

Two tests have been added to `tests/` to permanently verify the corrected behavior:

| Test | Verifies |
|---|---|
| `test_callback_order` | Log file exists on disk when `onCrash` fires |
| `test_stack_context` | Stack trace frame `[0]` is the crash-site function, not a handler frame |

---

## Upgrading

No changes to the public API. Replace `CrashCatch.hpp` (and `CrashCatchDLL.hpp` if used) with the v1.4.0 versions. Recompile. No code changes required.

---

## Platform Compatibility

| Platform | Status |
|---|---|
| Windows 10 / 11 (x64, x86) | Tested |
| Linux (GCC, Clang) | Fixes applied; signal-safety fix verified by code review — runtime testing on Linux recommended |
| macOS | Not yet supported |

---

## Full Changelog

See [CHANGELOG.md](CHANGELOG.md) for the complete history.
