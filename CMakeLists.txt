cmake_minimum_required(VERSION 3.12)
project(tinyRPC VERSION 0.1 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)



include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)

FetchContent_Declare(
        benchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.6.1.zip
)

FetchContent_Declare(
        epoll-shim
        URL https://github.com/jiixyj/epoll-shim/archive/refs/tags/v0.0.20210418.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "OS: Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "OS: macOS")
else()
    message(FATAL_ERROR "Not supported platform ${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    message(STATUS "Arch: x86_64 cpu")
else()
    message(FATAL_ERROR "Not supported arch ${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()

FetchContent_MakeAvailable(googletest benchmark epoll-shim)
enable_testing()
add_subdirectory(
        common
)
add_subdirectory(lab0)
add_subdirectory(lab1)
add_library(${PROJECT_NAME} tinyRPC.cc)