#include "kck_cnf.hpp"

namespace kck {

std::ostream& operator <<( std::ostream& os, cnf_stats s )
{
    return os << "CnfStats ( clauses = " << s.clauses << ", max clause size = " << s.max_clause_size << ", variable count = " << s.var_count << " )";
}


template < typename var_t >
cnf_stats cnf_get_stats( cnf_t< var_t > cnf )
{
    std::set< var_t > variables;

    cnf_stats res;
    for ( auto &clause : cnf )
    {
        for ( auto &lit : clause ) 
        {
            res.max_clause_size = std::max( clause.size(), res.max_clause_size );
            res.clauses += 1;
            variables.insert( lit.var );
        }
    }
    res.var_count = variables.size();
    return res;
}

}
