add_library(async async.cpp event.cpp thread_pool.cpp)
target_compile_features(async PUBLIC cxx_std_20)
target_compile_options(async PUBLIC -fcoroutines -Wall -Wextra -pipe)
target_link_libraries(async pthread)