add_library(async event.cc thread_pool.cc)
target_compile_features(async PUBLIC cxx_std_20)
target_compile_options(async PUBLIC -Wall -Wextra -pipe)
target_link_libraries(async pthread)

add_executable(async_test async_test.cc)
target_compile_features(async_test PUBLIC cxx_std_20)
target_compile_options(async_test PUBLIC -Wall -Wextra -pipe)
target_link_libraries(async_test async gtest_main ${GTEST_LIBRARIES})

include(GoogleTest)
gtest_discover_tests(
        async_test
)
add_executable(testcc example.cc)
target_link_libraries(testcc async)