

#include "kck_sat.hpp"

namespace kck {

void add_cnf( sat_solver_t &sat_solver, const cnf_clause_t< int > &clause )
{
    for ( int literal : clause )
        sat_solver.add( literal );
    sat_solver.add( 0 );
}

void add_cnf( sat_solver_t &sat_solver, const cnf_t< int > &cnf )
{
    for ( auto &clause : cnf ) {
        add_cnf( sat_solver, clause );
    }
}

void add_cnf( sat_solver_t& sat_solver, const cnf_leaf< int > &leaf )
{
    add_cnf( sat_solver, leaf.clauses );
}

void add_cnf( sat_solver_t& sat_solver, const cnf_rose_masked< int > &rose )
{

    for ( int i = 0; i < rose.children.size(); i++ )
    {
        if ( (*rose.mask)[ i ] ) add_cnf( sat_solver, rose.children[ i ] );
    }
    for ( auto& child : rose.children )
    {
        add_cnf( sat_solver, child );
    }
}

void add_cnf( sat_solver_t& sat_solver, const cnf_rose< int > &rose )
{
    for ( auto& child : rose.children )
    {
        add_cnf( sat_solver, child );
    }
}

void add_cnf( sat_solver_t& sat_solver, const cnf_tree_t< int > &cnf_tree ) 
{
    std::visit( [&]( const auto& c ) 
    {
        add_cnf( sat_solver, c );
    }, cnf_tree );
}

}
