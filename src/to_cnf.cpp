#include "to_cnf.hpp" 
#include <algorithm>
#include <ostream>

std::ostream& operator <<( std::ostream& os, cnf_stats s )
{
    return os << "CnfStats ( clauses = " << s.clauses << ", max clause size = " << s.max_clause_size << ", variable count = " << s.var_count << " )";
}

cnf_stats cnf_get_stats( cnf_t cnf )
{
    std::set< int > variables;

    cnf_stats res;
    for ( auto &clause : cnf )
    {
        for ( auto &lit : clause ) 
        {
            res.max_clause_size = std::max( clause.size(), res.max_clause_size );
            res.clauses += 1;
            variables.insert( std::abs( lit ) );
        }
    }
    res.var_count = variables.size();
    return res;
}

std::ostream& str_indent( std::ostream& ss, int level )
{
    for ( int i = 0; i < level; i++ )
        ss << " ";
    return ss;
}
