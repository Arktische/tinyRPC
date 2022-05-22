cmake_minimum_required(VERSION 3.12)
project(tinyRPC VERSION 0.1 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories(
        ${PROJECT_SOURCE_DIR}
)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://hub.fastgit.xyz/google/googletest/archive/refs/tags/release-1.11.0.zip
)

FetchContent_Declare(
        benchmark
        URL https://hub.fastgit.xyz/google/benchmark/archive/refs/tags/v1.6.1.zip
)

#For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "OS: Linux")
else ()
    message(FATAL_ERROR "Not supported platform ${CMAKE_SYSTEM_NAME}")
endif ()

if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    message(STATUS "Arch: x86_64 cpu")
else ()
    message(FATAL_ERROR "Not supported arch ${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif ()

FetchContent_MakeAvailable(googletest benchmark)
enable_testing()
add_subdirectory(common)
add_subdirectory(lab0)
add_subdirectory(lab1)
add_subdirectory(protocol)
add_subdirectory(net)
add_subdirectory(test)
add_subdirectory(async)
add_subdirectory(net2)
add_library(${PROJECT_NAME} tiny.cc)