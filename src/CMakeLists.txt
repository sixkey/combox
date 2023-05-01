add_executable( graph_finder main.cpp to_cnf.cpp sat.cpp )
target_link_directories( graph_finder PUBLIC ../lib )
target_link_libraries( graph_finder cadical )

add_executable( test_to_cnf test_to_cnf.cpp to_cnf.cpp sat.cpp )
target_link_directories( test_to_cnf PUBLIC ../lib )
target_link_libraries( test_to_cnf cadical )

add_executable( test_sat test_sat.cpp to_cnf.cpp sat.cpp )
target_link_directories( test_sat PUBLIC ../lib )
target_link_libraries( test_sat cadical )
