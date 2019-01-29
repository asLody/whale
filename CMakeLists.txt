cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 14)
add_subdirectory(whale)
include_directories(whale/include)
include_directories(whale/src)

#add_executable(test whale/test/test_hook.cc)
#target_link_libraries(test whale)
