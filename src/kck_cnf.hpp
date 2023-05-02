#pragma once

#include <vector>
#include <boost/bimap.hpp>

namespace kck {

//// Types ////////////////////////////////////////////////////////////////////

template < typename var_t >
struct lit_t
{
    var_t v;
    bool pos;

    lit_t operator-() { return { v, !pos }; }
};

template < typename var_t >
using var_to_id_t = boost::bimap< var_t, int >;

using sat_cnf_cls_t = std::vector< int >;
using sat_cnf_t = std::vector< sat_cnf_cls_t >;

template < typename var_t >
using cnf_clause_t = std::vector< lit_t< var_t > >;

template < typename var_t >
using cnf_t = std::vector< cnf_clause_t< var_t > >;

template < typename var_t >
struct cnf_comp
{
    boost::bimap< var_t, int > mapping;
    sat_cnf_t cnf;
    cnf_t< var_t > cnf_orig;

    cnf_comp( boost::bimap< var_t, int > mapping 
            , sat_cnf_t cnf
            , cnf_t< var_t > ) = default;

    std::size_t size()
    {
        return cnf.size();
    }
};


template < typename var_t >
cnf_comp< var_t > compile_cnf( cnf_t< var_t > cnf )
{

    var_to_id_t< var_t > mapping;

    sat_cnf_t compiled_cnf;

    int id_counter = 1;

    for ( auto& clause : cnf )
    {
        std::vector< int > comp_clause;
        for ( auto& lit : clause )
        {
            auto it = mapping.left.find( lit.var );
            
            int id = 0;
            if ( it == mapping.left.end() ) 
            {
                id = id_counter++;
                mapping.insert( { lit.var, id } );
            } else {
                id = it->second;
            }
            comp_clause.push_back( id * ( lit.pos ? 1 : -1 ) );
        }
        compiled_cnf.push_back( comp_clause ); 
    }

    return { mapping, compiled_cnf, std::move( cnf ) };
}

//// Stats ////////////////////////////////////////////////////////////////////

struct cnf_stats
{
    int clauses = 0;
    int var_count = 0;
    size_t max_clause_size = 0;
};

template < typename var_t >
cnf_stats cnf_get_stats( cnf_t< var_t > cnf );

std::ostream& operator <<( std::ostream& os, cnf_stats s );

}
