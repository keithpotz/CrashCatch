# CrashCatch v1.6.0 Release Notes

**Released:** Unreleased

This release adds macOS support and a `shutdown()` API for clean teardown, along with a set of robustness fixes from production usage. Existing code compiles and runs without modification.

---

## What Changed

### macOS support

CrashCatch now runs on macOS (Intel and Apple Silicon). It uses the same design as the Linux handler: POSIX signals (`SIGSEGV`, `SIGABRT`, `SIGFPE`, `SIGILL`, `SIGBUS`) are caught, and a `fork()`ed child process writes the `.txt` report and runs your callbacks, so the handler stays safe even if the heap is corrupt.

Stack traces use `backtrace()` with demangled C++ symbols. If Xcode Command Line Tools are installed and the binary has debug symbols, file and line numbers are resolved with a single batched `atos` call; otherwise frames fall back to `module symbol + offset`.

```text
Stack Trace:
  [0]: MyApp crashFunction() + 24 (main.cpp:12)
  [1]: MyApp main + 40 (main.cpp:20)
  [2]: dyld start + 6076
```

Not yet implemented on macOS: the crash message box and Mach exception interception. On Apple Silicon, integer divide-by-zero does not raise `SIGFPE` (ARM64 doesn't trap on it in hardware).

---

### New: `CrashCatch::shutdown()`

`initialize()` replaces the process-wide crash handler. There was previously no way to undo that, and the DbgHelp symbol state loaded at startup was never released, which shows up as leaks in tools such as Valgrind.

`shutdown()` restores whatever handler was installed before `initialize()` (the unhandled-exception filter on Windows, the five signal handlers on Linux/macOS) and calls `SymCleanup` on Windows.

```cpp
CrashCatch::initialize(config);
// ... your app ...
CrashCatch::shutdown();
```

C API users get the same thing through `crashcatch_shutdown()`.

`shutdown()` is safe to call more than once, or without a prior `initialize()`: in that case it does nothing and returns `false`, rather than overwriting the application's own handlers.

---

### `initialize()` can be called again to change configuration

Calling `initialize()` a second time used to install the handlers again. Doing so recorded CrashCatch's own handler as the "previous" one, so a later `shutdown()` could never restore the application's original handler. A second call now only updates the configuration.

---

### Windows: crash dump is no longer locked while the dialog is open

With `showCrashDialog` enabled, the message box says where the `.dmp` was written, but the file was still held open with exclusive access until the dialog was dismissed. The dump is now closed immediately after it is written.

---

### Windows: symbol buffer alignment

The buffer passed to `SymFromAddr` was a plain `char` array cast to `SYMBOL_INFO*`, which is undefined behavior if the array is not suitably aligned. It is now declared `alignas(SYMBOL_INFO)`.

---

## Internal Improvements

- Cleanup of `__cxa_demangle` results, `backtrace_symbols` arrays, the `atos` pipe, and the dump file handle now goes through a small RAII helper (`details::AtScopeExit`), so no return path can skip it.
- `GetModuleFileNameA` failure is handled; the executable path is reported as `(unknown)` instead of reading an uninitialized buffer.
- `user32.lib` is auto-linked on Windows (needed by `MessageBoxW`), alongside `dbgHelp.lib`.
- macOS `demangle()` no longer throws or loops on malformed backtrace lines, and the `atos` command line is built correctly (`atos -p <pid>`).
- `crashcatch_version()` returns `"1.6.0"`.

---

## Upgrading

No breaking changes. Replace `CrashCatch.hpp` (and `CrashCatchDLL.hpp` if used) with the v1.6.0 versions and recompile. Calling `CrashCatch::shutdown()` before exit is optional but recommended if you run leak checkers.

---

## Platform Compatibility

| Platform | Status |
|---|---|
| Windows 10 / 11 (x64, x86) | Tested |
| Linux (GCC, Clang) | Builds in CI |
| macOS (Intel, Apple Silicon) | Supported; reported working on macOS 27 (AArch64) |

---

## Credits

- `shutdown()`, RAII cleanup, symbol-buffer alignment, and `GetModuleFileNameA` handling via PR #17.

---

## Full Changelog

See [CHANGELOG.md](CHANGELOG.md) for the complete history.
