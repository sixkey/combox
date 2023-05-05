#pragma once

#include <vector>

template < typename T >
std::vector< T >& add_to( std::vector< T > &a, std::vector< T > &&b )
{
    a.insert( a.end(), b.begin(), b.end() );
    return a;
}
