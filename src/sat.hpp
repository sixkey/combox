#include "../inc/cadical.hpp"
#include <optional>
#include <map>
#include "to_cnf.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#define cdcl CaDiCaL

#define SAT_U 0
#define SAT_Y 10
#define SAT_N 20

using sat_solver = CaDiCaL::Solver;

template < typename var_t >
std::string to_string( var_to_id_t< var_t > var_to_id )
{
    std::stringstream ss;
    ss << "{\n";
    for ( auto it = var_to_id.left.begin(); it != var_to_id.left.end(); it++ )
    {
        ss << "\t" << it->first << ": " << it->second << ",\n";
    }
    ss << "}\n";
    return ss.str();
}

template < typename var_t > 
struct sat_res 
{
    int res = 0;
    std::shared_ptr< sat_solver > solver;
    var_to_id_t< var_t > var_to_id;

    bool val( var_t v ) {
        auto it = var_to_id.left.find( v );
        if ( it == var_to_id.left.end() )
            throw std::runtime_error( "the key is not present" );

        return solver->val( it->second ) > 0;
    }

    std::string to_string()
    {
        std::stringstream ss;
        ss << "Result ";
        switch ( res ) 
        { 
            case SAT_Y: ss << "sat"; break;
            case SAT_N: ss << "unsat"; break;
            case SAT_U: ss << "unknown"; break;
        }
        ss << "\n";

        for ( auto it = var_to_id.left.begin(); it != var_to_id.left.end(); it++ )
            ss << "  " << it->first << ": " << val( it->first ) << "\n"; 
        return ss.str();
    }
};


void add_cnf( std::shared_ptr< sat_solver > solver, cnf_t cnf );

template < typename var_t >
sat_res< var_t > sat_solve( form_ptr< var_t > formula )
{
    auto [ cnf, mapping ] = formula->to_cnf();

    auto solver = std::make_shared< sat_solver >();
    add_cnf( solver, cnf );
    int res = solver->solve();

    return sat_res< var_t >{ res, solver, std::move( mapping ) };
}

template < typename var_t >
std::pair< int, std::shared_ptr< sat_solver > > sat_solve( cnf_t cnf )
{
    auto solver = std::make_shared< sat_solver >();
    add_cnf( solver, cnf );
    int res = solver->solve();
    return { res, solver };
}

