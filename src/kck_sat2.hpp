#include "../inc/cadical.hpp"
#define cdcl CaDiCaL

#include "kck_cnf.hpp"
#include "kck_form.hpp"

#define SAT_U 0
#define SAT_Y 10
#define SAT_N 20

using sat_solver_t = CaDiCaL::Solver;


namespace kck {

void add_cnf( sat_solver_t& sat_solver, cnf_t< int > cnf );

void add_cnf( sat_solver_t& sat_solver, cnf_tree_t< int > cnf_tree );

}
