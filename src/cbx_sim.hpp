#pragma once

namespace cbx {

constexpr int choose( int n, int k ) 
{
    int nom = 1; 
    int den = 1;

    int rounds = k;
    for ( int i = 0; i < rounds; i++ )
    {
        nom *= n--;
        den *= k--;
    }
    return nom / den;
}

}
