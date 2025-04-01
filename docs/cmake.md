# ⚙️ CMake Integration

CrashCatch supports modern CMake out of the box, with both local usage and installable package support.

---

## 🧩 Using as a Subdirectory

If you cloned or copied CrashCatch into your project, add this to your `CMakeLists.txt`:

```cmake
add_subdirectory(CrashCatch)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE CrashCatch::CrashCatch)
```

This uses the header-only target defined inside CrashCatch’s `CMakeLists.txt`.

---

## 📦 Installing to Your System

To install CrashCatch to your machine or a custom prefix:

```bash
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=install
cmake --build . --target install
```

This will generate:
```
install/
├── include/CrashCatch.hpp
└── lib/cmake/CrashCatch/
    ├── CrashCatchConfig.cmake
    └── CrashCatchTargets.cmake
```

---

## 🔍 Using find_package()

In another project, you can now use CrashCatch as a package:

```cmake
find_package(CrashCatch REQUIRED)
add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE CrashCatch::CrashCatch)
```

CMake will locate the headers and preconfigured target with no additional setup.

---

## 🧪 Example
See [`examples/`](../examples/) folder for fully working sample projects that compile with CMake.