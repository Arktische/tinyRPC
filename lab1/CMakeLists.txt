set(CMAKE_CXX_STANDARD 17)
include_directories(
        ./
        ${PROJECT_SOURCE_DIR}
)
enable_testing()
add_executable(echo-server echo-server.hpp echo-server.cc)
target_link_libraries(
        echo-server
        pthread
)

add_executable(
        echo_test
        echo-test.cc
        )
target_link_libraries(
        echo_test
        gtest_main
        ${GTEST_LIBRARIES}
)

include(GoogleTest)

gtest_discover_tests(
        echo_test
)
