set(CMAKE_CXX_STANDARD 20)
include_directories(
        ${PROJECT_SOURCE_DIR}
)

add_executable(
        protocol_test
        protocol_test.cc
)

target_link_libraries(
        protocol_test
        gtest_main
        ${GTEST_LIBRARIES}
)
include(GoogleTest)

gtest_discover_tests(
        protocol_test
)
