cmake_minimum_required( VERSION 3.14 )

project( graph_finder )

set(CMAKE_CXX_STANDARD 17)

add_subdirectory( src )

target_include_directories( graph_finder PRIVATE inc )

