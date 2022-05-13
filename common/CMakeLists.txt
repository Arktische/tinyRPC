set(CMAKE_CXX_STANDARD 20)
include_directories(
        ${PROJECT_SOURCE_DIR}
)
enable_testing()

add_compile_options(-mavx2)
add_executable(
        common_test
        common_test.cc
        trait.hpp
)

target_link_libraries(
        common_test
        gtest_main
        ${GTEST_LIBRARIES}
)

add_executable(
        common_bench
        common_bench.cc
)
target_compile_options(
        common_bench
        PRIVATE -O3
)

target_link_libraries(
        common_bench
        benchmark::benchmark
)
add_subdirectory(coroutine)
include(GoogleTest)
gtest_discover_tests(
        common_test
)