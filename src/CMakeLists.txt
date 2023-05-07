add_library( kck 
             kck_str.cpp
             kck_cnf.cpp
             kck_sat.cpp
             kck_log.cpp )

add_library( cbx 
             cbx_utils.cpp
             cbx_sim.cpp
             cbx_3unihg.cpp )

target_link_libraries( cbx kck )

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

add_executable( graph_finder graph_finder_z3.cpp )
target_link_directories( graph_finder PUBLIC ../lib )
target_include_directories( graph_finder PRIVATE ../inc )
target_link_libraries( graph_finder kck cbx )
target_link_libraries( graph_finder cadical z3 spdlog::spdlog )

Target_compile_options( graph_finder PUBLIC "${CXX_OPTIONS}" )
target_compile_options( graph_finder PUBLIC "$<$<CONFIG:DEBUG>:${CXX_DEBUG_OPTIONS}>" )
target_compile_options( graph_finder PUBLIC "$<$<CONFIG:RELEASE>:${CXX_RELEASE_OPTIONS}>" )

add_executable( tst_kck_sat tst_kck_sat.cpp )
target_link_directories( tst_kck_sat PUBLIC ../lib )
target_include_directories( tst_kck_sat PRIVATE ../inc )
target_link_libraries( tst_kck_sat kck cbx )
target_link_libraries( tst_kck_sat cadical spdlog::spdlog )

#add_executable( tst_kck_formula tst_kck_formula.cpp kck_str.cpp kck_cnf.cpp ) 

#add_executable( test_to_cnf test_to_cnf.cpp to_cnf.cpp sat.cpp )
#target_link_directories( test_to_cnf PUBLIC ../lib )
#target_link_libraries( test_to_cnf cadical )

#add_executable( test_sat test_sat.cpp to_cnf.cpp sat.cpp )
#target_link_directories( test_sat PUBLIC ../lib )
#target_link_libraries( test_sat cadical )
