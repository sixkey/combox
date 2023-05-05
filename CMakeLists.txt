cmake_minimum_required( VERSION 3.14 )

project( graph_finder )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_COMPILER "/usr/bin/g++" )

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release )
endif()

set( CXX_OPTIONS -Wall -Wextra )
set( CXX_DEBUG_OPTIONS -g )
set( CXX_RELEASE_OPTIONS -O3 -Z )

add_subdirectory( src )

#target_compile_options( graph_finder PRIVATE -DSPDLOG_ACTIVE_LEVEL=100 )



