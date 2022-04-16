set(CMAKE_CXX_STANDARD 20)

include_directories(
        ./
        ${PROJECT_SOURCE_DIR}
)
set(LIBURING_LIBRARY ${CMAKE_SOURCE_DIR}/3rd/liburing/src/liburing.a)
add_custom_target(build_liburing ALL
        COMMAND ${CMAKE_MAKE_PROGRAM} all
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rd/liburing/src
        )

add_library(
        core
        buffer-pool.cc
        link-buffer.hpp
        tcp-stream-buffer.hpp
        timer.hpp
)
add_dependencies(core build_liburing)
target_link_libraries(core ${LIBURING_LIBRARY})
