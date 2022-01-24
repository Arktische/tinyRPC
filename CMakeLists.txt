cmake_minimum_required(VERSION 3.12)
project(tinyRPC VERSION 0.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
enable_testing()
add_subdirectory(
        common
)
add_subdirectory(lab0)
add_library(${PROJECT_NAME} tinyRPC.cc)