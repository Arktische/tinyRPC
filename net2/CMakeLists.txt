set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(net2 tcp_stream.cc listener.cc address.cc io.cc alloc.cc)