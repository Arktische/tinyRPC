set(CMAKE_CXX_STANDARD 20)
include_directories(
        ./
        ${PROJECT_SOURCE_DIR}
)
enable_testing()

add_library(
        net
        fd_event.cc
        timer.cc
        reactor.cc
        net_address.cc
        tcp_server.cc
        tcp_client.cc
        tcp_buffer.cc
        tcp_connection.cc
        io_thread.cc
        tcp_conn_timer.cc
        coroutine_hook.cc
)

add_executable(
        net_test
        net_test.cc
)
target_link_libraries(net_test net gtest_main ${GTEST_LIBRARIES})
include(GoogleTest)
gtest_discover_tests(
        net_test
)