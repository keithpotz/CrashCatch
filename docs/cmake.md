# ⚙️ CMake Integration

CrashCatch supports **modern CMake** out of the box, with both **header-only subdirectory usage** and **installable package support** for larger or multi-project environments. It is also available via **vcpkg** and **Conan**.

---

## 🧩 Using as a Subdirectory

If you cloned or copied CrashCatch into your project, add this to your `CMakeLists.txt`:

```cmake
add_subdirectory(CrashCatch)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE CrashCatch::CrashCatch)
```

This method is great for embedded or monorepo-style projects. The CrashCatch `INTERFACE` target is available immediately without installation.

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
├── include/
│   ├── CrashCatch.hpp
│   └── CrashCatchDLL.hpp
└── lib/cmake/CrashCatch/
    ├── CrashCatchConfig.cmake
    └── CrashCatchTargets.cmake
```

---

## 🔍 Using find_package()

If you've installed CrashCatch via `cmake --install`, it can be imported in another project:

```cmake
find_package(CrashCatch REQUIRED)
add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE CrashCatch::CrashCatch)
```

CMake will locate the headers and preconfigured target with no additional setup.

> **Linux Note:** Add `-rdynamic` for better symbol resolution in stack traces:
```cmake
target_link_options(MyApp PRIVATE -rdynamic)
```

---

## 📦 vcpkg

CrashCatch is available as a vcpkg port. If you have vcpkg installed:

```bash
vcpkg install crashcatch
```

Then in your `CMakeLists.txt`:

```cmake
find_package(CrashCatch CONFIG REQUIRED)
target_link_libraries(MyApp PRIVATE CrashCatch::CrashCatch)
```

Pass your vcpkg toolchain file when configuring:

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Or use a `vcpkg.json` manifest in your project root:

```json
{
  "dependencies": [ "crashcatch" ]
}
```

---

## 🐍 Conan

CrashCatch is available as a Conan package. Add it to your `conanfile.txt`:

```ini
[requires]
crashcatch/1.4.0

[generators]
CMakeDeps
CMakeToolchain
```

Then install and configure:

```bash
conan install . --output-folder=build --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
```

Or in a `conanfile.py`:

```python
def requirements(self):
    self.requires("crashcatch/1.4.0")
```

---

## 🧪 Example

See the [`examples/`](../examples/) folder for fully working sample projects that compile with CMake.
