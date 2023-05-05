#include "../inc/cadical.hpp"
#define cdcl CaDiCaL

#include "kck_cnf.hpp"
#include "kck_form.hpp"

#define SAT_U 0
#define SAT_Y 10
#define SAT_N 20


namespace kck {

using sat_solver_t = CaDiCaL::Solver;

void add_cnf( sat_solver_t& sat_solver, const cnf_t< int > &cnf );

void add_cnf( sat_solver_t& sat_solver, const cnf_tree_t< int > &cnf_tree );

}
