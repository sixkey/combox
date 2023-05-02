#include "kck_str.hpp"

std::ostream& kck::str_indent( std::ostream& ss, int level )
{
    for ( int i = 0; i < level; i++ )
        ss << " ";
    return ss;
}
