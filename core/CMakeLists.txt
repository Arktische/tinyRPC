set(CMAKE_CXX_STANDARD 20)
include_directories(
        ${PROJECT_SOURCE_DIR}
)

enable_testing()
add_compile_options(-fcoroutines -fconcepts)
add_executable(
    core_test
    core_test.cc
)

target_link_libraries(
    core_test
    gtest_main
    ${GTEST_LIBRARIES}
)
include(GoogleTest)
gtest_discover_tests(
    core_test
)

add_library(core core.cc)
