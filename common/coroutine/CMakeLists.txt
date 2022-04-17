set(CMAKE_CXX_STANDARD 20)

include_directories(
        ./
        ${PROJECT_SOURCE_DIR}
)
enable_testing()

add_library(
        coroutine
        coctx_swap.S
        coroutine_pool.cc
)