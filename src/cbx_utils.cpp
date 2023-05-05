#include "cbx_utils.hpp"

namespace cbx {

void sort_i ( int& a, int& b )
{
    if ( a > b )
    {
        int c = b;
        b = a;
        a = c;
    }
}

void sort_i ( int& a, int& b, int& c )
{
    sort_i( b, c );
    sort_i( a, b );
    sort_i( b, c );
}

}

