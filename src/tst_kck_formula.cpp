#include <memory>
#include <iostream>
#include <string>

#include "kck_form.hpp"
#include "kck_cnf.hpp"

using namespace kck;

std::string str_labeler( int i )
{
    return "x_" + std::to_string( i );
}

int main()
{
    formula_ptr< std::string > node = make_and< std::string >( 
        { make_or< std::string >( { make_lit< std::string >( "A", false )
                                  , make_lit< std::string >( "A", false ) } )
        , make_or< std::string >( { make_lit< std::string >( "A" )
                                  , make_lit< std::string >( "A" ) } ) } );

    cnf_t< std::string > cnf = node->to_cnf( str_labeler );

    std::cout << node->to_string() << std::endl;

    std::cout << cnf_get_stats< std::string >( cnf ) << std::endl;
}
