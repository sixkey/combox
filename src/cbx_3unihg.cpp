#include "cbx_3unihg.hpp"

namespace cbx 
{

std::ostream& operator<<( std::ostream& os, const std::set< int > &e )
{
    os << "{";
    for ( auto it = e.begin(); it != e.end(); it++)
    {
        if ( it != e.begin() ) os << ", ";
        os << *it;
    }
    return os << "}";
}

std::ostream& operator<<( std::ostream& os, const cbx::hypergraph_t& h )
{
    os << "Hypergraph[" << h.n << "]\n{\n";
    for ( auto &e : h.edges )
        kck::str_indent( os, 2 ) << e << std::endl;
        
    return os << "}";
}

}
