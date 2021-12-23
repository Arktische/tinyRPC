cmake_minimum_required(VERSION 3.12)
project(tinyRPC VERSION 0.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(
        lab1
)

add_library(${PROJECT_NAME} tinyRPC.cc)