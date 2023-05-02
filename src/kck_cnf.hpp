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

    std::size_t size()
    {
        return cnf.size();
    }
};

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
