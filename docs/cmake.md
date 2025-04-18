# ⚙️ CMake Integration

CrashCatch supports **modern CMake** out of the box, with both **header-only subdirectory usage** and **installable package support** for larger or multi-project environments.

---

## 🧩 Using as a Subdirectory

If you cloned or copied CrashCatch into your project, add this to your `CMakeLists.txt`:

```cmake
add_subdirectory(CrashCatch)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE CrashCatch::CrashCatch)
```

This method is great for embedded or monorepo-style projects. It ensures that the CrashCatch target `INTERFACE ` only is available immediatly without installation.

---

## 📦 Installing to Your System (Optional)

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

If you've installed CrashCatch via `cmake --install`, it can be imported in another project.

```cmake
find_package(CrashCatch REQUIRED)
add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE CrashCatch::CrashCatch)
```

CMake will locate the headers and preconfigured target with no additional setup.
>**Linux Note:** You may optionally add `-rdynamic` or link `-ldl` for bettery symbol resolution during stack tracing:
```cmake
target_link_options(MyApp PRIVATE -rdynamic)
``` 

---

## 🧪 Example
See [`examples/`](../examples/) folder for fully working sample projects that compile with CMake.