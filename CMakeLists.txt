cmake_minimum_required(VERSION 3.15)
project(CrashCatchExamples VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === Define CrashCatch Header-Only Library ===
add_library(CrashCatch INTERFACE)
target_include_directories(CrashCatch INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(CrashCatch INTERFACE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(CrashCatch INTERFACE DbgHelp User32)

# === Install CrashCatch (Header + Target) ===
install(FILES include/CrashCatch.hpp DESTINATION include)

# Export the target for downstream use
install(TARGETS CrashCatch
        EXPORT CrashCatchTargets
        INCLUDES DESTINATION include)

# Generate a config file to support find_package()
install(EXPORT CrashCatchTargets
        FILE CrashCatchTargets.cmake
        NAMESPACE CrashCatch::
        DESTINATION lib/cmake/CrashCatch)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CrashCatchConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CrashCatchConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CrashCatchConfig.cmake"
    INSTALL_DESTINATION lib/cmake/CrashCatch
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CrashCatchConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CrashCatchConfigVersion.cmake"
    DESTINATION lib/cmake/CrashCatch
)

# === Examples ===
add_executable(Example_ZeroConfig examples/Example_ZeroConfig.cpp)
target_link_libraries(Example_ZeroConfig PRIVATE CrashCatch)

add_executable(Example_OneLiner examples/Example_OneLiner.cpp)
target_link_libraries(Example_OneLiner PRIVATE CrashCatch)

add_executable(Example_FullConfig examples/Example_FullConfig.cpp)
target_link_libraries(Example_FullConfig PRIVATE CrashCatch)

add_executable(Example_ThreadCrash examples/Example_ThreadCrash.cpp)
target_link_libraries(Example_ThreadCrash PRIVATE CrashCatch)

add_executable(Example_divideByZero examples/Example_divideByZero.cpp)
target_link_libraries(Example_divideByZero PRIVATE CrashCatch)
