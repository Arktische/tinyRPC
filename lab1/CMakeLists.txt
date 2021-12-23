set(CMAKE_CXX_STANDARD 17)
include_directories(
        ./
        ${PROJECT_SOURCE_DIR}
)
add_executable(${PROJECT_NAME}_lab1 echo-server.hpp echo-server.cc main.cc)