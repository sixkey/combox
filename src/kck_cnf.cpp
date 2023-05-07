#include "kck_cnf.hpp"

namespace kck {

std::ostream& operator <<( std::ostream& os, const cnf_stats &s )
{
    return os << "CnfStats ( clauses = " << s.clauses << ", max clause size = " << s.max_clause_size << ", variable count = " << s.var_count << " )";
}

}
