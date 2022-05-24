set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(net2 tcp_stream.cc server.cc address.cc alloc.cc io.cc)

add_executable(
        net2_test
        net2_test.cc
)

target_link_libraries(
        net2_test
        gtest_main
        ${GTEST_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(net2_test)