if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

add_executable( proj_plane proj_plane.cpp )
target_link_directories( proj_plane PUBLIC ../lib )
target_include_directories( proj_plane PRIVATE ../inc )
target_link_libraries( proj_plane kck cbx )
target_link_libraries( proj_plane cadical z3 spdlog::spdlog )
Target_compile_options( proj_plane PUBLIC "${CXX_OPTIONS}" )
target_compile_options( proj_plane PUBLIC "$<$<CONFIG:DEBUG>:${CXX_DEBUG_OPTIONS}>" )
target_compile_options( proj_plane PUBLIC "$<$<CONFIG:RELEASE>:${CXX_RELEASE_OPTIONS}>" )
