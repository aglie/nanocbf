cmake_minimum_required(VERSION 3.25)
project(nanocbf)

set(CMAKE_CXX_STANDARD 17)

add_library(nanocbflib cbfframe_simple.cpp)

add_executable(nanocbf main.cpp)
target_link_libraries(nanocbf nanocbflib)