#include "to_cnf.hpp"
#include <memory>
#include <iostream>
#include "sat.hpp"
#include <string>

int main()
{
    form_ptr< std::string > node = make_and< std::string >( 
            { make_or< std::string >( { make_lit< std::string >( "A", false )
                                      , make_lit< std::string >( "A", false ) } )
            , make_or< std::string >( { make_lit< std::string >( "A" )
                                      , make_lit< std::string >( "A" ) } ) } );
    auto res = sat_solve( node );
    assert( res.res == SAT_N ); 
}
