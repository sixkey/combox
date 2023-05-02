add_library( kck 
             kck_str.cpp
             kck_cnf.cpp
             kck_sat.cpp )


add_executable( graph_finder main.cpp )
target_link_directories( graph_finder PUBLIC ../lib )
target_link_libraries( graph_finder kck )
target_link_libraries( graph_finder cadical )

add_executable( tst_kck_formula tst_kck_formula.cpp kck_str.cpp kck_cnf.cpp ) 


#add_executable( test_to_cnf test_to_cnf.cpp to_cnf.cpp sat.cpp )
#target_link_directories( test_to_cnf PUBLIC ../lib )
#target_link_libraries( test_to_cnf cadical )

#add_executable( test_sat test_sat.cpp to_cnf.cpp sat.cpp )
#target_link_directories( test_sat PUBLIC ../lib )
#target_link_libraries( test_sat cadical )
