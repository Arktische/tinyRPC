set(CMAKE_CXX_STANDARD 17)
find_package(GTest REQUIRED)
include_directories(
        ${PROJECT_SOURCE_DIR}
)
enable_testing()
add_executable(
        common_test
        common-test.cc
)

target_link_libraries(
        common_test
        gtest_main
        ${GTEST_LIBRARIES}
)
include(GoogleTest)
gtest_discover_tests(
        common_test
)
