cmake_minimum_required(VERSION 3.12)
project(tinyRPC VERSION 0.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

FetchContent_Declare(
        benchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.6.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest benchmark)
enable_testing()
add_subdirectory(
        common
)
add_subdirectory(lab0)
add_library(${PROJECT_NAME} tinyRPC.cc)