cmake_minimum_required(VERSION 3.15)
project(CrashCatchExamples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include CrashCatch
include_directories(include)

# Common Windows libraries
set(WIN_LIBS DbgHelp User32)

# List of example sources (one executable per example)
add_executable(Example_ZeroConfig examples/Example_ZeroConfig.cpp)
target_link_libraries(Example_ZeroConfig PRIVATE ${WIN_LIBS})

add_executable(Example_OneLiner examples/Example_OneLiner.cpp)
target_link_libraries(Example_OneLiner PRIVATE ${WIN_LIBS})

add_executable(Example_FullConfig examples/Example_FullConfig.cpp)
target_link_libraries(Example_FullConfig PRIVATE ${WIN_LIBS})

add_executable(Example_ThreadCrash examples/Example_ThreadCrash.cpp)
target_link_libraries(Example_ThreadCrash PRIVATE ${WIN_LIBS})

add_executable(Example_divideByZero examples/Example_divideByZero.cpp)
target_link_libraries(Example_divideByZero PRIVATE ${WIN_LIBS})

