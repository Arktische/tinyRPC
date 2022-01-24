set(CMAKE_CXX_STANDARD 17)
include_directories(
        ./
        ${PROJECT_SOURCE_DIR}
)

add_executable(lab0 echo-server.cc)