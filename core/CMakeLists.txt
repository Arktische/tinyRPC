set(CMAKE_CXX_STANDARD 17)

include_directories(
        ./
        ${PROJECT_SOURCE_DIR}
)

add_library(
        core
        buffer-pool.cc
        link-buffer.hpp
        tcp-stream-buffer.hpp
        timer.hpp
)